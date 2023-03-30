/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_MQTT_MESSAGE_ROUTER_HPP__
#define __ASIO2_MQTT_MESSAGE_ROUTER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <optional>

#include <asio2/base/iopool.hpp>
#include <asio2/base/define.hpp>

#include <asio2/base/detail/function_traits.hpp>
#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/shared_mutex.hpp>

#include <asio2/mqtt/detail/mqtt_topic_util.hpp>

#include <asio2/mqtt/message.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;

	/**
	 * used for:
	 * 
	 * bool ret = client.subscribe("/usr/topic1", 0, [](mqtt::message& msg){});
	 * util recvd the suback message, then the ret is true.
	 * 
	 * bool ret = client.publish("/usr/topic1", "...payload...", 0); 
	 * util recvd the puback message, then the ret is true.
	 * 
	 * and so on...
	 */
	template<class derived_t, class args_t>
	class mqtt_message_router_t
	{
		friend derived_t;

		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;

	public:
		using self = mqtt_message_router_t<derived_t, args_t>;

		using args_type = args_t;
		using subnode_type = typename args_type::template subnode<derived_t>;

		using key_type = std::pair<mqtt::control_packet_type, mqtt::two_byte_integer::value_type>;
		using val_type = detail::function<void(mqtt::message&)>;

		struct hasher
		{
			inline std::size_t operator()(key_type const& pair) const noexcept
			{
				std::size_t v = detail::fnv1a_hash<std::size_t>(
					(const unsigned char*)(std::addressof(pair.first)), sizeof(pair.first));
				return detail::fnv1a_hash<std::size_t>(v,
					(const unsigned char*)(std::addressof(pair.second)), sizeof(pair.second));
			}
		};

		/**
		 * @brief constructor
		 */
		mqtt_message_router_t() = default;

		/**
		 * @brief destructor
		 */
		~mqtt_message_router_t() = default;

	protected:
		template<class FunctionT>
		inline bool _add_router(mqtt::message& msg, FunctionT&& callback)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			bool r = false;

			std::visit([&derive, &callback, &r](auto& m) mutable
			{
				r = derive._add_router(m, std::forward<FunctionT>(callback));
			}, msg.base());

			return r;
		}

		template<class Message, class FunctionT>
		typename std::enable_if_t<mqtt::is_rawmsg<Message>(), bool>
		inline _add_router(Message& msg, FunctionT&& callback)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			using message_type = typename detail::remove_cvref_t<Message>;

			if constexpr (!mqtt::has_packet_id<message_type>::value)
			{
				static_assert(detail::always_false_v<Message> && "This mqtt message don't has Packet Identifier");
				return false;
			}
			else
			{
				ASIO2_ASSERT(
					msg.packet_type() >= mqtt::control_packet_type::connect &&
					msg.packet_type() <= mqtt::control_packet_type::auth);

				if (!(
					msg.packet_type() >= mqtt::control_packet_type::connect &&
					msg.packet_type() <= mqtt::control_packet_type::auth))
				{
					return false;
				}

				key_type key = { msg.packet_type(), msg.packet_id() };

				return derive._add_router(std::move(key), std::forward<FunctionT>(callback));
			}
		}

		template<class FunctionT>
		inline bool _add_router(key_type key, FunctionT&& callback)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			derive.dispatch([&derive, key, cb = std::forward<FunctionT>(callback)]() mutable
			{
				derive._do_add_router(std::move(key), std::move(cb));
			});

			return true;
		}

		template<class FunctionT>
		inline bool _do_add_router(key_type key, FunctionT&& callback)
		{
			using fun_traits_type = function_traits<FunctionT>;
			using arg0_type = typename std::remove_cv_t<std::remove_reference_t<
				typename fun_traits_type::template args<0>::type>>;

			asio2::unique_locker g(this->message_router_mutex_);

			if constexpr (std::is_same_v<arg0_type, mqtt::message>)
			{
				auto[_1, inserted] = this->message_router_.insert_or_assign(std::move(key),
					std::forward<FunctionT>(callback));

				ASIO2_ASSERT(inserted);

				asio2::ignore_unused(_1, inserted);
			}
			else
			{
				auto[_1, inserted] = this->message_router_.insert_or_assign(std::move(key),
					[cb = std::forward<FunctionT>(callback)](mqtt::message& msg) mutable
				{
					arg0_type* p = std::get_if<arg0_type>(std::addressof(msg.base()));
					if (p)
					{
						cb(*p);
					}
				});

				ASIO2_ASSERT(inserted);

				asio2::ignore_unused(_1, inserted);
			}

			return true;
		}

		inline void _del_router(mqtt::message& msg)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			std::visit([&derive](auto& m) mutable
			{
				derive._del_router(m);
			}, msg.base());
		}

		template<class Message>
		typename std::enable_if_t<mqtt::is_rawmsg<Message>(), void>
		inline _del_router(Message& msg)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			using message_type = typename detail::remove_cvref_t<Message>;

			if constexpr (!mqtt::has_packet_id<message_type>::value)
			{
				static_assert(detail::always_false_v<Message> && "This mqtt message don't has Packet Identifier");
				return;
			}
			else
			{
				ASIO2_ASSERT(
					msg.packet_type() >= mqtt::control_packet_type::connect &&
					msg.packet_type() <= mqtt::control_packet_type::auth);

				if (!(
					msg.packet_type() >= mqtt::control_packet_type::connect &&
					msg.packet_type() <= mqtt::control_packet_type::auth))
				{
					return;
				}

				key_type key = { msg.packet_type(), msg.packet_id() };

				derive._del_router(std::move(key));
			}
		}

		inline void _del_router(key_type key)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			derive.dispatch([this, key = std::move(key)]() mutable
			{
				asio2::unique_locker g(this->message_router_mutex_);

				this->message_router_.erase(key);
			});
		}

		inline bool _match_router(mqtt::message& msg)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			std::optional<key_type> key = derive._generate_key(msg);

			if (!key.has_value())
				return false;

			return derive._call_router(key.value(), msg);
		}

		inline bool _call_router(key_type key, mqtt::message& msg)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			derive.dispatch([this, msg, key = std::move(key)]() mutable
			{
				asio2::unique_locker g(this->message_router_mutex_);

				auto it = this->message_router_.find(key);
				if (it == this->message_router_.end())
					return;

				(it->second)(msg);

				this->message_router_.erase(it);
			});

			return true;
		}

		inline std::optional<key_type> _generate_key(mqtt::message& msg)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			std::optional<key_type> r;

			std::visit([&derive, &r](auto& m) mutable
			{
				r = derive._generate_key(m);
			}, msg.base());

			return r;
		}

		template<class Message>
		typename std::enable_if_t<mqtt::is_rawmsg<Message>(), std::optional<key_type>>
		inline _generate_key(Message& msg)
		{
			using message_type = typename detail::remove_cvref_t<Message>;

			if constexpr (!mqtt::has_packet_id<message_type>::value)
			{
				return std::nullopt;
			}
			else
			{
				ASIO2_ASSERT(
					msg.packet_type() >= mqtt::control_packet_type::connect &&
					msg.packet_type() <= mqtt::control_packet_type::auth);

				if (!(
					msg.packet_type() >= mqtt::control_packet_type::connect &&
					msg.packet_type() <= mqtt::control_packet_type::auth))
				{
					return std::nullopt;
				}

				std::optional<key_type> key;

				if /**/ constexpr (mqtt::is_puback_message<message_type>())
				{
					key = { mqtt::control_packet_type::publish, msg.packet_id() };
				}
				else if constexpr (mqtt::is_pubcomp_message<message_type>())
				{
					key = { mqtt::control_packet_type::publish, msg.packet_id() };
				}
				else if constexpr (mqtt::is_suback_message<message_type>())
				{
					key = { mqtt::control_packet_type::subscribe, msg.packet_id() };
				}
				else if constexpr (mqtt::is_unsuback_message<message_type>())
				{
					key = { mqtt::control_packet_type::unsubscribe, msg.packet_id() };
				}
				else
				{
					return std::nullopt;
				}

				return key;
			}
		}

	protected:
		/// use rwlock to make thread safe
		mutable asio2::shared_mutexer                  message_router_mutex_;

		/// router map, key - pair<mqtt::control_packet_type, packet id>
		std::unordered_map<key_type, val_type, hasher> message_router_ ASIO2_GUARDED_BY(message_router_mutex_);
	};
}

#endif // !__ASIO2_MQTT_MESSAGE_ROUTER_HPP__
