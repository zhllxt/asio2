/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_MQTT_AOP_CONNACK_HPP__
#define __ASIO2_MQTT_AOP_CONNACK_HPP__

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
	class mqtt_aop_connack
	{
		friend caller_t;

	protected:
		template<class Message, bool IsClient = args_t::is_client>
		inline std::enable_if_t<!IsClient, void>
		_before_connack_callback(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			Message& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg);

			// if server recvd connack message, disconnect
			ec = mqtt::make_error_code(mqtt::error::malformed_packet);
		}

		template<class Message, bool IsClient = args_t::is_client>
		inline std::enable_if_t<IsClient, void>
		_before_connack_callback(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			Message& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg);

			// if started already and recvd connack message again, disconnect
			state_t expected = state_t::started;
			if (caller->state_.compare_exchange_strong(expected, state_t::started))
			{
				ec = mqtt::make_error_code(mqtt::error::malformed_packet);
				return;
			}

			caller->connack_message_ = msg;
		}

		// must be client
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v3::connack& msg)
		{
			if (_before_connack_callback(ec, caller_ptr, caller, om, msg); ec)
				return;

			switch(msg.reason_code())
			{
			case mqtt::v3::connect_reason_code::success                       : ec = mqtt::make_error_code(mqtt::error::success                     ); break;
			case mqtt::v3::connect_reason_code::unacceptable_protocol_version : ec = mqtt::make_error_code(mqtt::error::unsupported_protocol_version); break;
			case mqtt::v3::connect_reason_code::identifier_rejected			  : ec = mqtt::make_error_code(mqtt::error::client_identifier_not_valid ); break;
			case mqtt::v3::connect_reason_code::server_unavailable			  : ec = mqtt::make_error_code(mqtt::error::server_unavailable          ); break;
			case mqtt::v3::connect_reason_code::bad_user_name_or_password	  : ec = mqtt::make_error_code(mqtt::error::bad_user_name_or_password   ); break;
			case mqtt::v3::connect_reason_code::not_authorized				  : ec = mqtt::make_error_code(mqtt::error::not_authorized              ); break;
			default                                                           : ec = mqtt::make_error_code(mqtt::error::malformed_packet            ); break;
			}
		}

		// must be client
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v4::connack& msg)
		{
			if (_before_connack_callback(ec, caller_ptr, caller, om, msg); ec)
				return;

			switch(msg.reason_code())
			{
			case mqtt::v4::connect_reason_code::success						  : ec = mqtt::make_error_code(mqtt::error::success                     ); break;
			case mqtt::v4::connect_reason_code::unacceptable_protocol_version : ec = mqtt::make_error_code(mqtt::error::unsupported_protocol_version); break;
			case mqtt::v4::connect_reason_code::identifier_rejected			  : ec = mqtt::make_error_code(mqtt::error::client_identifier_not_valid ); break;
			case mqtt::v4::connect_reason_code::server_unavailable			  : ec = mqtt::make_error_code(mqtt::error::server_unavailable          ); break;
			case mqtt::v4::connect_reason_code::bad_user_name_or_password	  : ec = mqtt::make_error_code(mqtt::error::bad_user_name_or_password   ); break;
			case mqtt::v4::connect_reason_code::not_authorized				  : ec = mqtt::make_error_code(mqtt::error::not_authorized              ); break;
			default                                                           : ec = mqtt::make_error_code(mqtt::error::malformed_packet            ); break;
			}
		}

		// must be client
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v5::connack& msg)
		{
			if (_before_connack_callback(ec, caller_ptr, caller, om, msg); ec)
				return;

			ec = mqtt::make_error_code(static_cast<mqtt::error>(msg.reason_code()));
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v3::connack& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg);

			// if already has error, return
			if (ec)
				return;

			switch(msg.reason_code())
			{
			case mqtt::v3::connect_reason_code::success                       : ec = mqtt::make_error_code(mqtt::error::success                     ); break;
			case mqtt::v3::connect_reason_code::unacceptable_protocol_version : ec = mqtt::make_error_code(mqtt::error::unsupported_protocol_version); break;
			case mqtt::v3::connect_reason_code::identifier_rejected			  : ec = mqtt::make_error_code(mqtt::error::client_identifier_not_valid ); break;
			case mqtt::v3::connect_reason_code::server_unavailable			  : ec = mqtt::make_error_code(mqtt::error::server_unavailable          ); break;
			case mqtt::v3::connect_reason_code::bad_user_name_or_password	  : ec = mqtt::make_error_code(mqtt::error::bad_user_name_or_password   ); break;
			case mqtt::v3::connect_reason_code::not_authorized				  : ec = mqtt::make_error_code(mqtt::error::not_authorized              ); break;
			default                                                           : ec = mqtt::make_error_code(mqtt::error::malformed_packet            ); break;
			}
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v4::connack& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg);

			// if already has error, return
			if (ec)
				return;

			switch(msg.reason_code())
			{
			case mqtt::v4::connect_reason_code::success						  : ec = mqtt::make_error_code(mqtt::error::success                     ); break;
			case mqtt::v4::connect_reason_code::unacceptable_protocol_version : ec = mqtt::make_error_code(mqtt::error::unsupported_protocol_version); break;
			case mqtt::v4::connect_reason_code::identifier_rejected			  : ec = mqtt::make_error_code(mqtt::error::client_identifier_not_valid ); break;
			case mqtt::v4::connect_reason_code::server_unavailable			  : ec = mqtt::make_error_code(mqtt::error::server_unavailable          ); break;
			case mqtt::v4::connect_reason_code::bad_user_name_or_password	  : ec = mqtt::make_error_code(mqtt::error::bad_user_name_or_password   ); break;
			case mqtt::v4::connect_reason_code::not_authorized				  : ec = mqtt::make_error_code(mqtt::error::not_authorized              ); break;
			default                                                           : ec = mqtt::make_error_code(mqtt::error::malformed_packet            ); break;
			}
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v5::connack& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg);

			// if already has error, return
			if (ec)
				return;

			ec = mqtt::make_error_code(static_cast<mqtt::error>(msg.reason_code()));
		}
	};
}

#endif // !__ASIO2_MQTT_AOP_CONNACK_HPP__
