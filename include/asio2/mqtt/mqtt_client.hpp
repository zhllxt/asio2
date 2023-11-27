/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_MQTT_CLIENT_HPP__
#define __ASIO2_MQTT_CLIENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/base/detail/shared_mutex.hpp>

#include <asio2/tcp/tcp_client.hpp>

#include <asio2/mqtt/impl/mqtt_send_connect_op.hpp>
#include <asio2/mqtt/impl/mqtt_send_op.hpp>

#include <asio2/mqtt/detail/mqtt_handler.hpp>
#include <asio2/mqtt/detail/mqtt_invoker.hpp>
#include <asio2/mqtt/detail/mqtt_topic_alias.hpp>
#include <asio2/mqtt/detail/mqtt_session_state.hpp>
#include <asio2/mqtt/detail/mqtt_message_router.hpp>
#include <asio2/mqtt/detail/mqtt_subscribe_router.hpp>

#include <asio2/mqtt/options.hpp>

#include <asio2/util/uuid.hpp>

namespace asio2::detail
{
	struct template_args_mqtt_client : public template_args_tcp_client
	{
		static constexpr bool rdc_call_cp_enabled = false;

		template<class caller_t>
		struct subnode
		{
			explicit subnode(
				std::weak_ptr<caller_t>  c,
				mqtt::subscription       s,
				mqtt::v5::properties_set p = mqtt::v5::properties_set{}
			)
				: caller(std::move(c))
				, sub   (std::move(s))
				, props (std::move(p))
			{
			}

			inline std::string_view share_name  () { return sub.share_name  (); }
			inline std::string_view topic_filter() { return sub.topic_filter(); }

			// 
			std::weak_ptr<caller_t>   caller;

			// subscription info
			mqtt::subscription        sub;

			// subscription properties
			mqtt::v5::properties_set  props;

			detail::function<void(mqtt::message&)> callback;
		};
	};

	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;

	template<class derived_t, class args_t = template_args_mqtt_client>
	class mqtt_client_impl_t
		: public tcp_client_impl_t      <derived_t, args_t>
		, public mqtt_options
		, public mqtt_handler_t         <derived_t, args_t>
		, public mqtt_invoker_t         <derived_t, args_t>
		, public mqtt_message_router_t  <derived_t, args_t>
		, public mqtt_subscribe_router_t<derived_t, args_t>
		, public mqtt_topic_alias_t     <derived_t, args_t>
		, public mqtt_send_op           <derived_t, args_t>
		, public mqtt::session_state
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;

	public:
		using super = tcp_client_impl_t <derived_t, args_t>;
		using self  = mqtt_client_impl_t<derived_t, args_t>;

		using args_type    = args_t;
		using subnode_type = typename args_type::template subnode<derived_t>;

		using super::send;
		using super::async_send;

	public:
		/**
		 * @brief constructor
		 */
		explicit mqtt_client_impl_t(
			std::size_t init_buf_size = tcp_frame_size,
			std::size_t max_buf_size  = mqtt::max_payload,
			std::size_t concurrency   = 1
		)
			: super(init_buf_size, max_buf_size, concurrency)
			, mqtt_options                              ()
			, mqtt_handler_t         <derived_t, args_t>()
			, mqtt_invoker_t         <derived_t, args_t>()
			, mqtt_message_router_t  <derived_t, args_t>()
			, mqtt_subscribe_router_t<derived_t, args_t>()
			, mqtt_topic_alias_t     <derived_t, args_t>()
			, mqtt_send_op           <derived_t, args_t>()
			, pingreq_timer_(this->io_->context())
		{
		}

		template<class Scheduler, std::enable_if_t<!std::is_integral_v<detail::remove_cvref_t<Scheduler>>, int> = 0>
		explicit mqtt_client_impl_t(
			std::size_t init_buf_size,
			std::size_t max_buf_size,
			Scheduler&& scheduler
		)
			: super(init_buf_size, max_buf_size, std::forward<Scheduler>(scheduler))
			, mqtt_options                              ()
			, mqtt_handler_t         <derived_t, args_t>()
			, mqtt_invoker_t         <derived_t, args_t>()
			, mqtt_message_router_t  <derived_t, args_t>()
			, mqtt_subscribe_router_t<derived_t, args_t>()
			, mqtt_topic_alias_t     <derived_t, args_t>()
			, mqtt_send_op           <derived_t, args_t>()
			, pingreq_timer_(this->io_->context())
		{
		}

		template<class Scheduler, std::enable_if_t<!std::is_integral_v<detail::remove_cvref_t<Scheduler>>, int> = 0>
		explicit mqtt_client_impl_t(Scheduler&& scheduler)
			: mqtt_client_impl_t(tcp_frame_size, mqtt::max_payload, std::forward<Scheduler>(scheduler))
		{
		}

		/**
		 * @brief destructor
		 */
		~mqtt_client_impl_t()
		{
			this->stop();
		}

		/**
		 * @brief start the client, blocking connect to server
		 * @param host - A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port - A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt, typename... Args>
		inline bool start(String&& host, StrOrInt&& port, Args&&... args)
		{
			if constexpr (sizeof...(Args) > std::size_t(0))
				return this->derived().template _do_connect_with_connect_message<false>(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					std::forward<Args>(args)...);
			else
				return this->derived().template _do_connect<false>(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					ecs_helper::make_ecs(asio::transfer_at_least(1),
						mqtt::mqtt_match_role, std::forward<Args>(args)...));
		}

		/**
		 * @brief start the client, asynchronous connect to server
		 * @param host - A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port - A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt, typename... Args>
		inline bool async_start(String&& host, StrOrInt&& port, Args&&... args)
		{
			if constexpr (sizeof...(Args) > std::size_t(0))
				return this->derived().template _do_connect_with_connect_message<true>(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					std::forward<Args>(args)...);
			else
				return this->derived().template _do_connect<true>(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					ecs_helper::make_ecs(asio::transfer_at_least(1),
						mqtt::mqtt_match_role, std::forward<Args>(args)...));
		}

	public:
		/**
		 * @brief get the mqtt version number
		 */
		inline mqtt::version version() const
		{
			return this->get_version();
		}

		/**
		 * @brief get the mqtt version number
		 */
		inline mqtt::version get_version() const
		{
			if /**/ (std::holds_alternative<mqtt::v3::connect>(connect_message_.base()))
			{
				return mqtt::version::v3;
			}
			else if (std::holds_alternative<mqtt::v4::connect>(connect_message_.base()))
			{
				return mqtt::version::v4;
			}
			else if (std::holds_alternative<mqtt::v5::connect>(connect_message_.base()))
			{
				return mqtt::version::v5;
			}

			ASIO2_ASSERT(false);
			return static_cast<mqtt::version>(0);
		}

		/**
		 * @brief get the mqtt client identifier 
		 */
		inline std::string_view client_id() const
		{
			return this->get_client_id();
		}

		/**
		 * @brief get the mqtt client identifier 
		 */
		inline std::string_view get_client_id() const
		{
			std::string_view v{};
			if (!this->connect_message_.empty())
			{
				if /**/ (std::holds_alternative<mqtt::v3::connect>(connect_message_.base()))
				{
					v = connect_message_.template get<mqtt::v3::connect>().client_id();
				}
				else if (std::holds_alternative<mqtt::v4::connect>(connect_message_.base()))
				{
					v = connect_message_.template get<mqtt::v4::connect>().client_id();
				}
				else if (std::holds_alternative<mqtt::v5::connect>(connect_message_.base()))
				{
					v = connect_message_.template get<mqtt::v5::connect>().client_id();
				}
			}
			if (v.empty())
			{
				if (const mqtt::v5::connack* m = std::get_if<mqtt::v5::connack>(std::addressof(connack_message_.base())))
				{
					const mqtt::v5::assigned_client_identifier* p =
						m->properties().get_if<mqtt::v5::assigned_client_identifier>();
					if (p)
						v = p->value();
				}
			}
			return v;
		}

		/**
		 * @brief get the mqtt Keep Alive which is a time interval measured in seconds. 
		 */
		inline std::uint16_t keep_alive_time() const
		{
			return this->get_keep_alive_time();
		}

		/**
		 * @brief get the mqtt Keep Alive which is a time interval measured in seconds. 
		 */
		inline std::uint16_t get_keep_alive_time() const
		{
			//The Keep Alive is a Two Byte Integer which is a time interval measured in seconds.
			// It is the maximum time interval that is permitted to elapse between the point at
			// which the Client finishes transmitting one MQTT Control Packet and the point it
			// starts sending the next. It is the responsibility of the Client to ensure that
			// the interval between MQTT Control Packets being sent does not exceed the Keep 
			// Alive value. If Keep Alive is non-zero and in the absence of sending any other 
			// MQTT Control Packets, the Client MUST send a PINGREQ packet [MQTT-3.1.2-20].
			// If the Server returns a Server Keep Alive on the CONNACK packet, the Client MUST 
			// use that value instead of the value it sent as the Keep Alive [MQTT-3.1.2-21].
			if (const mqtt::v5::connack* m = std::get_if<mqtt::v5::connack>(std::addressof(connack_message_.base())))
			{
				const mqtt::v5::server_keep_alive* p =
					m->properties().get_if<mqtt::v5::server_keep_alive>();
				if (p)
					return p->value();
			}
			// Default to 60 seconds
			std::uint16_t v = 60;
			if (!this->connect_message_.empty())
			{
				if /**/ (std::holds_alternative<mqtt::v3::connect>(connect_message_.base()))
				{
					v = this->connect_message_.template get_if<mqtt::v3::connect>()->keep_alive();
				}
				else if (std::holds_alternative<mqtt::v4::connect>(connect_message_.base()))
				{
					v = this->connect_message_.template get_if<mqtt::v4::connect>()->keep_alive();
				}
				else if (std::holds_alternative<mqtt::v5::connect>(connect_message_.base()))
				{
					v = this->connect_message_.template get_if<mqtt::v5::connect>()->keep_alive();
				}
			}
			return v;
		}

		/**
		 * @brief set the mqtt connect message packet
		 */
		template<class Message>
		inline derived_t& set_connect_message(Message&& connect_msg)
		{
			using msg_type = typename detail::remove_cvref_t<Message>;

			if constexpr (
				std::is_same_v<msg_type, mqtt::v3::connect> ||
				std::is_same_v<msg_type, mqtt::v4::connect> ||
				std::is_same_v<msg_type, mqtt::v5::connect>)
			{
				this->connect_message_ = std::forward<Message>(connect_msg);
			}
			else
			{
				static_assert(detail::always_false_v<Message>);
			}

			return (static_cast<derived_t&>(*this));
		}

		/**
		 * @brief get the mqtt connect message reference
		 */
		inline mqtt::message& get_connect_message() { return this->connect_message_; }

		/**
		 * @brief get the mqtt connect message reference
		 */
		inline mqtt::message const& get_connect_message() const { return this->connect_message_; }

		/**
		 * @brief get the mqtt connect message packet reference
		 */
		template<mqtt::version v>
		inline auto& get_connect_packet()
		{
			if constexpr /**/ (mqtt::version::v3 == v)
			{
				return std::get<mqtt::v3::connect>(this->connect_message_.base());
			}
			else if constexpr (mqtt::version::v4 == v)
			{
				return std::get<mqtt::v4::connect>(this->connect_message_.base());
			}
			else if constexpr (mqtt::version::v5 == v)
			{
				return std::get<mqtt::v5::connect>(this->connect_message_.base());
			}
			else
			{
				static_assert(mqtt::version::v3 == v || mqtt::version::v4 == v || mqtt::version::v5 == v);
			}
		}

	protected:
		template<bool IsAsync, typename String, typename StrOrInt, typename Arg1, typename... Args>
		bool _do_connect_with_connect_message(String&& host, StrOrInt&& port, Arg1&& arg1, Args&&... args)
		{
			using arg1_type = typename detail::remove_cvref_t<Arg1>;

			if constexpr (
				std::is_same_v<arg1_type, mqtt::v3::connect> ||
				std::is_same_v<arg1_type, mqtt::v4::connect> ||
				std::is_same_v<arg1_type, mqtt::v5::connect>)
			{
				this->connect_message_ = std::forward<Arg1>(arg1);

				return this->derived().template _do_connect<IsAsync>(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					ecs_helper::make_ecs(asio::transfer_at_least(1),
						mqtt::mqtt_match_role, std::forward<Args>(args)...));
			}
			else
			{
				return this->derived().template _do_connect<IsAsync>(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					ecs_helper::make_ecs(asio::transfer_at_least(1),
						mqtt::mqtt_match_role, std::forward<Arg1>(arg1), std::forward<Args>(args)...));
			}
		}

		template<bool IsAsync, typename String, typename StrOrInt, typename C>
		inline bool _do_connect(String&& host, StrOrInt&& port, std::shared_ptr<ecs_t<C>> ecs)
		{
			if (!this->connect_message_.template holds<mqtt::v3::connect, mqtt::v4::connect, mqtt::v5::connect>())
			{
				ASIO2_ASSERT(false);
				set_last_error(asio::error::invalid_argument);
				return false;
			}

			return super::template _do_connect<IsAsync>(
				std::forward<String>(host), std::forward<StrOrInt>(port), std::move(ecs));
		}

		template<typename C>
		inline void _bind_default_mqtt_handler(std::shared_ptr<ecs_t<C>>& ecs)
		{
			detail::ignore_unused(ecs);

			// must set default callback for every mqtt message.
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::connect    ))) this->on_connect    ([](mqtt::message&                ) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::connack    ))) this->on_connack    ([](mqtt::message&                ) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::publish    ))) this->on_publish    ([](mqtt::message&, mqtt::message&) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::puback     ))) this->on_puback     ([](mqtt::message&                ) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::pubrec     ))) this->on_pubrec     ([](mqtt::message&, mqtt::message&) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::pubrel     ))) this->on_pubrel     ([](mqtt::message&, mqtt::message&) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::pubcomp    ))) this->on_pubcomp    ([](mqtt::message&                ) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::subscribe  ))) this->on_subscribe  ([](mqtt::message&                ) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::suback     ))) this->on_suback     ([](mqtt::message&                ) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::unsubscribe))) this->on_unsubscribe([](mqtt::message&                ) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::unsuback   ))) this->on_unsuback   ([](mqtt::message&                ) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::pingreq    ))) this->on_pingreq    ([](mqtt::message&, mqtt::message&) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::pingresp   ))) this->on_pingresp   ([](mqtt::message&                ) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::disconnect ))) this->on_disconnect ([](mqtt::message&                ) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::auth       ))) this->on_auth       ([](mqtt::message&, mqtt::message&) mutable {});
		}

	protected:
		template<typename C>
		inline void _do_init(std::shared_ptr<ecs_t<C>>& ecs)
		{
			// must set default callback for every mqtt message.
			this->derived()._bind_default_mqtt_handler(ecs);

			super::_do_init(ecs);
		}

		template<typename E = defer_event<void, derived_t>>
		inline void _do_disconnect(
			const error_code& ec, std::shared_ptr<derived_t> this_ptr, E chain = defer_event<void, derived_t>{})
		{
			state_t expected = state_t::started;
			if (this->derived().state_.compare_exchange_strong(expected, state_t::started))
			{
				mqtt::version ver = this->derived().version();
				if /**/ (ver == mqtt::version::v3)
				{
					mqtt::v3::disconnect disconnect;
					this->derived().internal_async_send(std::move(this_ptr), std::move(disconnect),
					[this, ec, e = chain.move_event()]
					(std::shared_ptr<derived_t> this_ptr, const error_code&,
						std::size_t, event_queue_guard<derived_t> g) mutable
					{
						defer_event chain(std::move(e), std::move(g));
						super::_do_disconnect(ec, std::move(this_ptr), std::move(chain));
					}, chain.move_guard());
					return;
				}
				else if (ver == mqtt::version::v4)
				{
					mqtt::v4::disconnect disconnect;
					this->derived().internal_async_send(std::move(this_ptr), std::move(disconnect),
					[this, ec, e = chain.move_event()]
					(std::shared_ptr<derived_t> this_ptr, const error_code&,
						std::size_t, event_queue_guard<derived_t> g) mutable
					{
						defer_event chain(std::move(e), std::move(g));
						super::_do_disconnect(ec, std::move(this_ptr), std::move(chain));
					}, chain.move_guard());
					return;
				}
				else if (ver == mqtt::version::v5)
				{
					mqtt::v5::disconnect disconnect;
					switch (ec.value())
					{
					// https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901208
					case 0   : // Client or Server
					case 4   : // Client
					case 128 : // Client or Server
					case 129 : // Client or Server
					case 130 : // Client or Server
					case 131 : // Client or Server
					case 144 : // Client or Server
					case 147 : // Client or Server
					case 148 : // Client or Server
					case 149 : // Client or Server
					case 150 : // Client or Server
					case 151 : // Client or Server
					case 152 : // Client or Server
					case 153 : // Client or Server
						disconnect.reason_code(static_cast<std::uint8_t>(ec.value())); break;
					default: break;
					}
					this->derived().internal_async_send(std::move(this_ptr), std::move(disconnect),
					[this, ec, e = chain.move_event()]
					(std::shared_ptr<derived_t> this_ptr, const error_code&,
						std::size_t, event_queue_guard<derived_t> g) mutable
					{
						defer_event chain(std::move(e), std::move(g));
						super::_do_disconnect(ec, std::move(this_ptr), std::move(chain));
					}, chain.move_guard());
					return;
				}
			}

			super::_do_disconnect(ec, std::move(this_ptr), std::move(chain));
		}

		template<typename C, typename DeferEvent>
		inline void _handle_connect(
			const error_code& ec,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			derived_t& derive = this->derived();

			set_last_error(ec);

			if (ec)
			{
				return derive._done_connect(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
			}

			// send connect message to server use coroutine 
			mqtt_send_connect_op
			{
				derive.io_->context(),
				derive.connect_message_,
				derive.stream(),
				[&derive, this_ptr = std::move(this_ptr), ecs = std::move(ecs), chain = std::move(chain)]
				(error_code ec, std::unique_ptr<asio::streambuf> stream) mutable
				{
					derive._handle_mqtt_connect_response(ec, std::move(this_ptr), std::move(ecs),
						std::move(stream), std::move(chain));
				}
			};
		}

		template<typename C, typename DeferEvent>
		inline void _handle_mqtt_connect_response(
			error_code ec,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs,
			std::unique_ptr<asio::streambuf> stream, DeferEvent chain)
		{
			if (ec)
			{
				this->derived()._done_connect(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
				return;
			}

			std::string_view data{ reinterpret_cast<std::string_view::const_pointer>(
					static_cast<const char*>(stream->data().data())), stream->size() };

			mqtt::control_packet_type type = mqtt::message_type_from_data(data);

			bool valid_message = (type == mqtt::control_packet_type::connack) ||
				(this->derived().version() == mqtt::version::v5 && type == mqtt::control_packet_type::auth);

			// -- the connect_timeout_cp will disconnect after a reasonable amount of time.

			// If the client does not receive a CONNACK message from the server within a reasonable amount
			// of time, the client should close the TCP/IP socket connection,
			// and restart the session by opening a new socket to the server and issuing a CONNECT message.

			if (!valid_message)
			{
				ASIO2_ASSERT(false);
				ec = mqtt::make_error_code(mqtt::error::malformed_packet);
				this->derived()._done_connect(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
				return;
			}

			this->idmgr_.clear();

			ec = mqtt::make_error_code(mqtt::error::server_unavailable);

			this->derived()._call_mqtt_handler(type, ec, this_ptr, static_cast<derived_t*>(this), data);

			this->derived()._done_connect(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
		}

		template<typename C, typename DeferEvent>
		inline void _do_start(
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			super::_do_start(this_ptr, std::move(ecs), std::move(chain));

			this->derived()._post_pingreq_timer(
				std::move(this_ptr), std::chrono::seconds(this->derived().keep_alive_time()));
		}

		template<class Rep, class Period>
		inline void _post_pingreq_timer(
			std::shared_ptr<derived_t> this_ptr, std::chrono::duration<Rep, Period> duration)
		{
			derived_t& derive = this->derived();

			// start the timer 
			if (duration > std::chrono::duration<Rep, Period>::zero() && this->is_started())
			{
				this->pingreq_timer_.expires_after(duration);
				this->pingreq_timer_.async_wait(
				[&derive, this_ptr = std::move(this_ptr)](const error_code& ec) mutable
				{
					derive._handle_pingreq_timer(ec, std::move(this_ptr));
				});
			}
		}

		inline void _handle_pingreq_timer(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			derived_t& derive = this->derived();

			ASIO2_ASSERT(derive.io_->running_in_this_thread());
			ASIO2_ASSERT((!ec) || ec == asio::error::operation_aborted);

			if (ec)
				return;

			// The Client can send PINGREQ at any time, irrespective of the Keep Alive value, and check
			// for a corresponding PINGRESP to determine that the network and the Server are available.
			// If the Keep Alive value is non-zero and the Server does not receive an MQTT Control Packet
			// from the Client within one and a half times the Keep Alive time period, it MUST close the
			// Network Connection to the Client as if the network had failed [MQTT-3.1.2-22].
			// If a Client does not receive a PINGRESP packet within a reasonable amount of time after it
			// has sent a PINGREQ, it SHOULD close the Network Connection to the Server.
			// A Keep Alive value of 0 has the effect of turning off the Keep Alive mechanism. If Keep Alive
			// is 0 the Client is not obliged to send MQTT Control Packets on any particular schedule.

			// send pingreq message, don't case the last sent and recved time.
			mqtt::version ver = derive.version();
			if /**/ (ver == mqtt::version::v3)
			{
				derive.internal_async_send(this_ptr, mqtt::v3::pingreq{});
			}
			else if (ver == mqtt::version::v4)
			{
				derive.internal_async_send(this_ptr, mqtt::v4::pingreq{});
			}
			else if (ver == mqtt::version::v5)
			{
				derive.internal_async_send(this_ptr, mqtt::v5::pingreq{});
			}

			// do next timer
			derive._post_pingreq_timer(std::move(this_ptr), std::chrono::seconds(derive.keep_alive_time()));
		}

		inline void _stop_pingreq_timer()
		{
			detail::cancel_timer(this->pingreq_timer_);
		}

		template<typename DeferEvent>
		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			this->derived()._stop_pingreq_timer();

			super::_handle_disconnect(ec, std::move(this_ptr), std::move(chain));
		}

		template<typename DeferEvent>
		inline void _handle_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			super::_handle_stop(ec, std::move(this_ptr), std::move(chain));
		}

		template<class Data, class Callback>
		inline bool _do_send(Data& data, Callback&& callback)
		{
			return this->derived()._mqtt_send(data, std::forward<Callback>(callback));
		}

	protected:
		template<typename C>
		inline void _fire_recv(
			std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs, std::string_view data)
		{
			data = detail::call_data_filter_before_recv(this->derived(), data);

			this->listener_.notify(event_type::recv, data);

			this->derived()._rdc_handle_recv(this_ptr, ecs, data);

			mqtt::control_packet_type type = mqtt::message_type_from_data(data);

			if (type > mqtt::control_packet_type::auth)
			{
				ASIO2_ASSERT(false);

				// give a error callback and call it ?

				return;
			}

			error_code ec;

			this->derived()._call_mqtt_handler(type, ec, this_ptr, static_cast<derived_t*>(this), data);

			if (ec)
			{
				// give a error callback and call it ?
			}
		}

	protected:
		/// Should we set a default mqtt version to v4, default client id to a uuid string ?
		mqtt::message                  connect_message_{/* mqtt::v4::connect{ asio2::uuid().next().str() } */};

		/// 
		mqtt::message                  connack_message_{};

		/// timer for pingreq
		asio::steady_timer             pingreq_timer_;

		/// packet id manager
		mqtt::idmgr<std::atomic<mqtt::two_byte_integer::value_type>>           idmgr_;
	};
}

namespace asio2
{
	using mqtt_client_args = detail::template_args_mqtt_client;

	template<class derived_t, class args_t>
	using mqtt_client_impl_t = detail::mqtt_client_impl_t<derived_t, args_t>;

	template<class derived_t>
	class mqtt_client_t : public detail::mqtt_client_impl_t<derived_t, detail::template_args_mqtt_client>
	{
	public:
		using detail::mqtt_client_impl_t<derived_t, detail::template_args_mqtt_client>::mqtt_client_impl_t;
	};

	class mqtt_client : public mqtt_client_t<mqtt_client>
	{
	public:
		using mqtt_client_t<mqtt_client>::mqtt_client_t;
	};
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_MQTT_CLIENT_HPP__
