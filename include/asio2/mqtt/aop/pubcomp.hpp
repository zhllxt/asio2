/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_MQTT_AOP_PUBCOMP_HPP__
#define __ASIO2_MQTT_AOP_PUBCOMP_HPP__

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
	class mqtt_aop_pubcomp
	{
		friend caller_t;

	protected:
		// server or client
		inline void _before_user_callback_impl(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::pubcomp& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		// server or client
		inline void _before_user_callback_impl(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::pubcomp& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		// server or client
		inline void _before_user_callback_impl(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::pubcomp& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);

		}

		inline void _after_user_callback_impl(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::pubcomp& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);

		}

		inline void _after_user_callback_impl(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::pubcomp& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		inline void _after_user_callback_impl(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::pubcomp& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}
	};
}

#endif // !__ASIO2_MQTT_AOP_PUBCOMP_HPP__
