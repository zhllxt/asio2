/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_MQTT_INVOKER_HPP__
#define __ASIO2_MQTT_INVOKER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/iopool.hpp>
#include <asio2/base/define.hpp>
#include <asio2/base/log.hpp>

#include <asio2/external/magic_enum.hpp>

#include <asio2/base/detail/function_traits.hpp>
#include <asio2/base/detail/util.hpp>

#include <asio2/mqtt/detail/mqtt_topic_util.hpp>
#include <asio2/mqtt/detail/mqtt_subscription_map.hpp>
#include <asio2/mqtt/detail/mqtt_shared_target.hpp>
#include <asio2/mqtt/detail/mqtt_retained_message.hpp>
#include <asio2/mqtt/detail/mqtt_message_router.hpp>

#include <asio2/mqtt/message.hpp>

#include <asio2/bho/core/type_name.hpp>

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
		using handler_type = std::function<void(error_code&, std::shared_ptr<caller_t>&, caller_t*, std::string_view&)>;

		/**
		 * @constructor
		 */
		mqtt_invoker_t() = default;

		/**
		 * @destructor
		 */
		~mqtt_invoker_t() = default;

		/**
		 * @function : bind connect listener
		 * @param    : fun - a user defined callback function
		 * Function signature : 
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
		 * @function : bind connack listener
		 * @param    : fun - a user defined callback function
		 * Function signature : server : Don't need
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
		 * @function : bind publish listener
		 * @param    : fun - a user defined callback function
		 * Function signature : 
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
		 * @function : bind puback listener
		 * @param    : fun - a user defined callback function
		 * Function signature : 
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
		 * @function : bind pubrec listener
		 * @param    : fun - a user defined callback function
		 * Function signature : 
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
		 * @function : bind pubrel listener
		 * @param    : fun - a user defined callback function
		 * Function signature : 
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
		 * @function : bind pubcomp listener
		 * @param    : fun - a user defined callback function
		 * Function signature : server : void(std::shared_ptr<mqtt_session>& session_ptr,
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
		 * @function : bind subscribe listener
		 * @param    : fun - a user defined callback function
		 * Function signature : 
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
		 * @function : bind suback listener
		 * @param    : fun - a user defined callback function
		 * Function signature : server : Don't need
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
		 * @function : bind unsubscribe listener
		 * @param    : fun - a user defined callback function
		 * Function signature : 
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
		 * @function : bind unsuback listener
		 * @param    : fun - a user defined callback function
		 * Function signature : server : Don't need
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
		 * @function : bind pingreq listener
		 * @param    : fun - a user defined callback function
		 * Function signature : 
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
		 * @function : bind pingresp listener
		 * @param    : fun - a user defined callback function
		 * Function signature : server : Don't need
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
		 * @function : bind disconnect listener
		 * @param    : fun - a user defined callback function
		 * Function signature :
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
		 * @function : bind auth listener
		 * @param    : fun - a user defined callback function
		 * Function signature : 
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
			this->handlers_[detail::to_underlying(type)] = std::bind(
				&self::template _proxy<F, C>,
				this, std::move(f), c,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
		}

		template<class F, class C>
		inline void _proxy(F& f, C* c, error_code& ec,
			std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, std::string_view& data)
		{
			using fun_traits_type = function_traits<F>;

			_argc_proxy<fun_traits_type::argc>(f, c, ec, caller_ptr, caller, data);
		}

		// Argc == 1 : must be client, the callback signature : void (mqtt::xxx_message&)
		template<std::size_t Argc, class F, class C>
		typename std::enable_if_t<Argc == 1>
		inline _argc_proxy(F& f, C* c,
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, std::string_view& data)
		{
			using fun_traits_type = function_traits<F>;
			using arg0_type = typename std::remove_cv_t<std::remove_reference_t<
				typename fun_traits_type::template args<0>::type>>;

			using message_type = arg0_type;

			try
			{
				mqtt::data_to_message(caller->version(), data, [this, &f, &c, &ec, &caller_ptr, caller]
				(auto msg) mutable
				{
					if (msg.empty())
					{
						this->_do_malformed_packet(f, ec, caller_ptr, caller);

						return;
					}

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
				});
			}
			catch (system_error const& e)
			{
				if (!ec)
					ec = e.code();

				this->_handle_mqtt_error(ec, caller_ptr, caller);
			}
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
				using message_type = typename std::remove_cv_t<std::remove_reference_t<
					typename fun_traits_type::template args<1>::type>>;

				try
				{
					mqtt::data_to_message(caller->version(), data, [this, &f, &c, &ec, &caller_ptr, caller]
					(auto msg) mutable
					{
						if (msg.empty())
						{
							this->_do_malformed_packet(f, ec, caller_ptr, caller);

							return;
						}

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
					});
				}
				catch (system_error const& e)
				{
					if (!ec)
						ec = e.code();

					this->_handle_mqtt_error(ec, caller_ptr, caller);
				}
			}
			// must be client
			else
			{
				using message_type = arg0_type;
				using response_type = typename std::remove_cv_t<std::remove_reference_t<
					typename fun_traits_type::template args<1>::type>>;

				try
				{
					mqtt::data_to_message(caller->version(), data, [this, &f, &c, &ec, &caller_ptr, caller]
					(auto msg) mutable
					{
						if (msg.empty())
						{
							this->_do_malformed_packet(f, ec, caller_ptr, caller);

							return;
						}

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
					});
				}
				catch (system_error const& e)
				{
					if (!ec)
						ec = e.code();

					this->_handle_mqtt_error(ec, caller_ptr, caller);
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
			using fun_traits_type = function_traits<F>;
			using arg0_type = typename std::remove_cv_t<std::remove_reference_t<
				typename fun_traits_type::template args<0>::type>>;

			static_assert(std::is_same_v<std::shared_ptr<caller_t>, arg0_type>);

			using message_type = typename std::remove_cv_t<std::remove_reference_t<
				typename fun_traits_type::template args<1>::type>>;
			using response_type = typename std::remove_cv_t<std::remove_reference_t<
				typename fun_traits_type::template args<2>::type>>;

			try
			{
				mqtt::data_to_message(caller->version(), data, [this, &f, &c, &ec, &caller_ptr, caller]
				(auto msg) mutable
				{
					if (msg.empty())
					{
						this->_do_malformed_packet(f, ec, caller_ptr, caller);

						return;
					}

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
				});
			}
			catch (system_error const& e)
			{
				if (!ec)
					ec = e.code();

				this->_handle_mqtt_error(ec, caller_ptr, caller);
			}
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

			ASIO2_LOG(spdlog::level::info, "The user callback function signature do not match : {}({} ...)"
				, bho::core::type_name<detail::remove_cvref_t<F>>()
				, bho::core::type_name<detail::remove_cvref_t<M>>()
			);

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
				else if constexpr (
					std::is_same_v<message_type, mqtt::v3::publish> ||
					std::is_same_v<message_type, mqtt::v4::publish> ||
					std::is_same_v<message_type, mqtt::v5::publish>)
				{
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
				else if constexpr (
					std::is_same_v<message_type, mqtt::v3::pubrec> ||
					std::is_same_v<message_type, mqtt::v4::pubrec> ||
					std::is_same_v<message_type, mqtt::v5::pubrec>)
				{
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
				else if constexpr (
					std::is_same_v<message_type, mqtt::v3::pubrel> ||
					std::is_same_v<message_type, mqtt::v4::pubrel> ||
					std::is_same_v<message_type, mqtt::v5::pubrel>)
				{
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
				else if constexpr (
					std::is_same_v<message_type, mqtt::v3::subscribe> ||
					std::is_same_v<message_type, mqtt::v4::subscribe> ||
					std::is_same_v<message_type, mqtt::v5::subscribe>)
				{
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
				else if constexpr (
					std::is_same_v<message_type, mqtt::v3::unsubscribe> ||
					std::is_same_v<message_type, mqtt::v4::unsubscribe> ||
					std::is_same_v<message_type, mqtt::v5::unsubscribe>)
				{
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
				else if constexpr (
					std::is_same_v<message_type, mqtt::v3::pingreq> ||
					std::is_same_v<message_type, mqtt::v4::pingreq> ||
					std::is_same_v<message_type, mqtt::v5::pingreq>)
				{
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
				else if constexpr (
					std::is_same_v<message_type, mqtt::v5::auth>)
				{
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
		inline void _send_mqtt_response(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller,
			Message& msg, Response&& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, rep);

			using message_type  [[maybe_unused]] = typename detail::remove_cvref_t<Message>;
			using response_type [[maybe_unused]] = typename detail::remove_cvref_t<Response>;

			if constexpr (std::is_same_v<response_type, mqtt::message>)
			{
				if (rep.empty())
					return;

				bool sendflag = true;

				std::visit([&sendflag](auto& rep) mutable { sendflag = rep.get_send_flag(); }, rep.variant());

				if (sendflag == false)
					return;

				// can't use async_send, beacuse the caller maybe not started yet
				caller->push_event([caller_ptr, caller, rep = std::forward<Response>(rep)]
				(event_queue_guard<caller_t> g) mutable
				{
					detail::ignore_unused(caller_ptr);

					std::visit([caller, g = std::move(g)](auto& pr) mutable
					{
						ASIO2_LOG(spdlog::level::debug, "send {}", bho::core::type_name<decltype(pr)>());
						caller->_do_send(pr, [g = std::move(g)](const error_code&, std::size_t) mutable {});
					}, rep.variant());
				});
			}
			// response_message
			else
			{
				if (rep.get_send_flag() == false)
					return;

				// can't use async_send, beacuse the caller maybe not started yet
				caller->push_event([caller_ptr, caller, rep = std::forward<Response>(rep)]
				(event_queue_guard<caller_t> g) mutable
				{
					detail::ignore_unused(caller_ptr);
					ASIO2_LOG(spdlog::level::debug, "send {}", bho::core::type_name<decltype(rep)>());
					caller->_do_send(rep, [g = std::move(g)](const error_code&, std::size_t) mutable {});
				});
			}
		}

		inline void _call_mqtt_handler(mqtt::control_packet_type type, error_code& ec,
			std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, std::string_view& data)
		{
			ASIO2_ASSERT(caller->io().running_in_this_thread());

			handler_type* f = nullptr;

			if (detail::to_underlying(type) < this->handlers_.size())
				f = std::addressof(this->handlers_[detail::to_underlying(type)]);

			if (f && (*f))
			{
				(*f)(ec, caller_ptr, caller, data);
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
				asio::post(caller->io().context(), make_allocator(caller->wallocator(),
				[ec, caller_ptr, caller]() mutable
				{
					if (caller->state() == state_t::started)
					{
						caller->_do_disconnect(ec, std::move(caller_ptr));
					}
				}));
			}
		}

		inline const handler_type& _find_mqtt_handler(mqtt::control_packet_type type)
		{
			if (detail::to_underlying(type) < this->handlers_.size())
				return handlers_[detail::to_underlying(type)];

			return this->dummy_handler_;
		}

	protected:
		inline static handler_type dummy_handler_{};

		std::array<handler_type, magic_enum::enum_count<mqtt::control_packet_type>()> handlers_{};
	};
}

#endif // !__ASIO2_MQTT_INVOKER_HPP__
