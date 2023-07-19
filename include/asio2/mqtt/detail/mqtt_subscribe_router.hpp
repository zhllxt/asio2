/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_MQTT_SUBSCRIBE_ROUTER_HPP__
#define __ASIO2_MQTT_SUBSCRIBE_ROUTER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/mqtt/detail/mqtt_message_router.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;

	template<class derived_t, class args_t>
	class mqtt_subscribe_router_t
	{
		friend derived_t;

		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;

	public:
		using self = mqtt_subscribe_router_t<derived_t, args_t>;

		using args_type = args_t;
		using subnode_type = typename args_type::template subnode<derived_t>;
		using key_type = typename mqtt_message_router_t<derived_t, args_t>::key_type;

		struct hasher
		{
			inline std::size_t operator()(key_type const& pair) const noexcept
			{
				std::size_t v = asio2::detail::fnv1a_hash<std::size_t>(
					(const unsigned char*)(std::addressof(pair.first)), sizeof(pair.first));
				return asio2::detail::fnv1a_hash<std::size_t>(v,
					(const unsigned char*)(std::addressof(pair.second)), sizeof(pair.second));
			}
		};

		/**
		 * @brief constructor
		 */
		mqtt_subscribe_router_t() = default;

		/**
		 * @brief destructor
		 */
		~mqtt_subscribe_router_t() = default;

		template<class ReturnT = void, class QosOrInt, class FunctionT>
		typename std::enable_if_t<
			std::is_same_v<detail::remove_cvref_t<QosOrInt>, mqtt::qos_type> ||
			std::is_integral_v<detail::remove_cvref_t<QosOrInt>>, ReturnT>
		subscribe(std::string topic_filter, QosOrInt qos, FunctionT&& callback)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			mqtt::version ver = derive.version();

			auto pid = derive.idmgr_.get();

			if /**/ (ver == mqtt::version::v3)
			{
				mqtt::v3::subscribe msg;
				msg.packet_id(pid);
				msg.add_subscriptions(mqtt::subscription(std::move(topic_filter), qos));

				return this->subscribe<ReturnT>(std::move(msg), std::forward<FunctionT>(callback));
			}
			else if (ver == mqtt::version::v4)
			{
				mqtt::v4::subscribe msg;
				msg.packet_id(pid);
				msg.add_subscriptions(mqtt::subscription(std::move(topic_filter), qos));

				return this->subscribe<ReturnT>(std::move(msg), std::forward<FunctionT>(callback));
			}
			else if (ver == mqtt::version::v5)
			{
				mqtt::v5::subscribe msg;
				msg.packet_id(pid);
				msg.add_subscriptions(mqtt::subscription(std::move(topic_filter), qos));

				return this->subscribe<ReturnT>(std::move(msg), std::forward<FunctionT>(callback));
			}
			else
			{
				derive.idmgr_.release(pid);

				set_last_error(asio::error::invalid_argument);

				ASIO2_ASSERT(false);

				return derive.template _empty_result<ReturnT>();
			}
		}

		template<class ReturnT = void, class Message, class FunctionT>
		typename std::enable_if_t<
			std::is_same_v<detail::remove_cvref_t<Message>, mqtt::v3::subscribe> || 
			std::is_same_v<detail::remove_cvref_t<Message>, mqtt::v4::subscribe> ||
			std::is_same_v<detail::remove_cvref_t<Message>, mqtt::v5::subscribe>, ReturnT>
		subscribe(Message&& msg, FunctionT&& callback)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if (!derive.is_started())
			{
				set_last_error(asio::error::not_connected);

				ASIO2_ASSERT(false);

				return derive.template _empty_result<ReturnT>();
			}

			if (msg.subscriptions().data().empty())
			{
				set_last_error(asio::error::invalid_argument);

				ASIO2_ASSERT(false);

				return derive.template _empty_result<ReturnT>();
			}

			for (mqtt::subscription& sub : msg.subscriptions().data())
			{
				if (!mqtt::is_valid_qos(sub.qos()))
				{
					set_last_error(asio::error::invalid_argument);

					ASIO2_ASSERT(false);

					return derive.template _empty_result<ReturnT>();
				}
			}

			clear_last_error();

			[[maybe_unused]] key_type key = { msg.packet_type(), msg.packet_id() };

			derive._dispatch_subscribe(std::forward<Message>(msg), std::forward<FunctionT>(callback));

			if (derive.io_->running_in_this_thread())
			{
				return derive.template _in_progress<ReturnT>();
			}

			ASIO2_ASSERT(!derive.io_->running_in_this_thread());

			if /**/ constexpr (std::is_same_v<ReturnT, void>)
			{
				return;
			}
			else if constexpr (std::is_same_v<ReturnT, bool>)
			{
				return derive._do_router(key, [](auto& msg) mutable
				{
					if constexpr (mqtt::is_suback_message<decltype(msg)>())
					{
						for (auto&& reason : msg.reason_codes().data())
						{
							if (!mqtt::is_valid_qos(reason.value()))
							{
								return false;
							}
						}
						return true;
					}
					else
					{
						return false;
					}
				});
			}
			else if constexpr (std::is_same_v<ReturnT, mqtt::message>)
			{
				return derive._do_router(key);
			}
			else
			{
				static_assert(detail::always_false_v<ReturnT>);
			}
		}

		template<class ReturnT = void>
		ReturnT unsubscribe(std::string topic_filter)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			mqtt::version ver = derive.version();

			auto pid = derive.idmgr_.get();

			if /**/ (ver == mqtt::version::v3)
			{
				return this->unsubscribe<ReturnT>(mqtt::v3::unsubscribe(pid, std::move(topic_filter)));
			}
			else if (ver == mqtt::version::v4)
			{
				return this->unsubscribe<ReturnT>(mqtt::v4::unsubscribe(pid, std::move(topic_filter)));
			}
			else if (ver == mqtt::version::v5)
			{
				return this->unsubscribe<ReturnT>(mqtt::v5::unsubscribe(pid, std::move(topic_filter)));
			}
			else
			{
				derive.idmgr_.release(pid);

				set_last_error(asio::error::invalid_argument);

				ASIO2_ASSERT(false);

				return derive.template _empty_result<ReturnT>();
			}
		}

		template<class ReturnT = void, class Message>
		typename std::enable_if_t<
			std::is_same_v<detail::remove_cvref_t<Message>, mqtt::v3::unsubscribe> || 
			std::is_same_v<detail::remove_cvref_t<Message>, mqtt::v4::unsubscribe> ||
			std::is_same_v<detail::remove_cvref_t<Message>, mqtt::v5::unsubscribe>, ReturnT>
		unsubscribe(Message&& msg)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if (!derive.is_started())
			{
				set_last_error(asio::error::not_connected);

				ASIO2_ASSERT(false);

				return derive.template _empty_result<ReturnT>();
			}

			if (msg.topic_filters().data().empty())
			{
				set_last_error(asio::error::invalid_argument);

				ASIO2_ASSERT(false);

				return derive.template _empty_result<ReturnT>();
			}

			clear_last_error();

			[[maybe_unused]] key_type key = { msg.packet_type(), msg.packet_id() };

			// must ensure the member variable is read write in the io_context thread.
			// save the subscribed key and topic filters, beacuse if the unsubscribed
			// is sucussed, we need remove the topics from the sub map.
			derive.dispatch([&derive, key, topics = msg.topic_filters()]() mutable
			{
				if (derive.subs_map().get_subscribe_count() > 0)
					derive.unsubscribed_topics_.emplace(key, std::move(topics));
			});

			derive.async_send(std::forward<Message>(msg), [&derive, key]() mutable
			{
				// if send data failed, we need remove the added key and topics from the map.
				if (asio2::get_last_error())
				{
					derive.unsubscribed_topics_.erase(key);
				}
			});

			if (derive.io_->running_in_this_thread())
			{
				return derive.template _in_progress<ReturnT>();
			}

			ASIO2_ASSERT(!derive.io_->running_in_this_thread());

			if /**/ constexpr (std::is_same_v<ReturnT, void>)
			{
				return;
			}
			else if constexpr (std::is_same_v<ReturnT, bool>)
			{
				return derive._do_router(key, [](auto& msg) mutable
				{
					if constexpr (mqtt::is_unsuback_message<decltype(msg)>())
					{
						// UNSUBACK Payload : The Payload contains a list of Reason Codes.
						// https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901194
						if constexpr (std::is_same_v<detail::remove_cvref_t<decltype(msg)>, mqtt::v5::unsuback>)
						{
							for (auto&& reason : msg.reason_codes().data())
							{
								mqtt::error e = static_cast<mqtt::error>(reason.value());
								if (!(e == mqtt::error::success || e == mqtt::error::no_subscription_existed))
								{
									return false;
								}
							}
							return true;
						}
						else
						{
							return true;
						}
					}
					else
					{
						return false;
					}
				});
			}
			else if constexpr (std::is_same_v<ReturnT, mqtt::message>)
			{
				return derive._do_router(key);
			}
			else
			{
				static_assert(detail::always_false_v<ReturnT>);
			}
		}

		template<class ReturnT = void, class QosOrInt>
		ReturnT publish(std::string topic_name, std::string payload, QosOrInt qos)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			mqtt::version ver = derive.version();

			if /**/ (ver == mqtt::version::v3)
			{
				return this->publish<ReturnT>(mqtt::v3::publish(std::move(topic_name), std::move(payload), qos));
			}
			else if (ver == mqtt::version::v4)
			{
				return this->publish<ReturnT>(mqtt::v4::publish(std::move(topic_name), std::move(payload), qos));
			}
			else if (ver == mqtt::version::v5)
			{
				return this->publish<ReturnT>(mqtt::v5::publish(std::move(topic_name), std::move(payload), qos));
			}
			else
			{
				set_last_error(asio::error::invalid_argument);

				ASIO2_ASSERT(false);

				return derive.template _empty_result<ReturnT>();
			}
		}

		template<class ReturnT = void, class Message>
		typename std::enable_if_t<
			std::is_same_v<detail::remove_cvref_t<Message>, mqtt::v3::publish> || 
			std::is_same_v<detail::remove_cvref_t<Message>, mqtt::v4::publish> ||
			std::is_same_v<detail::remove_cvref_t<Message>, mqtt::v5::publish>, ReturnT>
		publish(Message&& msg)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if (!derive.is_started())
			{
				set_last_error(asio::error::not_connected);

				ASIO2_ASSERT(false);

				return derive.template _empty_result<ReturnT>();
			}

			if (msg.qos() > mqtt::qos_type::at_most_once && !msg.has_packet_id())
			{
				msg.packet_id(derive.idmgr_.get());
			}

			clear_last_error();

			[[maybe_unused]] std::optional<key_type> key{};

			if (msg.qos() > mqtt::qos_type::at_most_once && msg.has_packet_id())
				key = { msg.packet_type(), msg.packet_id() };

			derive.async_send(std::forward<Message>(msg));

			// qos 0 don't need a response
			if (!key.has_value())
			{
				return derive.template _empty_result<ReturnT>();
			}

			if (derive.io_->running_in_this_thread())
			{
				return derive.template _in_progress<ReturnT>();
			}

			ASIO2_ASSERT(!derive.io_->running_in_this_thread());

			if /**/ constexpr (std::is_same_v<ReturnT, void>)
			{
				return;
			}
			else if constexpr (std::is_same_v<ReturnT, bool>)
			{
				return derive._do_bool_publish(key);
			}
			else if constexpr (std::is_same_v<ReturnT, mqtt::message>)
			{
				return derive._do_router(key.value());
			}
			else
			{
				static_assert(detail::always_false_v<ReturnT>);
			}
		}

	protected:
		template<class K>
		bool _do_bool_publish(std::optional<K>& key)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return derive._do_router(key.value(), [](auto& msg) mutable
			{
				// qos 1 response
				if constexpr (mqtt::is_puback_message<decltype(msg)>())
				{
					if constexpr (std::is_same_v<detail::remove_cvref_t<decltype(msg)>, mqtt::v5::puback>)
					{
						mqtt::error e = static_cast<mqtt::error>(msg.reason_code());
						if (!(e == mqtt::error::success))
						{
							return false;
						}
						return true;
					}
					else
					{
						return true;
					}
				}
				else if constexpr (mqtt::is_pubcomp_message<decltype(msg)>())
				{
					if constexpr (std::is_same_v<detail::remove_cvref_t<decltype(msg)>, mqtt::v5::pubcomp>)
					{
						mqtt::error e = static_cast<mqtt::error>(msg.reason_code());
						if (!(e == mqtt::error::success))
						{
							return false;
						}
						return true;
					}
					else
					{
						return true;
					}
				}
				else
				{
					return false;
				}
			});
		}

		template<class ReturnT>
		inline ReturnT _empty_result()
		{
			if constexpr (std::is_same_v<ReturnT, void>)
			{
				return;
			}
			else
			{
				return ReturnT{};
			}
		}

		template<class ReturnT>
		inline ReturnT _in_progress()
		{
			if /**/ constexpr (std::is_same_v<ReturnT, void>)
			{
				set_last_error(asio::error::in_progress);
				return;
			}
			else if constexpr (std::is_same_v<ReturnT, bool>)
			{
				set_last_error(asio::error::in_progress);
				return true;
			}
			else if constexpr (std::is_same_v<ReturnT, mqtt::message>)
			{
				set_last_error(asio::error::in_progress);
				return mqtt::message{};
			}
			else
			{
				static_assert(detail::always_false_v<ReturnT>);
			}
		}

		/**
		 * callback signature : bool (auto& msg)
		 */
		template<class KeyT, class FunctionT>
		bool _do_router(KeyT key, FunctionT&& callback)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			std::promise<bool> p;
			std::future<bool> f = p.get_future();

			derive._add_router(key, [&callback, p = std::move(p)](mqtt::message& m) mutable
			{
				std::visit([&callback, &p](auto& msg) mutable
				{
					p.set_value(callback(msg));
				}, m.base());
			});

			std::future_status status = f.wait_for(derive.get_default_timeout());

			if (status == std::future_status::ready)
			{
				derive._del_router(key);
				return true;
			}

			set_last_error(asio::error::timed_out);

			derive._del_router(key);
			return false;
		}

		template<class KeyT>
		mqtt::message _do_router(KeyT key)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			std::shared_ptr<mqtt::message> r = std::make_shared<mqtt::message>();

			std::promise<void> p;
			std::future<void> f = p.get_future();

			derive._add_router(key, [r, p = std::move(p)](mqtt::message& m) mutable
			{
				*r = m;
				p.set_value();
			});

			std::future_status status = f.wait_for(derive.get_default_timeout());

			if (status == std::future_status::ready)
			{
				derive._del_router(key);
				return std::move(*r);
			}

			set_last_error(asio::error::timed_out);

			derive._del_router(key);
			return mqtt::message{};
		}

		template<class Message, class FunctionT>
		inline void _dispatch_subscribe(Message&& msg, FunctionT&& callback)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			derive.dispatch(
			[&derive, msg = std::forward<Message>(msg), cb = std::forward<FunctionT>(callback)]() mutable
			{
				derive._do_subscribe(std::move(msg), std::move(cb));
			});
		}

		template<class Message, class FunctionT>
		void _do_subscribe(Message&& msg, FunctionT&& callback)
		{
			using message_type = typename detail::remove_cvref_t<Message>;

			using fun_traits_type = function_traits<FunctionT>;
			using arg0_type = typename std::remove_cv_t<std::remove_reference_t<
				typename fun_traits_type::template args<0>::type>>;

			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io_->running_in_this_thread());

			mqtt::v5::properties_set props;

			if constexpr (std::is_same_v<message_type, mqtt::v5::subscribe>)
			{
				props = msg.properties();
			}
			else
			{
				std::ignore = true;
			}

			std::vector<mqtt::subscription>& subs = msg.subscriptions().data();

			for (std::size_t i = 0; i < subs.size(); ++i)
			{
				mqtt::subscription& sub = subs[i];

				bool end = (i + 1 == subs.size());

				subnode_type node{ derive.selfptr(), sub, end ? std::move(props) : props };

				if constexpr (std::is_same_v<arg0_type, mqtt::message>)
				{
					node.callback = end ? std::forward<FunctionT>(callback) : callback;
				}
				else
				{
					node.callback = [cb = end ? std::forward<FunctionT>(callback) : callback]
					(mqtt::message& msg) mutable
					{
						arg0_type* p = std::get_if<arg0_type>(std::addressof(msg.base()));
						if (p)
						{
							cb(*p);
						}
					};
				}

				std::string_view share_name   = node.share_name();
				std::string_view topic_filter = node.topic_filter();

				auto[_1, inserted] = this->subs_map().insert_or_assign(topic_filter, "", std::move(node));

				asio2::ignore_unused(share_name, topic_filter, _1, inserted);
			}

			derive.async_send(std::forward<Message>(msg));
		}

		inline mqtt::subscription_map<std::string_view, subnode_type>& subs_map() { return subs_map_; }

	protected:
		/// subscription information map
		mqtt::subscription_map<std::string_view, subnode_type>      subs_map_;

		/// don't need mutex, beacuse client only has one thread, we use post to ensure this
		/// variable was read write in the client io_context thread.
		std::unordered_map<key_type, mqtt::utf8_string_set, hasher> unsubscribed_topics_;
	};
}

#endif // !__ASIO2_MQTT_SUBSCRIBE_ROUTER_HPP__
