/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_MQTT_AOP_UNSUBACK_HPP__
#define __ASIO2_MQTT_AOP_UNSUBACK_HPP__

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
	class mqtt_aop_unsuback
	{
		friend caller_t;

	protected:
		// must be server
		template<class Message, bool IsClient = args_t::is_client>
		inline std::enable_if_t<!IsClient, void>
		_after_unsuback_callback(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			Message& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg);

			ASIO2_ASSERT(false && "server should't recv the unsuback message");

			// if server recvd unsuback message, disconnect
			ec = mqtt::make_error_code(mqtt::error::malformed_packet);
		}

		template<class Message, bool IsClient = args_t::is_client>
		inline std::enable_if_t<IsClient, void>
		_after_unsuback_callback(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			Message& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg);

			using message_type  [[maybe_unused]] = typename detail::remove_cvref_t<Message>;
			using key_type = std::pair<mqtt::control_packet_type, mqtt::two_byte_integer::value_type>;

			key_type key{ mqtt::control_packet_type::unsubscribe, msg.packet_id() };

			if (auto it = caller->unsubscribed_topics_.find(key); it != caller->unsubscribed_topics_.end())
			{
				mqtt::utf8_string_set& topics = it->second;

				topics.for_each([caller](mqtt::utf8_string& topic) mutable
				{
					caller->subs_map().erase(topic, "");
				});

				caller->unsubscribed_topics_.erase(it);
			}
		}

		// must be client
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v3::unsuback& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg);
		}

		// must be client
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v4::unsuback& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg);
		}

		// must be client
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v5::unsuback& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg);
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v3::unsuback& msg)
		{
			if (_after_unsuback_callback(ec, caller_ptr, caller, om, msg); ec)
				return;
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v4::unsuback& msg)
		{
			if (_after_unsuback_callback(ec, caller_ptr, caller, om, msg); ec)
				return;
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v5::unsuback& msg)
		{
			if (_after_unsuback_callback(ec, caller_ptr, caller, om, msg); ec)
				return;
		}
	};
}

#endif // !__ASIO2_MQTT_AOP_UNSUBACK_HPP__
