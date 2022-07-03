/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_MQTT_AOP_PINGRESP_HPP__
#define __ASIO2_MQTT_AOP_PINGRESP_HPP__

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
	class mqtt_aop_pingresp
	{
		friend caller_t;

	protected:
		// must be client
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v3::pingresp& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg);
		}

		// must be client
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v4::pingresp& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg);
		}

		// must be client
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v5::pingresp& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg);
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v3::pingresp& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg);
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v4::pingresp& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg);
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v5::pingresp& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg);
		}
	};
}

#endif // !__ASIO2_MQTT_AOP_PINGRESP_HPP__
