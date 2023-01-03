/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_MQTT_HANDLER_HPP__
#define __ASIO2_MQTT_HANDLER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/base/define.hpp>
#include <asio2/base/error.hpp>

#include <asio2/mqtt/detail/mqtt_topic_util.hpp>

#include <asio2/mqtt/detail/mqtt_subscription_map.hpp>
#include <asio2/mqtt/detail/mqtt_shared_target.hpp>
#include <asio2/mqtt/detail/mqtt_retained_message.hpp>
#include <asio2/mqtt/detail/mqtt_offline_message.hpp>

#include <asio2/mqtt/aop/aop_auth.hpp>
#include <asio2/mqtt/aop/aop_connack.hpp>
#include <asio2/mqtt/aop/aop_connect.hpp>
#include <asio2/mqtt/aop/aop_disconnect.hpp>
#include <asio2/mqtt/aop/aop_pingreq.hpp>
#include <asio2/mqtt/aop/aop_pingresp.hpp>
#include <asio2/mqtt/aop/aop_puback.hpp>
#include <asio2/mqtt/aop/aop_pubcomp.hpp>
#include <asio2/mqtt/aop/aop_publish.hpp>
#include <asio2/mqtt/aop/aop_pubrec.hpp>
#include <asio2/mqtt/aop/aop_pubrel.hpp>
#include <asio2/mqtt/aop/aop_suback.hpp>
#include <asio2/mqtt/aop/aop_subscribe.hpp>
#include <asio2/mqtt/aop/aop_unsuback.hpp>
#include <asio2/mqtt/aop/aop_unsubscribe.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;

	template<class caller_t, class args_t>
	class mqtt_handler_t
		: public mqtt_aop_auth       <caller_t, args_t>
		, public mqtt_aop_connack    <caller_t, args_t>
		, public mqtt_aop_connect    <caller_t, args_t>
		, public mqtt_aop_disconnect <caller_t, args_t>
		, public mqtt_aop_pingreq    <caller_t, args_t>
		, public mqtt_aop_pingresp   <caller_t, args_t>
		, public mqtt_aop_puback     <caller_t, args_t>
		, public mqtt_aop_pubcomp    <caller_t, args_t>
		, public mqtt_aop_publish    <caller_t, args_t>
		, public mqtt_aop_pubrec     <caller_t, args_t>
		, public mqtt_aop_pubrel     <caller_t, args_t>
		, public mqtt_aop_suback     <caller_t, args_t>
		, public mqtt_aop_subscribe  <caller_t, args_t>
		, public mqtt_aop_unsuback   <caller_t, args_t>
		, public mqtt_aop_unsubscribe<caller_t, args_t>
	{
		friend caller_t;

		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;

	protected:
		using mqtt_aop_auth       <caller_t, args_t>::_before_user_callback_impl;
		using mqtt_aop_connack    <caller_t, args_t>::_before_user_callback_impl;
		using mqtt_aop_connect    <caller_t, args_t>::_before_user_callback_impl;
		using mqtt_aop_disconnect <caller_t, args_t>::_before_user_callback_impl;
		using mqtt_aop_pingreq    <caller_t, args_t>::_before_user_callback_impl;
		using mqtt_aop_pingresp   <caller_t, args_t>::_before_user_callback_impl;
		using mqtt_aop_puback     <caller_t, args_t>::_before_user_callback_impl;
		using mqtt_aop_pubcomp    <caller_t, args_t>::_before_user_callback_impl;
		using mqtt_aop_publish    <caller_t, args_t>::_before_user_callback_impl;
		using mqtt_aop_pubrec     <caller_t, args_t>::_before_user_callback_impl;
		using mqtt_aop_pubrel     <caller_t, args_t>::_before_user_callback_impl;
		using mqtt_aop_suback     <caller_t, args_t>::_before_user_callback_impl;
		using mqtt_aop_subscribe  <caller_t, args_t>::_before_user_callback_impl;
		using mqtt_aop_unsuback   <caller_t, args_t>::_before_user_callback_impl;
		using mqtt_aop_unsubscribe<caller_t, args_t>::_before_user_callback_impl;

		using mqtt_aop_auth       <caller_t, args_t>::_after_user_callback_impl;
		using mqtt_aop_connack    <caller_t, args_t>::_after_user_callback_impl;
		using mqtt_aop_connect    <caller_t, args_t>::_after_user_callback_impl;
		using mqtt_aop_disconnect <caller_t, args_t>::_after_user_callback_impl;
		using mqtt_aop_pingreq    <caller_t, args_t>::_after_user_callback_impl;
		using mqtt_aop_pingresp   <caller_t, args_t>::_after_user_callback_impl;
		using mqtt_aop_puback     <caller_t, args_t>::_after_user_callback_impl;
		using mqtt_aop_pubcomp    <caller_t, args_t>::_after_user_callback_impl;
		using mqtt_aop_publish    <caller_t, args_t>::_after_user_callback_impl;
		using mqtt_aop_pubrec     <caller_t, args_t>::_after_user_callback_impl;
		using mqtt_aop_pubrel     <caller_t, args_t>::_after_user_callback_impl;
		using mqtt_aop_suback     <caller_t, args_t>::_after_user_callback_impl;
		using mqtt_aop_subscribe  <caller_t, args_t>::_after_user_callback_impl;
		using mqtt_aop_unsuback   <caller_t, args_t>::_after_user_callback_impl;
		using mqtt_aop_unsubscribe<caller_t, args_t>::_after_user_callback_impl;

		template<class Message>
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om, Message& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg);

			ASIO2_ASSERT(false);
		}

		template<class Message>
		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om, Message& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg);

			ASIO2_ASSERT(false);
		}

		template<class Message, class Response>
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om, Message& msg, Response& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);

			ASIO2_ASSERT(false);
		}

		template<class Message, class Response>
		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om, Message& msg, Response& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);

			ASIO2_ASSERT(false);
		}

		template<class Message>
		inline void _before_user_callback(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om, Message& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg);

			using message_type  [[maybe_unused]] = typename detail::remove_cvref_t<Message>;

			if constexpr (std::is_same_v<message_type, mqtt::message>)
			{
				std::visit([&ec, &caller_ptr, &caller, &om](auto& pm) mutable
				{
					caller->_before_user_callback_impl(ec, caller_ptr, caller, om, pm);
				}, msg.variant());
			}
			else
			{
				caller->_before_user_callback_impl(ec, caller_ptr, caller, om, msg);
			}
		}

		template<class Message>
		inline void _after_user_callback(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om, Message& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg);

			using message_type  [[maybe_unused]] = typename detail::remove_cvref_t<Message>;

			if constexpr (std::is_same_v<message_type, mqtt::message>)
			{
				std::visit([&ec, &caller_ptr, &caller, &om](auto& pm) mutable
				{
					caller->_after_user_callback_impl(ec, caller_ptr, caller, om, pm);
				}, msg.variant());
			}
			else
			{
				caller->_after_user_callback_impl(ec, caller_ptr, caller, om, msg);
			}
		}

		template<class Message, class Response>
		inline void _before_user_callback_with_message(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om, Message& msg, Response& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);

			using message_type  [[maybe_unused]] = typename detail::remove_cvref_t<Message>;
			using response_type [[maybe_unused]] = typename detail::remove_cvref_t<Response>;

			if constexpr (std::is_same_v<response_type, mqtt::message>)
			{
				if (!rep.empty())
				{
					std::visit([&ec, &caller_ptr, &caller, &om, &rep](auto& pm) mutable
					{
						std::visit([&ec, &caller_ptr, &caller, &om, &pm](auto& pr) mutable
						{
							caller->_before_user_callback_impl(ec, caller_ptr, caller, om, pm, pr);
						}, rep.variant());
					}, msg.variant());
				}
			}
			else
			{
				std::visit([&ec, &caller_ptr, &caller, &om, &rep](auto& pm) mutable
				{
					caller->_before_user_callback_impl(ec, caller_ptr, caller, om, pm, rep);
				}, msg.variant());
			}
		}

		template<class Message, class Response>
		inline void _before_user_callback_with_packet(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om, Message& msg, Response& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);

			using message_type  [[maybe_unused]] = typename detail::remove_cvref_t<Message>;
			using response_type [[maybe_unused]] = typename detail::remove_cvref_t<Response>;

			if constexpr (std::is_same_v<response_type, mqtt::message>)
			{
				if (!rep.empty())
				{
					std::visit([&ec, &caller_ptr, &caller, &om, &msg](auto& pr) mutable
					{
						caller->_before_user_callback_impl(ec, caller_ptr, caller, om, msg, pr);
					}, rep.variant());
				}
			}
			else
			{
				caller->_before_user_callback_impl(ec, caller_ptr, caller, om, msg, rep);
			}
		}

		template<class Message, class Response>
		inline void _before_user_callback(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om, Message& msg, Response& rep)
		{
			using message_type  [[maybe_unused]] = typename detail::remove_cvref_t<Message>;
			using response_type [[maybe_unused]] = typename detail::remove_cvref_t<Response>;

			if constexpr (std::is_same_v<message_type, mqtt::message>)
			{
				this->_before_user_callback_with_message(ec, caller_ptr, caller, om, msg, rep);
			}
			else
			{
				this->_before_user_callback_with_packet(ec, caller_ptr, caller, om, msg, rep);
			}
		}

		template<class Message, class Response>
		inline void _after_user_callback_with_message(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om, Message& msg, Response& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);

			using message_type  [[maybe_unused]] = typename detail::remove_cvref_t<Message>;
			using response_type [[maybe_unused]] = typename detail::remove_cvref_t<Response>;

			if constexpr (std::is_same_v<response_type, mqtt::message>)
			{
				if (!rep.empty())
				{
					std::visit([&ec, &caller_ptr, &caller, &om, &rep](auto& pm) mutable
					{
						std::visit([&ec, &caller_ptr, &caller, &om, &pm](auto& pr) mutable
						{
							caller->_after_user_callback_impl(ec, caller_ptr, caller, om, pm, pr);
						}, rep.variant());
					}, msg.variant());
				}
			}
			else
			{
				std::visit([&ec, &caller_ptr, &caller, &om, &rep](auto& pm) mutable
				{
					caller->_after_user_callback_impl(ec, caller_ptr, caller, om, pm, rep);
				}, msg.variant());
			}
		}

		template<class Message, class Response>
		inline void _after_user_callback_with_packet(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om, Message& msg, Response& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);

			using message_type  [[maybe_unused]] = typename detail::remove_cvref_t<Message>;
			using response_type [[maybe_unused]] = typename detail::remove_cvref_t<Response>;

			if constexpr (std::is_same_v<response_type, mqtt::message>)
			{
				if (!rep.empty())
				{
					std::visit([&ec, &caller_ptr, &caller, &om, &msg](auto& pr) mutable
					{
						caller->_after_user_callback_impl(ec, caller_ptr, caller, om, msg, pr);
					}, rep.variant());
				}
			}
			else
			{
				caller->_after_user_callback_impl(ec, caller_ptr, caller, om, msg, rep);
			}
		}

		template<class Message, class Response>
		inline void _after_user_callback(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om, Message& msg, Response& rep)
		{
			using message_type  [[maybe_unused]] = typename detail::remove_cvref_t<Message>;
			using response_type [[maybe_unused]] = typename detail::remove_cvref_t<Response>;

			if constexpr (std::is_same_v<message_type, mqtt::message>)
			{
				this->_after_user_callback_with_message(ec, caller_ptr, caller, om, msg, rep);
			}
			else
			{
				this->_after_user_callback_with_packet(ec, caller_ptr, caller, om, msg, rep);
			}
		}
	};
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_MQTT_HANDLER_HPP__
