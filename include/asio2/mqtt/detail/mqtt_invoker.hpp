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

#include <asio2/external/asio.hpp>
#include <asio2/base/iopool.hpp>
#include <asio2/base/error.hpp>
#include <asio2/base/define.hpp>

#include <asio2/base/detail/function_traits.hpp>
#include <asio2/base/detail/util.hpp>

#include <asio2/mqtt/mqtt_protocol_util.hpp>
#include <asio2/mqtt/detail/mqtt_topic_util.hpp>

#include <asio2/mqtt/detail/mqtt_subscription_map.hpp>
#include <asio2/mqtt/detail/mqtt_shared_target.hpp>
#include <asio2/mqtt/detail/mqtt_retained_message.hpp>

#include <asio2/external/magic_enum.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;

	template<class caller_t>
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
		using self = mqtt_invoker_t<caller_t>;
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
		 *                 mqtt::v4::connect& connect, mqtt::v4::connack& connack) or v3 or v5
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
		 *                      client : void(asio2::mqtt::v4::connack& connack) or v3 or v5
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
		 *                 asio2::mqtt::v4::publish& publish, 
		 *                 std::variant<asio2::mqtt::v4::puback, asio2::mqtt::v4::pubrec>& response) or v3 or v5
		 *   client : void(asio2::mqtt::v4::publish& publish, 
		 *                 std::variant<asio2::mqtt::v4::puback, asio2::mqtt::v4::pubrec>& response) or v3 or v5
		 */
		template<int Qos = -1, class F, class ...C>
		inline self& on_publish(F&& fun, C&&... obj)
		{
			if constexpr /**/ (Qos == -1)
			{
				this->_bind(mqtt::control_packet_type::publish, std::forward<F>(fun), std::forward<C>(obj)...);
			}
			else if constexpr (Qos == 0)
			{

			}
			else if constexpr (Qos == 1)
			{

			}
			else if constexpr (Qos == 2)
			{

			}
			else
			{
				ASIO2_ASSERT(false);
			}
			return (*this);
		}

		/**
		 * @function : bind puback listener
		 * @param    : fun - a user defined callback function
		 * Function signature : 
		 *   server : void(std::shared_ptr<mqtt_session>& session_ptr,
		 *                 asio2::mqtt::v4::puback& puback) or v3 or v5
		 *   client : void(asio2::mqtt::v4::puback& puback) or v3 or v5
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
		 *                 asio2::mqtt::v4::pubrec& pubrec, asio2::mqtt::v4::pubrel& pubrel) or v3 or v5
		 *   client : void(asio2::mqtt::v4::pubrec& pubrec, asio2::mqtt::v4::pubrel& pubrel) or v3 or v5
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
		 *                 asio2::mqtt::v4::pubrel& pubrel, asio2::mqtt::v4::pubcomp& pubcomp) or v3 or v5
		 *   client : void(asio2::mqtt::v4::pubrel& pubrel, asio2::mqtt::v4::pubcomp& pubcomp) or v3 or v5
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
		 *                                    asio2::mqtt::v4::pubcomp& pubcomp) or v3 or v5
		 *                      client : void(asio2::mqtt::v4::pubcomp& pubcomp) or v3 or v5
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
		 *                 mqtt::v4::subscribe& subscribe, mqtt::v4::suback& suback) or v3 or v5
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
		 *                      client : void(mqtt::v4::suback& suback) or v3 or v5
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
		 *                 mqtt::v4::unsubscribe& unsubscribe, mqtt::v4::unsuback& unsuback) or v3 or v5
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
		 *                      client : void(mqtt::v4::unsuback& unsuback) or v3 or v5
		 */
		template<class F, class ...C>
		inline self& on_unsuback(F&& fun, C&&... obj)
		{
			this->_bind(mqtt::control_packet_type::unsuback, std::forward<F>(fun), std::forward<C>(obj)...);
			return (*this);
		}

		/**
		 * @function : bind connack listener
		 * @param    : fun - a user defined callback function
		 * Function signature : 
		 *   server : void(std::shared_ptr<mqtt_session>& session_ptr,
		 *                 mqtt::v4::pingreq& pingreq, mqtt::v4::pingresp& pingresp) or v3 or v5
		 *   client : Don't need
		 */
		template<class F, class ...C>
		inline self& on_pingreq(F&& fun, C&&... obj)
		{
			this->_bind(mqtt::control_packet_type::pingreq, std::forward<F>(fun), std::forward<C>(obj)...);
			return (*this);
		}

		/**
		 * @function : bind connack listener
		 * @param    : fun - a user defined callback function
		 * Function signature : server : Don't need
		 *                      client : void(asio2::mqtt::v4::pingresp& pingresp) or v3 or v5
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
		 * Function signature : server : void(std::shared_ptr<mqtt_session>& session_ptr, mqtt::v4::disconnect& disconnect) or v3 or v5
		 *                      client : void(asio2::mqtt::v4::disconnect& disconnect) or v3 or v5
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
		 *                 std::variant<mqtt::v5::auth, mqtt::v5::connack>& response)
		 *   client : void(asio2::mqtt::v5::auth& auth, asio2::mqtt::v5::auth& response)
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
			using fun_traits_type = function_traits<F>;

			this->_argc_bind<fun_traits_type::argc>(type, std::move(f), ((dummy*)nullptr));
		}

		template<class F, class C>
		inline void _bind(mqtt::control_packet_type type, F f, C& c)
		{
			using fun_traits_type = function_traits<F>;

			this->_argc_bind<fun_traits_type::argc>(type, std::move(f), &c);
		}

		template<class F, class C>
		inline void _bind(mqtt::control_packet_type type, F f, C* c)
		{
			using fun_traits_type = function_traits<F>;

			this->_argc_bind<fun_traits_type::argc>(type, std::move(f), c);
		}

		// Argc == 1 : must be client, the callback signature : void (mqtt::xxx_message&)
		template<std::size_t Argc, class F, class C>
		typename std::enable_if_t<Argc == 1>
		inline _argc_bind(mqtt::control_packet_type type, F f, C* c)
		{
			using fun_traits_type = function_traits<F>;
			using message_type = typename std::remove_cv_t<std::remove_reference_t<
				typename fun_traits_type::template args<0>::type>>;

			this->template _version_bind<message_type, F, C>(type, std::move(f), c);
		}

		// Argc == 2 : client or server
		// if client, the callback signature : void (mqtt::xxx_message& message, mqtt::xxx_message& response)
		// if server, the callback signature : void (std::shared_ptr<xxx_session>& session_ptr, mqtt::xxx_message& message)
		template<std::size_t Argc, class F, class C>
		typename std::enable_if_t<Argc == 2>
		inline _argc_bind(mqtt::control_packet_type type, F f, C* c)
		{
			using fun_traits_type = function_traits<F>;
			using arg0_type = typename std::remove_cv_t<std::remove_reference_t<
				typename fun_traits_type::template args<0>::type>>;

			// must be server
			if constexpr (std::is_same_v<std::shared_ptr<caller_t>, arg0_type>)
			{
				using message_type = typename std::remove_cv_t<std::remove_reference_t<
					typename fun_traits_type::template args<1>::type>>;

				this->template _version_bind<message_type, F, C>(type, std::move(f), c);
			}
			// must be client
			else
			{
				using message_type = arg0_type;

				this->template _version_bind<message_type, F, C>(type, std::move(f), c);
			}
		}

		// Argc == 3 : must be server, the callback signature : 
		// void (std::shared_ptr<xxx_session>& session_ptr, mqtt::xxx_message& message, mqtt::xxx_message& response)
		template<std::size_t Argc, class F, class C>
		typename std::enable_if_t<Argc == 3>
		inline _argc_bind(mqtt::control_packet_type type, F f, C* c)
		{
			using fun_traits_type = function_traits<F>;
			using message_type = typename std::remove_cv_t<std::remove_reference_t<
				typename fun_traits_type::template args<1>::type>>;

			this->template _version_bind<message_type, F, C>(type, std::move(f), c);
		}

		template<class message_type, class F, class C>
		inline void _version_bind(mqtt::control_packet_type type, F f, C* c)
		{
			if constexpr /**/ (mqtt::is_v3_message<message_type>())
			{
				this->v3_handlers_[detail::to_underlying(type)] = std::bind(
					&self::template _proxy<mqtt::version::v3, F, C>,
					this, std::move(f), c,
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
			}
			else if constexpr (mqtt::is_v4_message<message_type>())
			{
				this->v4_handlers_[detail::to_underlying(type)] = std::bind(
					&self::template _proxy<mqtt::version::v4, F, C>,
					this, std::move(f), c,
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
			}
			else if constexpr (mqtt::is_v5_message<message_type>())
			{
				this->v5_handlers_[detail::to_underlying(type)] = std::bind(
					&self::template _proxy<mqtt::version::v5, F, C>,
					this, std::move(f), c,
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
			}
			else
			{
				ASIO2_ASSERT(false);
			}
		}

		template<mqtt::version V, class F, class C>
		inline void _proxy(F& f, C* c, error_code& ec,
			std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, std::string_view& data)
		{
			using fun_traits_type = function_traits<F>;

			_argc_proxy<fun_traits_type::argc, V>(f, c, ec, caller_ptr, caller, data);
		}

		// Argc == 1 : must be client, the callback signature : void (mqtt::xxx_message&)
		template<std::size_t Argc, mqtt::version V, class F, class C>
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
				mqtt::data_to_message<V>(data, [this, &f, &c, &ec, &caller_ptr, caller]
				(auto message) mutable
				{
					if constexpr (std::is_same_v<message_type, decltype(message)>)
					{
						ec.clear();

						caller->_before_user_callback(ec, caller_ptr, caller, message);

						this->_invoke_user_callback(f, c, message);

						caller->_after_user_callback(ec, caller_ptr, caller, message);

						this->_handle_mqtt_error(ec, caller_ptr, caller);
					}
					else
					{
						ASIO2_ASSERT(false &&
							"The parameters of the user callback function do not match."\
							" Check that the parameters of your callback function are of the correct type");

						if (!ec)
							ec = mqtt::make_error_code(mqtt::error::malformed_packet);

						this->_handle_mqtt_error(ec, caller_ptr, caller);
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
		template<std::size_t Argc, mqtt::version V, class F, class C>
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
					mqtt::data_to_message<V>(data, [this, &f, &c, &ec, &caller_ptr, caller]
					(auto message) mutable
					{
						if constexpr (std::is_same_v<message_type, decltype(message)>)
						{
							ec.clear();

							caller->_before_user_callback(ec, caller_ptr, caller, message);

							this->_invoke_user_callback(f, c, caller_ptr, message);

							caller->_after_user_callback(ec, caller_ptr, caller, message);

							this->_handle_mqtt_error(ec, caller_ptr, caller);
						}
						else
						{
							ASIO2_ASSERT(false &&
								"The parameters of the user callback function do not match."\
								" Check that the parameters of your callback function are of the correct type");

							if (!ec)
								ec = mqtt::make_error_code(mqtt::error::malformed_packet);

							this->_handle_mqtt_error(ec, caller_ptr, caller);
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
					mqtt::data_to_message<V>(data, [this, &f, &c, &ec, &caller_ptr, caller]
					(auto message) mutable
					{
						if constexpr (std::is_same_v<message_type, decltype(message)>)
						{
							ec.clear();

							if constexpr (detail::is_template_instance_of_v<std::variant, response_type>)
							{
								// a default-constructed variant holds a value of its first alternative
								response_type response;

								this->_initialize_response<V>(ec, caller_ptr, caller, message, response);

								if (response.index() != std::variant_npos)
								{
									std::visit([&ec, &caller_ptr, caller, &message](auto& rep) mutable
									{
										caller->_before_user_callback(ec, caller_ptr, caller, message, rep);
									}, response);
								}

								this->_invoke_user_callback(f, c, message, response);

								if (response.index() != std::variant_npos)
								{
									std::visit([&ec, &caller_ptr, caller, &message](auto& rep) mutable
									{
										caller->_after_user_callback(ec, caller_ptr, caller, message, rep);
									}, response);
								}

								this->_send_mqtt_response<V>(ec, caller_ptr, caller, message, std::move(response));

								this->_handle_mqtt_error(ec, caller_ptr, caller);
							}
							else
							{
								response_type response{};

								this->_initialize_response<V>(ec, caller_ptr, caller, message, response);

								caller->_before_user_callback(ec, caller_ptr, caller, message, response);

								this->_invoke_user_callback(f, c, message, response);

								caller->_after_user_callback(ec, caller_ptr, caller, message, response);

								this->_send_mqtt_response<V>(ec, caller_ptr, caller, message, std::move(response));

								this->_handle_mqtt_error(ec, caller_ptr, caller);
							}
						}
						else
						{
							ASIO2_ASSERT(false &&
								"The parameters of the user callback function do not match."\
								" Check that the parameters of your callback function are of the correct type");

							if (!ec)
								ec = mqtt::make_error_code(mqtt::error::malformed_packet);

							this->_handle_mqtt_error(ec, caller_ptr, caller);
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
		template<std::size_t Argc, mqtt::version V, class F, class C>
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
				mqtt::data_to_message<V>(data, [this, &f, &c, &ec, &caller_ptr, caller]
				(auto message) mutable
				{
					if constexpr (std::is_same_v<message_type, decltype(message)>)
					{
						ec.clear();

						if constexpr (detail::is_template_instance_of_v<std::variant, response_type>)
						{
							// a default-constructed variant holds a value of its first alternative
							response_type response;

							this->_initialize_response<V>(ec, caller_ptr, caller, message, response);

							if (response.index() != std::variant_npos)
							{
								std::visit([&ec, &caller_ptr, caller, &message](auto& rep) mutable
								{
									caller->_before_user_callback(ec, caller_ptr, caller, message, rep);
								}, response);
							}

							this->_invoke_user_callback(f, c, caller_ptr, message, response);

							if (response.index() != std::variant_npos)
							{
								std::visit([&ec, &caller_ptr, caller, &message](auto& rep) mutable
								{
									caller->_after_user_callback(ec, caller_ptr, caller, message, rep);
								}, response);
							}

							this->_send_mqtt_response<V>(ec, caller_ptr, caller, message, std::move(response));

							this->_handle_mqtt_error(ec, caller_ptr, caller);
						}
						else
						{
							response_type response{};

							this->_initialize_response<V>(ec, caller_ptr, caller, message, response);

							caller->_before_user_callback(ec, caller_ptr, caller, message, response);

							this->_invoke_user_callback(f, c, caller_ptr, message, response);

							caller->_after_user_callback(ec, caller_ptr, caller, message, response);

							this->_send_mqtt_response<V>(ec, caller_ptr, caller, message, std::move(response));

							this->_handle_mqtt_error(ec, caller_ptr, caller);
						}
					}
					else
					{
						ASIO2_ASSERT(false &&
							"The parameters of the user callback function do not match."\
							" Check that the parameters of your callback function are of the correct type");

						if (!ec)
							ec = mqtt::make_error_code(mqtt::error::malformed_packet);

						this->_handle_mqtt_error(ec, caller_ptr, caller);
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

		template<typename F, typename C, typename... Args>
		inline void _invoke_user_callback(F& f, C* c, Args&&... args)
		{
			detail::ignore_unused(c);

			if constexpr (std::is_same_v<detail::remove_cvref_t<C>, dummy>)
				f(std::forward<Args>(args)...);
			else
				(c->*f)(std::forward<Args>(args)...);
		}

		template<mqtt::version V, class Message, class Response>
		inline void _initialize_response(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, Message& msg, Response& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);

			using message_type  [[maybe_unused]] = typename detail::remove_cvref_t<Message>;
			using response_type [[maybe_unused]] = typename detail::remove_cvref_t<Response>;

			if constexpr (
				std::is_same_v<message_type, mqtt::v3::publish> ||
				std::is_same_v<message_type, mqtt::v4::publish> ||
				std::is_same_v<message_type, mqtt::v5::publish>)
			{
				if constexpr /**/ (V == mqtt::version::v3)
				{
					switch (msg.qos())
					{
					// the qos 0 publish messgae don't need response
					case mqtt::qos_type::at_most_once : break;
					case mqtt::qos_type::at_least_once: response = mqtt::v3::puback{}; break;
					case mqtt::qos_type::exactly_once : response = mqtt::v3::pubrec{}; break;
					default:break;
					}
				}
				else if constexpr (V == mqtt::version::v4)
				{
					switch (msg.qos())
					{
					// the qos 0 publish messgae don't need response
					case mqtt::qos_type::at_most_once : break;
					case mqtt::qos_type::at_least_once: response = mqtt::v4::puback{}; break;
					case mqtt::qos_type::exactly_once : response = mqtt::v4::pubrec{}; break;
					default:break;
					}
				}
				else if constexpr (V == mqtt::version::v5)
				{
					switch (msg.qos())
					{
					// the qos 0 publish messgae don't need response
					case mqtt::qos_type::at_most_once : break;
					case mqtt::qos_type::at_least_once: response = mqtt::v5::puback{}; break;
					case mqtt::qos_type::exactly_once : response = mqtt::v5::pubrec{}; break;
					default:break;
					}
				}
			}
			else
			{
				std::ignore = true;
			}
		}

		template<mqtt::version V, class Message, class Response>
		inline void _send_mqtt_response(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller,
			Message& msg, Response&& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);

			using message_type  [[maybe_unused]] = typename detail::remove_cvref_t<Message>;
			using response_type [[maybe_unused]] = typename detail::remove_cvref_t<Response>;

			// std::variant<response_message1, response_message2, response_message3>
			if constexpr (detail::is_template_instance_of_v<std::variant, response_type>)
			{
				if (response.index() == std::variant_npos)
					return;

				bool send_flag = true;

				std::visit([&send_flag](auto& rep) mutable { send_flag = rep.send_flag(); }, response);

				if (send_flag == false)
					return;

				// can't use async_send, beacuse the caller maybe not started yet
				caller->push_event([caller_ptr, caller, response = std::forward<Response>(response)]
				(event_queue_guard<caller_t> g) mutable
				{
					detail::ignore_unused(caller_ptr);

					std::visit([caller, g = std::move(g)](auto& rep) mutable
					{
						caller->_do_send(rep, [g = std::move(g)](const error_code&, std::size_t) mutable {});
					}, response);
				});
			}
			// response_message
			else
			{
				if (response.send_flag() == false)
					return;

				// can't use async_send, beacuse the caller maybe not started yet
				caller->push_event([caller_ptr, caller, response = std::forward<Response>(response)]
				(event_queue_guard<caller_t> g) mutable
				{
					detail::ignore_unused(caller_ptr);

					caller->_do_send(response, [g = std::move(g)](const error_code&, std::size_t) mutable {});
				});
			}
		}

		inline void _call_mqtt_handler(mqtt::control_packet_type type, error_code& ec,
			std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, std::string_view& data)
		{
			ASIO2_ASSERT(caller->io().strand().running_in_this_thread());

			handler_type* f = nullptr;

			mqtt::version ver = caller->version();

			if /**/ (ver == mqtt::version::v3)
			{
				if (detail::to_underlying(type) < this->v3_handlers_.size())
					f = &(this->v3_handlers_[detail::to_underlying(type)]);
			}
			else if (ver == mqtt::version::v4)
			{
				if (detail::to_underlying(type) < this->v4_handlers_.size())
					f = &(this->v4_handlers_[detail::to_underlying(type)]);
			}
			else if (ver == mqtt::version::v5)
			{
				if (detail::to_underlying(type) < this->v5_handlers_.size())
					f = &(this->v5_handlers_[detail::to_underlying(type)]);
			}

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

		inline void _handle_mqtt_error(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller)
		{
			if (!ec)
				return;

			// post a async event to disconnect, don't call _do_disconnect directly,
			// otherwise the client's bind_disconnect callback maybe can't be called.
			asio::post(caller->io().strand(), make_allocator(caller->wallocator(),
			[ec, caller_ptr, caller]() mutable
			{
				if (caller->state() == state_t::started)
				{
					caller->_do_disconnect(ec, std::move(caller_ptr));
				}
			}));
		}

	protected:
		template<mqtt::version Version>
		inline const handler_type& _find_mqtt_handler(mqtt::control_packet_type type)
		{
			if constexpr /**/ (Version == mqtt::version::v3)
			{
				if (detail::to_underlying(type) < this->v3_handlers_.size())
					return v3_handlers_[detail::to_underlying(type)];
			}
			else if constexpr (Version == mqtt::version::v4)
			{
				if (detail::to_underlying(type) < this->v4_handlers_.size())
					return v4_handlers_[detail::to_underlying(type)];
			}
			else if constexpr (Version == mqtt::version::v5)
			{
				if (detail::to_underlying(type) < this->v5_handlers_.size())
					return v5_handlers_[detail::to_underlying(type)];
			}

			return this->dummy_handler_;
		}

	protected:
		handler_type dummy_handler_;

		std::array<handler_type, magic_enum::enum_count<mqtt::control_packet_type>()> v3_handlers_;
		std::array<handler_type, magic_enum::enum_count<mqtt::control_packet_type>()> v4_handlers_;
		std::array<handler_type, magic_enum::enum_count<mqtt::control_packet_type>()> v5_handlers_;

		std::array<handler_type, magic_enum::enum_count<mqtt::qos_type>()> v3_publish_handlers_;
		std::array<handler_type, magic_enum::enum_count<mqtt::qos_type>()> v4_publish_handlers_;
		std::array<handler_type, magic_enum::enum_count<mqtt::qos_type>()> v5_publish_handlers_;
	};
}

#endif // !__ASIO2_MQTT_INVOKER_HPP__
