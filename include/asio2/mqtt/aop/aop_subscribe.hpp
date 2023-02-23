/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_MQTT_AOP_SUBSCRIBE_HPP__
#define __ASIO2_MQTT_AOP_SUBSCRIBE_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/iopool.hpp>

#include <asio2/base/detail/function_traits.hpp>
#include <asio2/base/detail/util.hpp>

#include <asio2/mqtt/message.hpp>

namespace asio2::detail
{
	template<class caller_t, class args_t>
	class mqtt_aop_subscribe
	{
		friend caller_t;

	protected:
		template<class Message>
		inline mqtt::v5::properties_set _check_subscribe_properties(error_code& ec, Message& msg)
		{
			using message_type = typename detail::remove_cvref_t<Message>;

			if constexpr (std::is_same_v<message_type, mqtt::v5::subscribe>)
			{
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
					}
					else
					{
						std::ignore = true;
					}
				}

				return msg.properties();
			}
			else
			{
				return {};
			}
		}

		// must be server
		template<class Message, class Response, bool IsClient = args_t::is_client>
		inline std::enable_if_t<!IsClient, void>
		_before_subscribe_callback(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			Message& msg, Response& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);

			using message_type = typename detail::remove_cvref_t<Message>;

			bool is_v5 = std::is_same_v<message_type, mqtt::v5::subscribe>;

			// A SUBACK and UNSUBACK MUST contain the Packet Identifier that was used in the
			// corresponding SUBSCRIBE and UNSUBSCRIBE packet respectively [MQTT-2.2.1-6].
			rep.packet_id(msg.packet_id());

			// subscription properties
			mqtt::v5::properties_set props = _check_subscribe_properties(ec, msg);

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
					continue;
				}

				mqtt::qos_type qos = sub.qos();

				// error, the session will be disconnect a later
				if (!mqtt::is_valid_qos(qos))
				{
					ec = mqtt::make_error_code(mqtt::error::unspecified_error);
					rep.add_reason_codes(detail::to_underlying(mqtt::error::unspecified_error));
					continue;
				}

				// not error, but not supported, and the session will not be disconnect
				if (detail::to_underlying(qos) > caller->maximum_qos())
				{
					ec = mqtt::make_error_code(mqtt::error::qos_not_supported);
					rep.add_reason_codes(detail::to_underlying(is_v5 ?
						mqtt::error::quota_exceeded : mqtt::error::unspecified_error));
					continue;
				}

				// not error, and supported too
				rep.add_reason_codes(detail::to_underlying(qos));

				typename caller_t::subnode_type node{ caller_ptr, sub, std::move(props) };

				std::string_view share_name   = node.share_name();
				std::string_view topic_filter = node.topic_filter();

				if (!share_name.empty())
				{
					caller->shared_targets().insert(caller_ptr, share_name, topic_filter);
				}

				bool inserted = caller->subs_map().insert_or_assign(
					topic_filter, caller->client_id(), std::move(node)).second;

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
		}

		template<class Message, class Response, bool IsClient = args_t::is_client>
		inline std::enable_if_t<IsClient, void>
		_before_subscribe_callback(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			Message& msg, Response& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);

			ASIO2_ASSERT(false && "client should't recv the subscribe message");
		}

		inline void _send_retained_messages(
			std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::subscription& sub)
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

				caller->retained_messages().find(topic_filter, [caller_ptr, caller, &sub, &props]
				(mqtt::rmnode& node) mutable
				{
					std::visit([caller_ptr, caller, &sub, &props](auto& pub) mutable
					{
						using T = asio2::detail::remove_cvref_t<decltype(pub)>;
						if constexpr (mqtt::is_publish_message<T>())
						{
							caller->_send_publish_to_subscriber(caller_ptr, sub, props, pub);
						}
						else
						{
							ASIO2_ASSERT(false);
						}
					}, node.message.base());
				});
			});
		}

		// must be server
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v3::subscribe& msg, mqtt::v3::suback& rep)
		{
			if (_before_subscribe_callback(ec, caller_ptr, caller, om, msg, rep); ec)
				return;
		}

		// must be server
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v4::subscribe& msg, mqtt::v4::suback& rep)
		{
			if (_before_subscribe_callback(ec, caller_ptr, caller, om, msg, rep); ec)
				return;
		}

		// must be server
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v5::subscribe& msg, mqtt::v5::suback& rep)
		{
			if (_before_subscribe_callback(ec, caller_ptr, caller, om, msg, rep); ec)
				return;
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v3::subscribe& msg, mqtt::v3::suback& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v4::subscribe& msg, mqtt::v4::suback& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v5::subscribe& msg, mqtt::v5::suback& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);
		}
	};
}

#endif // !__ASIO2_MQTT_AOP_SUBSCRIBE_HPP__
