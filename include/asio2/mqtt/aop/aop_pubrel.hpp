/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_MQTT_AOP_PUBREL_HPP__
#define __ASIO2_MQTT_AOP_PUBREL_HPP__

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
	class mqtt_aop_pubrel
	{
		friend caller_t;

	protected:
		// server or client
		template<class Message, class Response>
		inline void _before_pubrel_callback(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			Message& msg, Response& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);

			using message_type  [[maybe_unused]] = typename detail::remove_cvref_t<Message>;
			using response_type [[maybe_unused]] = typename detail::remove_cvref_t<Response>;

			rep.packet_id(msg.packet_id());

			// PUBCOMP Reason Code
			// https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901154
			if constexpr (std::is_same_v<response_type, mqtt::v5::pubcomp>)
			{
				rep.reason_code(detail::to_underlying(mqtt::error::success));
			}
			else
			{
				std::ignore = true;
			}
		}

		// server or client
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v3::pubrel& msg, asio2::mqtt::v3::pubcomp& rep)
		{
			if (_before_pubrel_callback(ec, caller_ptr, caller, om, msg, rep); ec)
				return;
		}

		// server or client
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v4::pubrel& msg, asio2::mqtt::v4::pubcomp& rep)
		{
			if (_before_pubrel_callback(ec, caller_ptr, caller, om, msg, rep); ec)
				return;
		}

		// server or client
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v5::pubrel& msg, asio2::mqtt::v5::pubcomp& rep)
		{
			if (_before_pubrel_callback(ec, caller_ptr, caller, om, msg, rep); ec)
				return;
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v3::pubrel& msg, asio2::mqtt::v3::pubcomp& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v4::pubrel& msg, asio2::mqtt::v4::pubcomp& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v5::pubrel& msg, asio2::mqtt::v5::pubcomp& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);
		}
	};
}

#endif // !__ASIO2_MQTT_AOP_PUBREL_HPP__
