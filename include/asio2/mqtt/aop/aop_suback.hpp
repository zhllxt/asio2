/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_MQTT_AOP_SUBACK_HPP__
#define __ASIO2_MQTT_AOP_SUBACK_HPP__

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
	class mqtt_aop_suback
	{
		friend caller_t;

	protected:
		// server or client
		template<class Message>
		inline void _before_suback_callback(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			Message& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg);

			using message_type [[maybe_unused]] = typename detail::remove_cvref_t<Message>;

			if (msg.reason_codes().count() == 0)
			{
				ec = mqtt::make_error_code(mqtt::error::malformed_packet);
			}
		}

		// must be client
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v3::suback& msg)
		{
			if (_before_suback_callback(ec, caller_ptr, caller, om, msg); ec)
				return;
		}

		// must be client
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v4::suback& msg)
		{
			if (_before_suback_callback(ec, caller_ptr, caller, om, msg); ec)
				return;
		}

		// must be client
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v5::suback& msg)
		{
			if (_before_suback_callback(ec, caller_ptr, caller, om, msg); ec)
				return;
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v3::suback& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg);
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v4::suback& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg);
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v5::suback& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg);
		}
	};
}

#endif // !__ASIO2_MQTT_AOP_SUBACK_HPP__
