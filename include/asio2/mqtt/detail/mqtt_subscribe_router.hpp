/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_MQTT_SUBSCRIBE_ROUTER_HPP__
#define __ASIO2_MQTT_SUBSCRIBE_ROUTER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/iopool.hpp>
#include <asio2/base/define.hpp>

#include <asio2/external/magic_enum.hpp>

#include <asio2/base/detail/function_traits.hpp>
#include <asio2/base/detail/util.hpp>

#include <asio2/mqtt/detail/mqtt_topic_util.hpp>
#include <asio2/mqtt/detail/mqtt_subscription_map.hpp>
#include <asio2/mqtt/detail/mqtt_shared_target.hpp>
#include <asio2/mqtt/detail/mqtt_retained_message.hpp>

#include <asio2/mqtt/message.hpp>

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

		/**
		 * @constructor
		 */
		mqtt_subscribe_router_t() = default;

		/**
		 * @destructor
		 */
		~mqtt_subscribe_router_t() = default;

		template<class ReturnT = void, class IntOrQos, class FunctionT>
		typename std::enable_if_t<
			std::is_same_v<detail::remove_cvref_t<IntOrQos>, mqtt::qos_type> ||
			std::is_integral_v<detail::remove_cvref_t<IntOrQos>>>
		subscribe(std::string topic_filter, IntOrQos qos, FunctionT&& callback)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			mqtt::version ver = derive.version();

			auto pid = derive.idmgr_.get();

			if /**/ constexpr (std::is_same_v<ReturnT, void>)
			{
				if (!mqtt::is_valid_qos(qos))
				{
					ASIO2_ASSERT(false);
					return;
				}

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
					ASIO2_ASSERT(false);
					return;
				}

			}
			else if constexpr (std::is_same_v<ReturnT, bool>)
			{

			}
			else if constexpr (std::is_same_v<ReturnT, mqtt::message>)
			{

			}
			else
			{
				static_assert(false);
			}
		}

		template<class ReturnT = void, class Message, class FunctionT>
		typename std::enable_if_t<
			std::is_same_v<detail::remove_cvref_t<Message>, mqtt::v3::subscribe> || 
			std::is_same_v<detail::remove_cvref_t<Message>, mqtt::v4::subscribe> ||
			std::is_same_v<detail::remove_cvref_t<Message>, mqtt::v5::subscribe>>
		subscribe(Message&& msg, FunctionT&& callback)
		{
			using message_type = typename detail::remove_cvref_t<Message>;

			using fun_traits_type = function_traits<FunctionT>;
			using arg0_type = typename std::remove_cv_t<std::remove_reference_t<
				typename fun_traits_type::template args<0>::type>>;

			derived_t& derive = static_cast<derived_t&>(*this);

			mqtt::v5::properties_set props;

			if constexpr (std::is_same_v<message_type, mqtt::v5::subscribe>)
			{
				props = msg.properties();
			}
			else
			{
				std::ignore = true;
			}

			if /**/ constexpr (std::is_same_v<ReturnT, void>)
			{
				for (mqtt::subscription& sub : msg.subscriptions().data())
				{
					if (!mqtt::is_valid_qos(sub.qos()))
					{
						ASIO2_ASSERT(false);
						return;
					}
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

					auto[_1, inserted] = this->subs_map_.emplace(topic_filter, "", std::move(node));

					asio2::ignore_unused(_1, inserted);
				}

				derive.async_send(std::forward<Message>(msg), []()
				{
				});

				

			}
			else if constexpr (std::is_same_v<ReturnT, bool>)
			{

			}
			else if constexpr (std::is_same_v<ReturnT, mqtt::message>)
			{

			}
			else
			{
				static_assert(false);
			}
		}

	protected:
		/// subscription information map
		mqtt::subscription_map<std::string_view, subnode_type> subs_map_;
	};
}

#endif // !__ASIO2_MQTT_SUBSCRIBE_ROUTER_HPP__
