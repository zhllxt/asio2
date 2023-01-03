/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_MQTT_AOP_PINGREQ_HPP__
#define __ASIO2_MQTT_AOP_PINGREQ_HPP__

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
	class mqtt_aop_pingreq
	{
		friend caller_t;

	protected:
		// must be server
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v3::pingreq& msg, mqtt::v3::pingresp& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);
		}

		// must be server
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v4::pingreq& msg, mqtt::v4::pingresp& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);
		}

		// must be server
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v5::pingreq& msg, mqtt::v5::pingresp& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v3::pingreq& msg, mqtt::v3::pingresp& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v4::pingreq& msg, mqtt::v4::pingresp& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v5::pingreq& msg, mqtt::v5::pingresp& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);
		}
	};
}

#endif // !__ASIO2_MQTT_AOP_PINGREQ_HPP__
