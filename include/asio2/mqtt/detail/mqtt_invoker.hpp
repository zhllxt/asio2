/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_MQTT_INVOKER_HPP__
#define __ASIO2_MQTT_INVOKER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/iopool.hpp>
#include <asio2/base/define.hpp>
#include <asio2/base/log.hpp>

#include <asio2/base/detail/function_traits.hpp>
#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/shared_mutex.hpp>

#include <asio2/mqtt/detail/mqtt_topic_util.hpp>
#include <asio2/mqtt/detail/mqtt_subscription_map.hpp>
#include <asio2/mqtt/detail/mqtt_shared_target.hpp>
#include <asio2/mqtt/detail/mqtt_retained_message.hpp>
#include <asio2/mqtt/detail/mqtt_message_router.hpp>

#include <asio2/mqtt/message.hpp>

#if !defined(ASIO2_HEADER_ONLY) && __has_include(<boost/core/type_name.hpp>)
#include <boost/core/type_name.hpp>
#else
#include <asio2/bho/core/type_name.hpp>
#endif

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;

	template<class caller_t, class args_t>
	class mqtt_invoker_t
	{
		friend caller_t;

		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;

	protected:
		struct dummy {};

	public:
		using self = mqtt_invoker_t<caller_t, args_t>;
		using handler_type = std::function<
			void(error_code&, std::shared_ptr<caller_t>&, caller_t*, std::string_view&)>;

		/**
		 * @brief constructor
		 */
		mqtt_invoker_t() noexcept : mqtt_handlers_() {}

		/**
		 * @brief destructor
		 */
		~mqtt_invoker_t() = default;

		/**
		 * @brief bind connect listener
		 * @param fun - a user defined callback function.
		 * @li Function signature : 
		 *   server : void(std::shared_ptr<mqtt_session>& session_ptr,
		 *                 mqtt::v4::connect& msg, mqtt::v4::connack& rep) or v3 or v5
		 *       or : void(std::shared_ptr<mqtt_session>& session_ptr,
		 *                 mqtt::message& msg, mqtt::message& rep)
		 *   client : Don't need
		 */
		template<class F, class ...C>
		inline self& on_connect(F&& fun, C&&... obj)
		{
			this->_bind(mqtt::control_packet_type::connect, std::forward<F>(fun), std::forward<C>(obj)...);

			return (*this);
		}

		/**
		 * @brief bind connack listener
		 * @param fun - a user defined callback function.
		 * @li Function signature : server : Don't need
		 *                      client : void(mqtt::v4::connack& msg) or v3 or v5
		 *                          or : void(mqtt::message& msg)
		 */
		template<class F, class ...C>
		inline self& on_connack(F&& fun, C&&... obj)
		{
			this->_bind(mqtt::control_packet_type::connack, std::forward<F>(fun), std::forward<C>(obj)...);
			return (*this);
		}

		/**
		 * @brief bind publish listener
		 * @param fun - a user defined callback function.
		 * @li Function signature : 
		 *   server : void(std::shared_ptr<mqtt_session>& session_ptr,
		 *                 mqtt::v4::publish& msg, mqtt::v4::puback rep) or v3 or v5
		 *       or : void(std::shared_ptr<mqtt_session>& session_ptr,
		 *                 mqtt::message& msg, mqtt::message rep)
		 *   client : void(mqtt::v4::publish& msg, mqtt::v4::puback rep) or v3 or v5
		 *       or : void(mqtt::message& msg, mqtt::message rep)
		 */
		template<class F, class ...C>
		inline self& on_publish(F&& fun, C&&... obj)
		{
			this->_bind(mqtt::control_packet_type::publish, std::forward<F>(fun), std::forward<C>(obj)...);
			return (*this);
		}

		/**
		 * @brief bind puback listener
		 * @param fun - a user defined callback function.
		 * @li Function signature : 
		 *   server : void(std::shared_ptr<mqtt_session>& session_ptr, mqtt::v4::puback& msg) or v3 or v5
		 *       or : void(std::shared_ptr<mqtt_session>& session_ptr, mqtt::message& msg)
		 *   client : void(mqtt::v4::puback& puback) or v3 or v5
		 *       or : void(mqtt::message& msg)
		 */
		template<class F, class ...C>
		inline self& on_puback(F&& fun, C&&... obj)
		{
			this->_bind(mqtt::control_packet_type::puback, std::forward<F>(fun), std::forward<C>(obj)...);
			return (*this);
		}

		/**
		 * @brief bind pubrec listener
		 * @param fun - a user defined callback function.
		 * @li Function signature : 
		 *   server : void(std::shared_ptr<mqtt_session>& session_ptr,
		 *                 mqtt::v4::pubrec& msg, mqtt::v4::pubrel& rep) or v3 or v5
		 *       or : void(std::shared_ptr<mqtt_session>& session_ptr,
		 *                 mqtt::message& msg, mqtt::message& rep)
		 *   client : void(mqtt::v4::pubrec& msg, mqtt::v4::pubrel& rep) or v3 or v5
		 *       or : void(mqtt::message& msg, mqtt::message& rep)
		 */
		template<class F, class ...C>
		inline self& on_pubrec(F&& fun, C&&... obj)
		{
			this->_bind(mqtt::control_packet_type::pubrec, std::forward<F>(fun), std::forward<C>(obj)...);
			return (*this);
		}

		/**
		 * @brief bind pubrel listener
		 * @param fun - a user defined callback function.
		 * @li Function signature : 
		 *   server : void(std::shared_ptr<mqtt_session>& session_ptr,
		 *                 mqtt::v4::pubrel& msg, mqtt::v4::pubcomp& rep) or v3 or v5
		 *       or : void(std::shared_ptr<mqtt_session>& session_ptr,
		 *                 mqtt::message& msg, mqtt::message& rep)
		 *   client : void(mqtt::v4::pubrel& msg, mqtt::v4::pubcomp& rep) or v3 or v5
		 *       or : void(mqtt::message& msg, mqtt::message& rep)
		 */
		template<class F, class ...C>
		inline self& on_pubrel(F&& fun, C&&... obj)
		{
			this->_bind(mqtt::control_packet_type::pubrel, std::forward<F>(fun), std::forward<C>(obj)...);
			return (*this);
		}

		/**
		 * @brief bind pubcomp listener
		 * @param fun - a user defined callback function.
		 * @li Function signature : server : void(std::shared_ptr<mqtt_session>& session_ptr,
		 *                                    mqtt::v4::pubcomp& msg) or v3 or v5
		 *                          or : void(std::shared_ptr<mqtt_session>& session_ptr,
		 *                                    mqtt::message& msg)
		 *                      client : void(mqtt::v4::pubcomp& msg) or v3 or v5
		 *                          or : void(mqtt::message& msg)
		 */
		template<class F, class ...C>
		inline self& on_pubcomp(F&& fun, C&&... obj)
		{
			this->_bind(mqtt::control_packet_type::pubcomp, std::forward<F>(fun), std::forward<C>(obj)...);
			return (*this);
		}

		/**
		 * @brief bind subscribe listener
		 * @param fun - a user defined callback function.
		 * @li Function signature : 
		 *   server : void(std::shared_ptr<mqtt_session>& session_ptr,
		 *                 mqtt::v4::subscribe& msg, mqtt::v4::suback& rep) or v3 or v5
		 *       or : void(std::shared_ptr<mqtt_session>& session_ptr,
		 *                 mqtt::message& msg, mqtt::message& rep)
		 *   client : Don't need
		 */
		template<class F, class ...C>
		inline self& on_subscribe(F&& fun, C&&... obj)
		{
			this->_bind(mqtt::control_packet_type::subscribe, std::forward<F>(fun), std::forward<C>(obj)...);
			return (*this);
		}

		/**
		 * @brief bind suback listener
		 * @param fun - a user defined callback function.
		 * @li Function signature : server : Don't need
		 *                      client : void(mqtt::v4::suback& msg) or v3 or v5
		 *                          or : void(mqtt::message& msg)
		 */
		template<class F, class ...C>
		inline self& on_suback(F&& fun, C&&... obj)
		{
			this->_bind(mqtt::control_packet_type::suback, std::forward<F>(fun), std::forward<C>(obj)...);
			return (*this);
		}

		/**
		 * @brief bind unsubscribe listener
		 * @param fun - a user defined callback function.
		 * @li Function signature : 
		 *   server : void(std::shared_ptr<mqtt_session>& session_ptr,
		 *                 mqtt::v4::unsubscribe& msg, mqtt::v4::unsuback& rep) or v3 or v5
		 *   server : void(std::shared_ptr<mqtt_session>& session_ptr,
		 *                 mqtt::message& msg, mqtt::message& rep)
		 *   client : Don't need
		 */
		template<class F, class ...C>
		inline self& on_unsubscribe(F&& fun, C&&... obj)
		{
			this->_bind(mqtt::control_packet_type::unsubscribe, std::forward<F>(fun), std::forward<C>(obj)...);
			return (*this);
		}

		/**
		 * @brief bind unsuback listener
		 * @param fun - a user defined callback function.
		 * @li Function signature : server : Don't need
		 *                      client : void(mqtt::v4::unsuback& msg) or v3 or v5
		 *                          or : void(mqtt::message& msg)
		 */
		template<class F, class ...C>
		inline self& on_unsuback(F&& fun, C&&... obj)
		{
			this->_bind(mqtt::control_packet_type::unsuback, std::forward<F>(fun), std::forward<C>(obj)...);
			return (*this);
		}

		/**
		 * @brief bind pingreq listener
		 * @param fun - a user defined callback function.
		 * @li Function signature : 
		 *   server : void(std::shared_ptr<mqtt_session>& session_ptr,
		 *                 mqtt::v4::pingreq& msg, mqtt::v4::pingresp& rep) or v3 or v5
		 *       or : void(std::shared_ptr<mqtt_session>& session_ptr,
		 *                 mqtt::message& msg, mqtt::message& rep)
		 *   client : Don't need
		 */
		template<class F, class ...C>
		inline self& on_pingreq(F&& fun, C&&... obj)
		{
			this->_bind(mqtt::control_packet_type::pingreq, std::forward<F>(fun), std::forward<C>(obj)...);
			return (*this);
		}

		/**
		 * @brief bind pingresp listener
		 * @param fun - a user defined callback function.
		 * @li Function signature : server : Don't need
		 *                      client : void(mqtt::v4::pingresp& msg) or v3 or v5
		 *                          or : void(mqtt::message& msg)
		 */
		template<class F, class ...C>
		inline self& on_pingresp(F&& fun, C&&... obj)
		{
			this->_bind(mqtt::control_packet_type::pingresp, std::forward<F>(fun), std::forward<C>(obj)...);
			return (*this);
		}

		/**
		 * @brief bind disconnect listener
		 * @param fun - a user defined callback function.
		 * @li Function signature :
		 *    server : void(std::shared_ptr<mqtt_session>& session_ptr, mqtt::v4::disconnect& msg) or v3 or v5
		 *        or : void(std::shared_ptr<mqtt_session>& session_ptr, mqtt::message& msg)
		 *    client : void(mqtt::v4::disconnect& msg) or v3 or v5
		 *        or : void(mqtt::message& msg)
		 */
		template<class F, class ...C>
		inline self& on_disconnect(F&& fun, C&&... obj)
		{
			this->_bind(mqtt::control_packet_type::disconnect, std::forward<F>(fun), std::forward<C>(obj)...);
			return (*this);
		}

		/**
		 * @brief bind auth listener
		 * @param fun - a user defined callback function.
		 * @li Function signature : 
		 *   server : void(std::shared_ptr<mqtt_session>& session_ptr,
		 *                 mqtt::v5::auth& msg, mqtt::v5::connack& rep)
		 *       or : void(std::shared_ptr<mqtt_session>& session_ptr,
		 *                 mqtt::message& msg, mqtt::message& rep)
		 *   client : void(mqtt::v5::auth& msg, mqtt::v5::auth& rep)
		 *       or : void(mqtt::message& msg, mqtt::message& rep)
		 */
		template<class F, class ...C>
		inline self& on_auth(F&& fun, C&&... obj)
		{
			this->_bind(mqtt::control_packet_type::auth, std::forward<F>(fun), std::forward<C>(obj)...);
			return (*this);
		}

	protected:
		template<class F>
		inline void _bind(mqtt::control_packet_type type, F f)
		{
			this->_do_bind(type, std::move(f), ((dummy*)nullptr));
		}

		template<class F, class C>
		inline void _bind(mqtt::control_packet_type type, F f, C& c)
		{
			this->_do_bind(type, std::move(f), std::addressof(c));
		}

		template<class F, class C>
		inline void _bind(mqtt::control_packet_type type, F f, C* c)
		{
			this->_do_bind(type, std::move(f), c);
		}

		template<class F, class C>
		inline void _do_bind(mqtt::control_packet_type type, F f, C* c)
		{
			asio2::unique_locker g(this->mqtt_invoker_mutex_);

			this->mqtt_handlers_[detail::to_underlying(type)] = std::make_shared<handler_type>(std::bind(
				&self::template _proxy<F, C>,
				this, std::move(f), c,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
		}

		template<class F, class C>
		inline void _proxy(F& f, C* c, error_code& ec,
			std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, std::string_view& data)
		{
			using fun_traits_type = function_traits<F>;

			_argc_proxy<fun_traits_type::argc>(f, c, ec, caller_ptr, caller, data);
		}

		template<class F, class C, class M>
		inline void _do_argc_1_proxy(F& f, C* c,
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, M& msg)
		{
			using fun_traits_type = function_traits<F>;
			using arg0_type = typename std::remove_cv_t<std::remove_reference_t<
				typename fun_traits_type::template args<0>::type>>;

			using message_type = arg0_type;

			if constexpr (std::is_same_v<message_type, mqtt::message>)
			{
				ec.clear();

				this->_do_client_no_response(f, c, ec, caller_ptr, caller, msg, msg);
			}
			else
			{
				message_type* pmsg = std::get_if<message_type>(std::addressof(msg.variant()));

				if (pmsg)
				{
					ec.clear();

					this->_do_client_no_response(f, c, ec, caller_ptr, caller, msg, *pmsg);
				}
				else
				{
					this->_do_no_match_callback(f, ec, caller_ptr, caller, pmsg);
				}
			}
		}

		// Argc == 1 : must be client, the callback signature : void (mqtt::xxx_message&)
		template<std::size_t Argc, class F, class C>
		typename std::enable_if_t<Argc == 1>
		inline _argc_proxy(F& f, C* c,
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, std::string_view& data)
		{
			mqtt::data_to_message(caller->version(), data, [this, &f, &c, &ec, &caller_ptr, caller]
			(auto msg) mutable
			{
				if (msg.empty() || asio2::get_last_error())
				{
					this->_do_malformed_packet(f, ec ? ec : asio2::get_last_error(), caller_ptr, caller);

					return;
				}

				this->_do_argc_1_proxy(f, c, ec, caller_ptr, caller, msg);
			});
		}

		template<class F, class C, class M>
		inline void _do_argc_2_proxy_server(F& f, C* c,
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, M& msg)
		{
			using fun_traits_type = function_traits<F>;

			using message_type = typename std::remove_cv_t<std::remove_reference_t<
				typename fun_traits_type::template args<1>::type>>;

			if constexpr (std::is_same_v<message_type, mqtt::message>)
			{
				ec.clear();

				this->_do_server_no_response(f, c, ec, caller_ptr, caller, msg, msg);
			}
			else
			{
				message_type* pmsg = std::get_if<message_type>(std::addressof(msg.variant()));

				if (pmsg)
				{
					ec.clear();

					this->_do_server_no_response(f, c, ec, caller_ptr, caller, msg, *pmsg);
				}
				else
				{
					this->_do_no_match_callback(f, ec, caller_ptr, caller, pmsg);
				}
			}
		}

		template<class F, class C>
		inline void _argc_2_proxy_server(F& f, C* c,
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, std::string_view& data)
		{
			mqtt::data_to_message(caller->version(), data, [this, &f, &c, &ec, &caller_ptr, caller]
			(auto msg) mutable
			{
				if (msg.empty() || asio2::get_last_error())
				{
					this->_do_malformed_packet(f, ec ? ec : asio2::get_last_error(), caller_ptr, caller);

					return;
				}

				this->_do_argc_2_proxy_server(f, c, ec, caller_ptr, caller, msg);
			});
		}

		template<class F, class C, class M>
		inline void _do_argc_2_proxy_client(F& f, C* c,
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, M& msg)
		{
			using fun_traits_type = function_traits<F>;
			using arg0_type = typename std::remove_cv_t<std::remove_reference_t<
				typename fun_traits_type::template args<0>::type>>;

			using message_type = arg0_type;
			using response_type = typename std::remove_cv_t<std::remove_reference_t<
				typename fun_traits_type::template args<1>::type>>;

			if constexpr (std::is_same_v<message_type, mqtt::message>)
			{
				ec.clear();

				this->_do_client_with_response(
					f, c, ec, caller_ptr, caller, msg, msg, response_type{});
			}
			else
			{
				message_type* pmsg = std::get_if<message_type>(std::addressof(msg.variant()));

				if (pmsg)
				{
					ec.clear();

					this->_do_client_with_response(
						f, c, ec, caller_ptr, caller, msg, *pmsg, response_type{});
				}
				else
				{
					this->_do_no_match_callback(f, ec, caller_ptr, caller, pmsg);
				}
			}
		}

		template<class F, class C>
		inline void _argc_2_proxy_client(F& f, C* c,
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, std::string_view& data)
		{
			mqtt::data_to_message(caller->version(), data, [this, &f, &c, &ec, &caller_ptr, caller]
			(auto msg) mutable
			{
				if (msg.empty() || asio2::get_last_error())
				{
					this->_do_malformed_packet(f, ec ? ec : asio2::get_last_error(), caller_ptr, caller);

					return;
				}

				this->_do_argc_2_proxy_client(f, c, ec, caller_ptr, caller, msg);
			});
		}

		// Argc == 2 : client or server
		// if client, the callback signature : void (mqtt::xxx_message& message, mqtt::xxx_message& response)
		// if server, the callback signature : void (std::shared_ptr<xxx_session>& session, mqtt::xxx_message& message)
		template<std::size_t Argc, class F, class C>
		typename std::enable_if_t<Argc == 2>
		inline _argc_proxy(F& f, C* c,
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, std::string_view& data)
		{
			using fun_traits_type = function_traits<F>;
			using arg0_type = typename std::remove_cv_t<std::remove_reference_t<
				typename fun_traits_type::template args<0>::type>>;

			// must be server
			if constexpr (std::is_same_v<std::shared_ptr<caller_t>, arg0_type>)
			{
				this->_argc_2_proxy_server(f, c, ec, caller_ptr, caller, data);
			}
			// must be client
			else
			{
				this->_argc_2_proxy_client(f, c, ec, caller_ptr, caller, data);
			}
		}

		template<class F, class C, class M>
		inline void _do_argc_3_proxy(F& f, C* c,
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, M& msg)
		{
			using fun_traits_type = function_traits<F>;
			using arg0_type = typename std::remove_cv_t<std::remove_reference_t<
				typename fun_traits_type::template args<0>::type>>;

			static_assert(std::is_same_v<std::shared_ptr<caller_t>, arg0_type>);

			using message_type = typename std::remove_cv_t<std::remove_reference_t<
				typename fun_traits_type::template args<1>::type>>;
			using response_type = typename std::remove_cv_t<std::remove_reference_t<
				typename fun_traits_type::template args<2>::type>>;

			if constexpr (std::is_same_v<message_type, mqtt::message>)
			{
				ec.clear();

				this->_do_server_with_response(
					f, c, ec, caller_ptr, caller, msg, msg, response_type{});
			}
			else
			{
				message_type* pmsg = std::get_if<message_type>(std::addressof(msg.variant()));

				if (pmsg)
				{
					ec.clear();

					this->_do_server_with_response(
						f, c, ec, caller_ptr, caller, msg, *pmsg, response_type{});
				}
				else
				{
					this->_do_no_match_callback(f, ec, caller_ptr, caller, pmsg);
				}
			}
		}

		// Argc == 3 : must be server, the callback signature : 
		// void (std::shared_ptr<xxx_session>&, mqtt::xxx_message& message, mqtt::xxx_message& response)
		template<std::size_t Argc, class F, class C>
		typename std::enable_if_t<Argc == 3>
		inline _argc_proxy(F& f, C* c,
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, std::string_view& data)
		{
			mqtt::data_to_message(caller->version(), data, [this, &f, &c, &ec, &caller_ptr, caller]
			(auto msg) mutable
			{
				if (msg.empty() || asio2::get_last_error())
				{
					this->_do_malformed_packet(f, ec ? ec : asio2::get_last_error(), caller_ptr, caller);

					return;
				}

				this->_do_argc_3_proxy(f, c, ec, caller_ptr, caller, msg);
			});
		}

		template<class F>
		inline void _do_malformed_packet(F& f,
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller)
		{
			detail::ignore_unused(f);

			if (!ec)
				ec = mqtt::make_error_code(mqtt::error::malformed_packet);

			this->_handle_mqtt_error(ec, caller_ptr, caller);
		}

		template<class F, class M>
		inline void _do_no_match_callback(F& f,
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, M* pmsg)
		{
			detail::ignore_unused(f, pmsg);

		#if !defined(ASIO2_HEADER_ONLY) && __has_include(<boost/core/type_name.hpp>)
			ASIO2_LOG_INFOR("The user callback function signature do not match : {}({} ...)"
				, boost::core::type_name<detail::remove_cvref_t<F>>()
				, boost::core::type_name<detail::remove_cvref_t<M>>()
			);
		#else
			ASIO2_LOG_INFOR("The user callback function signature do not match : {}({} ...)"
				, bho::core::type_name<detail::remove_cvref_t<F>>()
				, bho::core::type_name<detail::remove_cvref_t<M>>()
			);
		#endif

			//ASIO2_ASSERT(false &&
			//	"The parameters of the user callback function do not match."
			//	" Check that the parameters of your callback function are of the correct type");

			if (!ec)
				ec = mqtt::make_error_code(mqtt::error::malformed_packet);

			this->_handle_mqtt_error(ec, caller_ptr, caller);
		}

		template<typename F, typename C, class Message>
		inline void _do_client_no_response(F& f, C* c,
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			Message& msg)
		{
			caller->_before_user_callback(ec, caller_ptr, caller, om, msg);

			if (ec)
				return this->_handle_mqtt_error(ec, caller_ptr, caller);

			this->_invoke_user_callback(f, c, msg);

			caller->_match_router(om);

			caller->_after_user_callback(ec, caller_ptr, caller, om, msg);

			this->_handle_mqtt_error(ec, caller_ptr, caller);
		}

		template<typename F, typename C, class Message>
		inline void _do_server_no_response(F& f, C* c,
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			Message& msg)
		{
			caller->_before_user_callback(ec, caller_ptr, caller, om, msg);

			if (ec)
				return this->_handle_mqtt_error(ec, caller_ptr, caller);

			this->_invoke_user_callback(f, c, caller_ptr, msg);

			caller->_after_user_callback(ec, caller_ptr, caller, om, msg);

			this->_handle_mqtt_error(ec, caller_ptr, caller);
		}

		template<typename F, typename C, class Message, class Response>
		inline void _do_client_with_response(F& f, C* c,
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			Message& msg, Response rep)
		{
			this->_init_response(ec, caller_ptr, caller, msg, rep);

			caller->_before_user_callback(ec, caller_ptr, caller, om, msg, rep);

			if (ec)
				return this->_handle_mqtt_error(ec, caller_ptr, caller);

			this->_invoke_user_callback(f, c, msg, rep);

			caller->_after_user_callback(ec, caller_ptr, caller, om, msg, rep);

			this->_send_mqtt_response(ec, caller_ptr, caller, msg, std::move(rep));

			this->_handle_mqtt_error(ec, caller_ptr, caller);
		}

		template<typename F, typename C, class Message, class Response>
		inline void _do_server_with_response(F& f, C* c,
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			Message& msg, Response rep)
		{
			this->_init_response(ec, caller_ptr, caller, msg, rep);

			caller->_before_user_callback(ec, caller_ptr, caller, om, msg, rep);

			if (ec)
				return this->_handle_mqtt_error(ec, caller_ptr, caller);

			this->_invoke_user_callback(f, c, caller_ptr, msg, rep);

			caller->_after_user_callback(ec, caller_ptr, caller, om, msg, rep);

			this->_send_mqtt_response(ec, caller_ptr, caller, msg, std::move(rep));

			this->_handle_mqtt_error(ec, caller_ptr, caller);
		}

		template<typename F, typename C, typename... Args>
		inline void _invoke_user_callback(F& f, C* c, Args&&... args)
		{
			detail::ignore_unused(c);

			if constexpr (std::is_same_v<detail::remove_cvref_t<C>, dummy>)
				f(std::forward<Args>(args)...);
			else
				(c->*f)(std::forward<Args>(args)...);
		}

		template<class Message, class Response>
		typename std::enable_if_t<std::is_same_v<typename detail::remove_cvref_t<Message>, mqtt::message>>
		inline _init_response(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller,
			Message& msg, Response& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, rep);

			using message_type  [[maybe_unused]] = typename detail::remove_cvref_t<Message>;
			using response_type [[maybe_unused]] = typename detail::remove_cvref_t<Response>;

			if constexpr (std::is_same_v<response_type, mqtt::message>)
			{
				if constexpr (std::is_same_v<message_type, mqtt::message>)
				{
					std::visit([this, &ec, &caller_ptr, &caller, &rep](auto& pm) mutable
					{
						this->_init_response(ec, caller_ptr, caller, pm, rep);
					}, msg.variant());
				}
				else
				{
					std::ignore = true;
				}
			}
			else
			{
				std::ignore = true;
			}
		}

		template<class Message, class Response>
		inline void _init_connect_response(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, Message& msg, Response& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, rep);

			mqtt::version ver = caller->version();

			if /**/ (ver == mqtt::version::v3)
			{
				rep = mqtt::v3::connack{};
			}
			else if (ver == mqtt::version::v4)
			{
				rep = mqtt::v4::connack{};
			}
			else if (ver == mqtt::version::v5)
			{
				rep = mqtt::v5::connack{};
			}
		}

		template<class Message, class Response>
		inline void _init_publish_response(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, Message& msg, Response& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, rep);

			mqtt::version ver = caller->version();

			if /**/ (ver == mqtt::version::v3)
			{
				switch (msg.qos())
				{
				// the qos 0 publish messgae don't need response, here just a placeholder,
				// if has't set the rep to a msg, the _before_user_callback_impl can't be 
				// called correctly.
				case mqtt::qos_type::at_most_once : rep = mqtt::v3::puback{}; break;
				case mqtt::qos_type::at_least_once: rep = mqtt::v3::puback{}; break;
				case mqtt::qos_type::exactly_once : rep = mqtt::v3::pubrec{}; break;
				default:break;
				}
			}
			else if (ver == mqtt::version::v4)
			{
				switch (msg.qos())
				{
				// the qos 0 publish messgae don't need response, here just a placeholder,
				// if has't set the rep to a msg, the _before_user_callback_impl can't be 
				// called correctly.
				case mqtt::qos_type::at_most_once : rep = mqtt::v4::puback{}; break;
				case mqtt::qos_type::at_least_once: rep = mqtt::v4::puback{}; break;
				case mqtt::qos_type::exactly_once : rep = mqtt::v4::pubrec{}; break;
				default:break;
				}
			}
			else if (ver == mqtt::version::v5)
			{
				switch (msg.qos())
				{
				// the qos 0 publish messgae don't need response, here just a placeholder,
				// if has't set the rep to a msg, the _before_user_callback_impl can't be 
				// called correctly.
				case mqtt::qos_type::at_most_once : rep = mqtt::v5::puback{}; break;
				case mqtt::qos_type::at_least_once: rep = mqtt::v5::puback{}; break;
				case mqtt::qos_type::exactly_once : rep = mqtt::v5::pubrec{}; break;
				default:break;
				}
			}
		}

		template<class Message, class Response>
		inline void _init_pubrec_response(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, Message& msg, Response& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, rep);

			mqtt::version ver = caller->version();

			if /**/ (ver == mqtt::version::v3)
			{
				rep = mqtt::v3::pubrel{};
			}
			else if (ver == mqtt::version::v4)
			{
				rep = mqtt::v4::pubrel{};
			}
			else if (ver == mqtt::version::v5)
			{
				rep = mqtt::v5::pubrel{};
			}
		}

		template<class Message, class Response>
		inline void _init_pubrel_response(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, Message& msg, Response& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, rep);

			mqtt::version ver = caller->version();

			if /**/ (ver == mqtt::version::v3)
			{
				rep = mqtt::v3::pubcomp{};
			}
			else if (ver == mqtt::version::v4)
			{
				rep = mqtt::v4::pubcomp{};
			}
			else if (ver == mqtt::version::v5)
			{
				rep = mqtt::v5::pubcomp{};
			}
		}

		template<class Message, class Response>
		inline void _init_subscribe_response(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, Message& msg, Response& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, rep);

			mqtt::version ver = caller->version();

			if /**/ (ver == mqtt::version::v3)
			{
				rep = mqtt::v3::suback{};
			}
			else if (ver == mqtt::version::v4)
			{
				rep = mqtt::v4::suback{};
			}
			else if (ver == mqtt::version::v5)
			{
				rep = mqtt::v5::suback{};
			}
		}

		template<class Message, class Response>
		inline void _init_unsubscribe_response(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, Message& msg, Response& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, rep);

			mqtt::version ver = caller->version();

			if /**/ (ver == mqtt::version::v3)
			{
				rep = mqtt::v3::unsuback{};
			}
			else if (ver == mqtt::version::v4)
			{
				rep = mqtt::v4::unsuback{};
			}
			else if (ver == mqtt::version::v5)
			{
				rep = mqtt::v5::unsuback{};
			}
		}

		template<class Message, class Response>
		inline void _init_pingreq_response(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, Message& msg, Response& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, rep);

			mqtt::version ver = caller->version();

			if /**/ (ver == mqtt::version::v3)
			{
				rep = mqtt::v3::pingresp{};
			}
			else if (ver == mqtt::version::v4)
			{
				rep = mqtt::v4::pingresp{};
			}
			else if (ver == mqtt::version::v5)
			{
				rep = mqtt::v5::pingresp{};
			}
		}

		template<class Message, class Response>
		inline void _init_auth_response(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, Message& msg, Response& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, rep);

			mqtt::version ver = caller->version();

			if /**/ (ver == mqtt::version::v3)
			{
				rep = mqtt::v3::connack{};
			}
			else if (ver == mqtt::version::v4)
			{
				rep = mqtt::v4::connack{};
			}
			else if (ver == mqtt::version::v5)
			{
				rep = mqtt::v5::auth{};
			}
		}

		template<class Message, class Response>
		typename std::enable_if_t<mqtt::is_rawmsg<typename detail::remove_cvref_t<Message>>()>
		inline _init_response(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller,
			Message& msg, Response& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, rep);

			using message_type  [[maybe_unused]] = typename detail::remove_cvref_t<Message>;
			using response_type [[maybe_unused]] = typename detail::remove_cvref_t<Response>;

			if constexpr (std::is_same_v<response_type, mqtt::message>)
			{
				if /**/ constexpr (
					std::is_same_v<message_type, mqtt::v3::connect> ||
					std::is_same_v<message_type, mqtt::v4::connect> ||
					std::is_same_v<message_type, mqtt::v5::connect>)
				{
					this->_init_connect_response(ec, caller_ptr, caller, msg, rep);
				}
				else if constexpr (
					std::is_same_v<message_type, mqtt::v3::publish> ||
					std::is_same_v<message_type, mqtt::v4::publish> ||
					std::is_same_v<message_type, mqtt::v5::publish>)
				{
					this->_init_publish_response(ec, caller_ptr, caller, msg, rep);
				}
				else if constexpr (
					std::is_same_v<message_type, mqtt::v3::pubrec> ||
					std::is_same_v<message_type, mqtt::v4::pubrec> ||
					std::is_same_v<message_type, mqtt::v5::pubrec>)
				{
					this->_init_pubrec_response(ec, caller_ptr, caller, msg, rep);
				}
				else if constexpr (
					std::is_same_v<message_type, mqtt::v3::pubrel> ||
					std::is_same_v<message_type, mqtt::v4::pubrel> ||
					std::is_same_v<message_type, mqtt::v5::pubrel>)
				{
					this->_init_pubrel_response(ec, caller_ptr, caller, msg, rep);
				}
				else if constexpr (
					std::is_same_v<message_type, mqtt::v3::subscribe> ||
					std::is_same_v<message_type, mqtt::v4::subscribe> ||
					std::is_same_v<message_type, mqtt::v5::subscribe>)
				{
					this->_init_subscribe_response(ec, caller_ptr, caller, msg, rep);
				}
				else if constexpr (
					std::is_same_v<message_type, mqtt::v3::unsubscribe> ||
					std::is_same_v<message_type, mqtt::v4::unsubscribe> ||
					std::is_same_v<message_type, mqtt::v5::unsubscribe>)
				{
					this->_init_unsubscribe_response(ec, caller_ptr, caller, msg, rep);
				}
				else if constexpr (
					std::is_same_v<message_type, mqtt::v3::pingreq> ||
					std::is_same_v<message_type, mqtt::v4::pingreq> ||
					std::is_same_v<message_type, mqtt::v5::pingreq>)
				{
					this->_init_pingreq_response(ec, caller_ptr, caller, msg, rep);
				}
				else if constexpr (
					std::is_same_v<message_type, mqtt::v5::auth>)
				{
					this->_init_auth_response(ec, caller_ptr, caller, msg, rep);
				}
				else
				{
					std::ignore = true;
				}
			}
			else
			{
				std::ignore = true;
			}
		}

		template<class Message, class Response>
		inline void _send_mqtt_message_response(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller,
			Message& msg, Response&& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, rep);

			using message_type  [[maybe_unused]] = typename detail::remove_cvref_t<Message>;
			using response_type [[maybe_unused]] = typename detail::remove_cvref_t<Response>;

			if (rep.empty())
				return;

			bool sendflag = true;

			std::visit([&sendflag](auto& rep) mutable { sendflag = rep.get_send_flag(); }, rep.variant());

			if (sendflag == false)
				return;

			// can't use async_send, beacuse the caller maybe not started yet
			caller->push_event([caller_ptr, caller, id = caller->life_id(), rep = std::forward<Response>(rep)]
			(event_queue_guard<caller_t> g) mutable
			{
				detail::ignore_unused(caller_ptr);

				if (id != caller->life_id())
				{
					set_last_error(asio::error::operation_aborted);
					return;
				}

				std::visit([caller, g = std::move(g)](auto& pr) mutable
				{
				#if !defined(ASIO2_HEADER_ONLY) && __has_include(<boost/core/type_name.hpp>)
					ASIO2_LOG_DEBUG("mqtt send {}", boost::core::type_name<decltype(pr)>());
				#else
					ASIO2_LOG_DEBUG("mqtt send {}", bho::core::type_name<decltype(pr)>());
				#endif

					caller->_do_send(pr, [g = std::move(g)](const error_code&, std::size_t) mutable {});
				}, rep.variant());
			});
		}

		template<class Message, class Response>
		inline void _send_mqtt_packet_response(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller,
			Message& msg, Response&& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, rep);

			using message_type  [[maybe_unused]] = typename detail::remove_cvref_t<Message>;
			using response_type [[maybe_unused]] = typename detail::remove_cvref_t<Response>;

			if (rep.get_send_flag() == false)
				return;

			// can't use async_send, beacuse the caller maybe not started yet
			caller->push_event([caller_ptr, caller, id = caller->life_id(), rep = std::forward<Response>(rep)]
			(event_queue_guard<caller_t> g) mutable
			{
				detail::ignore_unused(caller_ptr);

				if (id != caller->life_id())
				{
					set_last_error(asio::error::operation_aborted);
					return;
				}

			#if !defined(ASIO2_HEADER_ONLY) && __has_include(<boost/core/type_name.hpp>)
				ASIO2_LOG_DEBUG("mqtt send {}", boost::core::type_name<decltype(rep)>());
			#else
				ASIO2_LOG_DEBUG("mqtt send {}", bho::core::type_name<decltype(rep)>());
			#endif

				caller->_do_send(rep, [g = std::move(g)](const error_code&, std::size_t) mutable {});
			});
		}

		template<class Message, class Response>
		inline void _send_mqtt_response(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller,
			Message& msg, Response&& rep)
		{
			using message_type  [[maybe_unused]] = typename detail::remove_cvref_t<Message>;
			using response_type [[maybe_unused]] = typename detail::remove_cvref_t<Response>;

			if constexpr (std::is_same_v<response_type, mqtt::message>)
			{
				this->_send_mqtt_message_response(ec, caller_ptr, caller, msg, std::forward<Response>(rep));
			}
			else
			{
				this->_send_mqtt_packet_response(ec, caller_ptr, caller, msg, std::forward<Response>(rep));
			}
		}

		inline void _call_mqtt_handler(mqtt::control_packet_type type, error_code& ec,
			std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, std::string_view& data)
		{
			ASIO2_ASSERT(caller->io_->running_in_this_thread());

			std::shared_ptr<handler_type> p;

			{
				asio2::shared_locker g(this->mqtt_invoker_mutex_);

				if (detail::to_underlying(type) < this->mqtt_handlers_.size())
				{
					p = this->mqtt_handlers_[detail::to_underlying(type)];
				}
			}

			if (p && (*p))
			{
				(*p)(ec, caller_ptr, caller, data);
			}
			else
			{
				// Should't run to here.
				ASIO2_ASSERT(false);
				ec = mqtt::make_error_code(mqtt::error::malformed_packet);
				this->_handle_mqtt_error(ec, caller_ptr, caller);
				return;
			}
		}

		template<class CallerT = caller_t>
		inline void _handle_mqtt_error(error_code& ec, std::shared_ptr<CallerT>& caller_ptr, CallerT* caller)
		{
			if constexpr (CallerT::is_client())
			{
				return;
			}
			else
			{
				if (!ec)
					return;

				// post a async event to disconnect, don't call _do_disconnect directly,
				// otherwise the client's bind_disconnect callback maybe can't be called.
				asio::post(caller->io_->context(), make_allocator(caller->wallocator(),
				[ec, caller_ptr, caller]() mutable
				{
					if (caller->state_ == state_t::started)
					{
						caller->_do_disconnect(ec, std::move(caller_ptr));
					}
				}));
			}
		}

		inline std::shared_ptr<handler_type> _find_mqtt_handler(mqtt::control_packet_type type)
		{
			asio2::shared_locker g(this->mqtt_invoker_mutex_);

			if (detail::to_underlying(type) < this->mqtt_handlers_.size())
			{
				std::shared_ptr<handler_type> p = mqtt_handlers_[detail::to_underlying(type)];

				if (p && (*p))
					return p;
			}

			return nullptr;
		}

	protected:
		/// use rwlock to make thread safe
		mutable asio2::shared_mutexer               mqtt_invoker_mutex_;

		// magic_enum has bug: maybe return 0 under wsl ubuntu
		std::array<std::shared_ptr<handler_type>, detail::to_underlying(mqtt::control_packet_type::auth) + 1>
			                                        mqtt_handlers_ ASIO2_GUARDED_BY(mqtt_invoker_mutex_);
	};
}

#endif // !__ASIO2_MQTT_INVOKER_HPP__
