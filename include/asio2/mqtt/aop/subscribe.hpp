/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_MQTT_AOP_SUBSCRIBE_HPP__
#define __ASIO2_MQTT_AOP_SUBSCRIBE_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/iopool.hpp>

#include <asio2/base/detail/function_traits.hpp>
#include <asio2/base/detail/util.hpp>

#include <asio2/mqtt/message_util.hpp>

namespace asio2::detail
{
	template<class caller_t>
	class mqtt_aop_subscribe
	{
		friend caller_t;

	protected:
		// must be server
		template<class Message, class Response>
		inline bool _before_subscribe_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, Message& msg, Response& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, rep);

			using message_type = typename detail::remove_cvref_t<Message>;

			if constexpr (caller_t::is_session())
			{
				bool flag = true;

				constexpr bool is_v5 = std::is_same_v<message_type, mqtt::v5::subscribe>;

				// A SUBACK and UNSUBACK MUST contain the Packet Identifier that was used in the
				// corresponding SUBSCRIBE and UNSUBSCRIBE packet respectively [MQTT-2.2.1-6].
				rep.packet_id(msg.packet_id());

				// subscription properties
				mqtt::v5::properties_set props;

				if constexpr (std::is_same_v<message_type, mqtt::v5::subscribe>)
				{
					props = msg.properties();

					// Get subscription identifier
					mqtt::v5::subscription_identifier* sub_id =
						msg.properties().template get_if<mqtt::v5::subscription_identifier>();
					if (sub_id)
					{
						// The Subscription Identifier can have the value of 1 to 268,435,455.
						// It is a Protocol Error if the Subscription Identifier has a value of 0
						auto v = sub_id->value();
						if (v < 1 || v > 268435455)
						{
							ASIO2_ASSERT(false);
							ec = mqtt::make_error_code(mqtt::error::protocol_error);
							flag = false;
						}
						else
						{
							std::ignore = true;
						}
					}
				}
				else
				{
					std::ignore = true;
				}

				for (mqtt::subscription& sub : msg.subscriptions().data())
				{
					// Reason Codes
					// https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901178

					// error, the session will be disconnect a later
					if (sub.topic_filter().empty())
					{
						ec = mqtt::make_error_code(mqtt::error::topic_filter_invalid);
						rep.add_reason_codes(detail::to_underlying(is_v5 ?
							mqtt::error::topic_filter_invalid : mqtt::error::unspecified_error));
						flag = false;
						continue;
					}

					mqtt::qos_type qos = sub.qos();

					// error, the session will be disconnect a later
					if (detail::to_underlying(qos) > 2)
					{
						ec = mqtt::make_error_code(mqtt::error::unspecified_error);
						rep.add_reason_codes(detail::to_underlying(mqtt::error::unspecified_error));
						flag = false;
						continue;
					}

					// not error, but not supported, and the session will not be disconnect
					if (detail::to_underlying(qos) > caller->maximum_qos())
					{
						ec = mqtt::make_error_code(mqtt::error::qos_not_supported);
						rep.add_reason_codes(detail::to_underlying(is_v5 ?
							mqtt::error::quota_exceeded : mqtt::error::unspecified_error));
						flag = false;
						continue;
					}

					// not error, and supported too
					rep.add_reason_codes(detail::to_underlying(qos));

					mqtt::subnode<caller_t> node{ caller_ptr, sub, std::move(props) };

					std::string_view share_name   = node.share_name();
					std::string_view topic_filter = node.topic_filter();

					if (!share_name.empty())
					{
						caller->shared_targets_.insert(caller_ptr, share_name, topic_filter);
					}

					auto[_1, inserted] = caller->subs_map_.emplace(topic_filter, caller->client_id(), std::move(node));

					asio2::ignore_unused(_1, inserted);

					mqtt::retain_handling_type rh = sub.retain_handling();

					if /**/ (rh == mqtt::retain_handling_type::send)
					{
						_send_retained_messages(caller_ptr, caller, sub);
					}
					else if (rh == mqtt::retain_handling_type::send_only_new_subscription)
					{
						if (inserted)
						{
							_send_retained_messages(caller_ptr, caller, sub);
						}
					}
				}

				return flag;
			}
			else
			{
				return true;
			}
		}

		inline void _send_retained_messages(std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::subscription& sub)
		{
			detail::ignore_unused(caller_ptr, caller, sub);

			// use push_event to ensure the publish message is sent to clients must after mqtt 
			// response is sent already.
			caller->push_event(
			[caller_ptr, caller, sub = std::move(sub), topic_filter = std::string{ sub.topic_filter() }]
			(event_queue_guard<caller_t> g) mutable
			{
				detail::ignore_unused(g);

				mqtt::v5::properties_set props;

				caller->retained_messages_.find(topic_filter, [caller_ptr, caller, &sub, &props]
				(mqtt::rmnode& node) mutable
				{
					std::visit([caller_ptr, caller, &sub, &props](auto& pub) mutable
					{
						caller->_send_publish_to_subscriber(caller_ptr, sub, props, pub);
					}, node.message);
				});
			});
		}

		// must be server
		inline void _before_user_callback_impl(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::subscribe& msg, mqtt::v3::suback& rep)
		{
			if (!_before_subscribe_callback(ec, caller_ptr, caller, msg, rep))
				return;


		}

		// must be server
		inline void _before_user_callback_impl(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::subscribe& msg, mqtt::v4::suback& rep)
		{
			if (!_before_subscribe_callback(ec, caller_ptr, caller, msg, rep))
				return;
		}

		// must be server
		inline void _before_user_callback_impl(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::subscribe& msg, mqtt::v5::suback& rep)
		{
			if (!_before_subscribe_callback(ec, caller_ptr, caller, msg, rep))
				return;
		}

		inline void _after_user_callback_impl(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::subscribe& msg, mqtt::v3::suback& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, rep);
		}

		inline void _after_user_callback_impl(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::subscribe& msg, mqtt::v4::suback& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, rep);
		}

		inline void _after_user_callback_impl(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::subscribe& msg, mqtt::v5::suback& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, rep);
		}
	};
}

#endif // !__ASIO2_MQTT_AOP_SUBSCRIBE_HPP__
