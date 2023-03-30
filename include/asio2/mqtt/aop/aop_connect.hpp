/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_MQTT_AOP_CONNECT_HPP__
#define __ASIO2_MQTT_AOP_CONNECT_HPP__

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
	class mqtt_aop_connect
	{
		friend caller_t;

	protected:
		template<class Response>
		inline void _set_connack_reason_code_with_invalid_id(error_code& ec, Response& rep)
		{
			using response_type = typename detail::remove_cvref_t<Response>;

			ec = mqtt::make_error_code(mqtt::error::client_identifier_not_valid);

			if constexpr /**/ (std::is_same_v<response_type, mqtt::v3::connack>)
			{
				rep.reason_code(mqtt::v3::connect_reason_code::identifier_rejected);
			}
			else if constexpr (std::is_same_v<response_type, mqtt::v4::connack>)
			{
				rep.reason_code(mqtt::v4::connect_reason_code::identifier_rejected);
			}
			else if constexpr (std::is_same_v<response_type, mqtt::v5::connack>)
			{
				rep.reason_code(mqtt::error::client_identifier_not_valid);
			}
			else
			{
				static_assert(detail::always_false_v<Response>);
			}
		}

		template<class Response>
		inline void _set_connack_reason_code_with_bad_user_name_or_password(error_code& ec, Response& rep)
		{
			using response_type = typename detail::remove_cvref_t<Response>;

			ec = mqtt::make_error_code(mqtt::error::bad_user_name_or_password);

			if constexpr /**/ (std::is_same_v<response_type, mqtt::v3::connack>)
			{
				rep.reason_code(mqtt::v3::connect_reason_code::bad_user_name_or_password);
			}
			else if constexpr (std::is_same_v<response_type, mqtt::v4::connack>)
			{
				rep.reason_code(mqtt::v4::connect_reason_code::bad_user_name_or_password);
			}
			else if constexpr (std::is_same_v<response_type, mqtt::v5::connack>)
			{
				rep.reason_code(mqtt::error::bad_user_name_or_password);
			}
			else
			{
				static_assert(detail::always_false_v<Response>);
			}
		}

		template<class Response>
		inline void _set_connack_reason_code_with_not_authorized(error_code& ec, Response& rep)
		{
			using response_type = typename detail::remove_cvref_t<Response>;

			ec = mqtt::make_error_code(mqtt::error::not_authorized);

			if constexpr /**/ (std::is_same_v<response_type, mqtt::v3::connack>)
			{
				rep.reason_code(mqtt::v3::connect_reason_code::not_authorized);
			}
			else if constexpr (std::is_same_v<response_type, mqtt::v4::connack>)
			{
				rep.reason_code(mqtt::v4::connect_reason_code::not_authorized);
			}
			else if constexpr (std::is_same_v<response_type, mqtt::v5::connack>)
			{
				rep.reason_code(mqtt::error::not_authorized);
			}
			else
			{
				static_assert(detail::always_false_v<Response>);
			}
		}

		template<class Message, class Response>
		inline void _check_connect_client_id(error_code& ec, caller_t* caller, Message& msg, Response& rep)
		{
			using message_type  [[maybe_unused]] = typename detail::remove_cvref_t<Message>;
			using response_type [[maybe_unused]] = typename detail::remove_cvref_t<Response>;

			std::string_view client_id = msg.client_id();

			if constexpr /**/ (std::is_same_v<response_type, mqtt::v3::connack>)
			{
				// The first UTF-encoded string. The Client Identifier (Client ID) is between 1 and 23
				// characters long, and uniquely identifies the client to the server. It must be unique
				// across all clients connecting to a single server, and is the key in handling Message
				// IDs messages with QoS levels 1 and 2. If the Client ID contains more than 23 characters,
				// the server responds to the CONNECT message with a CONNACK return code 2: Identifier Rejected.
				if (client_id.size() < static_cast<std::string_view::size_type>(1) ||
					client_id.size() > static_cast<std::string_view::size_type>(23))
					return _set_connack_reason_code_with_invalid_id(ec, rep);
			}
			else if constexpr (std::is_same_v<response_type, mqtt::v4::connack>)
			{
				// If the Client supplies a zero-byte ClientId, the Client MUST also set CleanSession 
				// to 1[MQTT-3.1.3-7].
				// If the Client supplies a zero-byte ClientId with CleanSession set to 0, the Server 
				// MUST respond to the CONNECT Packet with a CONNACK return code 0x02 (Identifier rejected) 
				// and then close the Network Connection[MQTT-3.1.3-8].
				// If the Server rejects the ClientId it MUST respond to the CONNECT Packet with a CONNACK
				// return code 0x02 (Identifier rejected) and then close the Network Connection[MQTT-3.1.3-9].
				if (client_id.empty())
				{
					if (msg.clean_session() == false)
						return _set_connack_reason_code_with_invalid_id(ec, rep);

					// assign a unique ClientId to that Client.
					msg.client_id(std::to_string(reinterpret_cast<std::size_t>(caller)));
				}
			}
			else if constexpr (std::is_same_v<response_type, mqtt::v5::connack>)
			{
				// and MUST return the Assigned Client Identifier in the CONNACK packet [MQTT-3.1.3-7].
				if (client_id.empty())
				{
					// assign a unique ClientId to that Client.
					msg.client_id(std::to_string(reinterpret_cast<std::size_t>(caller)));

					rep.properties().add(mqtt::v5::assigned_client_identifier(msg.client_id()));
				}
			}
			else
			{
				static_assert(detail::always_false_v<Response>);
			}

			// check whether uniquely identifier
		}

		// must be server
		template<class Message, class Response, bool IsClient = args_t::is_client>
		inline std::enable_if_t<!IsClient, void>
		_before_connect_callback(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			Message& msg, Response& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);

			using message_type  [[maybe_unused]] = typename detail::remove_cvref_t<Message>;
			using response_type [[maybe_unused]] = typename detail::remove_cvref_t<Response>;

			// if started already and recvd connect message again, disconnect
			state_t expected = state_t::started;
			if (caller->state_.compare_exchange_strong(expected, state_t::started))
			{
				ec = mqtt::make_error_code(mqtt::error::malformed_packet);
				rep.set_send_flag(false);
				return;
			}

			// mqtt auth
			if (caller->security().enabled())
			{
				std::optional<std::string> username;

				if (caller->get_preauthed_username())
				{
					if (caller->security().login_cert(caller->get_preauthed_username().value()))
					{
						username = caller->get_preauthed_username();
					}
				}
				else if (msg.has_username() && msg.has_password())
				{
					username = caller->security().login(msg.username(), msg.password());
				}
				else
				{
					username = caller->security().login_anonymous();
				}

				// If login fails, try the unauthenticated user
				if (!username)
				{
					username = caller->security().login_unauthenticated();
				}

				if (!username)
				{
					ASIO2_LOG_TRACE("User failed to login: {}",
						(msg.has_username() ? msg.username() : "anonymous user"));

					if (msg.has_username() && msg.has_password())
						return _set_connack_reason_code_with_bad_user_name_or_password(ec, rep);

					return _set_connack_reason_code_with_not_authorized(ec, rep);
				}
			}

			// check client id
			if (_check_connect_client_id(ec, caller, msg, rep); ec)
				return;

			// set keepalive timeout
			// If the Keep Alive value is non-zero and the Server does not receive a Control Packet from
			// the Client within one and a half times the Keep Alive time period, it MUST disconnect the
			// Network Connection to the Client as if the network had failed [MQTT-3.1.2-24].
			// A Keep Alive value of zero (0) has the effect of turning off the keep alive mechanism.
			// This means that, in this case, the Server is not required to disconnect the Client on the
			// grounds of inactivity. Note that a Server is permitted to disconnect a Client that it
			// determines to be inactive or non-responsive at any time, regardless of the Keep Alive value
			// provided by that Client.
			mqtt::two_byte_integer::value_type keepalive = msg.keep_alive();
			if (keepalive == 0)
			{
				caller->set_silence_timeout(std::chrono::milliseconds(detail::tcp_silence_timeout));
			}
			else
			{
				caller->set_silence_timeout(std::chrono::milliseconds(keepalive * 1500));
			}

			// fill the reason code with 0.
			rep.reason_code(0);

			caller->connect_message_ = msg;

			// If a client with the same Client ID is already connected to the server, the "older" client
			// must be disconnected by the server before completing the CONNECT flow of the new client.

			// If CleanSession is set to 0, the Server MUST resume communications with the Client based on state
			// from the current Session (as identified by the Client identifier).
			// If there is no Session associated with the Client identifier the Server MUST create a new Session.
			// The Client and Server MUST store the Session after the Client and Server are disconnected [MQTT-3.1.2-4].
			// After the disconnection of a Session that had CleanSession set to 0, the Server MUST store further
			// QoS 1 and QoS 2 messages that match any subscriptions that the client had at the time of disconnection
			// as part of the Session state [MQTT-3.1.2-5]. It MAY also store QoS 0 messages that meet the same criteria.
			// If CleanSession is set to 1, the Client and Server MUST discard any previous Session and start a new one.
			// This Session lasts as long as the Network Connection.State data associated with this Session MUST NOT be
			// reused in any subsequent Session[MQTT-3.1.2-6].

			// If a CONNECT packet is received with Clean Start is set to 1, the Client and Server MUST discard any
			// existing Session and start a new Session [MQTT-3.1.2-4]. Consequently, the Session Present flag in
			// CONNACK is always set to 0 if Clean Start is set to 1.
			// If a CONNECT packet is received with Clean Start set to 0 and there is a Session associated with the
			// Client Identifier, the Server MUST resume communications with the Client based on state from the existing
			// Session[MQTT-3.1.2-5].If a CONNECT packet is received with Clean Start set to 0 and there is no Session
			// associated with the Client Identifier, the Server MUST create a new Session[MQTT-3.1.2-6].

			std::shared_ptr<caller_t> session_ptr = caller->mqtt_sessions().find_mqtt_session(caller->client_id());

			// If the Server accepts a connection with Clean Start set to 1, the Server MUST set Session
			// Present to 0 in the CONNACK packet in addition to setting a 0x00 (Success) Reason Code in
			// the CONNACK packet [MQTT-3.2.2-2].
			if (msg.clean_session())
				rep.session_present(false);
			// If the Server accepts a connection with Clean Start set to 0 and the Server has Session 
			// State for the ClientID, it MUST set Session Present to 1 in the CONNACK packet, otherwise
			// it MUST set Session Present to 0 in the CONNACK packet. In both cases it MUST set a 0x00
			// (Success) Reason Code in the CONNACK packet [MQTT-3.2.2-3].
			else
				rep.session_present(session_ptr != nullptr);

			if (session_ptr == nullptr)
			{
				caller->mqtt_sessions().push_mqtt_session(caller->client_id(), caller_ptr);

				session_ptr = caller_ptr;
			}
			else
			{
				if (session_ptr->is_started())
				{
					// send will message
					if (!session_ptr->connect_message_.empty())
					{
						auto f = [caller_ptr, caller](auto& conn) mutable
						{
							if (!conn.has_will())
								return;

							// note : why v5 ?
							mqtt::v5::publish pub;
							pub.qos(conn.will_qos());
							pub.retain(conn.will_retain());
							pub.topic_name(conn.will_topic());
							pub.payload(conn.will_payload());

							caller->push_event(
							[caller_ptr, caller, pub = std::move(pub)]
							(event_queue_guard<caller_t> g) mutable
							{
								detail::ignore_unused(g);

								caller->_multicast_publish(caller_ptr, caller, std::move(pub), std::string{});
							});
						};

						if /**/ (std::holds_alternative<mqtt::v3::connect>(session_ptr->connect_message_.base()))
						{
							mqtt::v3::connect* p = session_ptr->connect_message_.template get_if<mqtt::v3::connect>();
							f(*p);
						}
						else if (std::holds_alternative<mqtt::v4::connect>(session_ptr->connect_message_.base()))
						{
							mqtt::v4::connect* p = session_ptr->connect_message_.template get_if<mqtt::v4::connect>();
							f(*p);
						}
						else if (std::holds_alternative<mqtt::v5::connect>(session_ptr->connect_message_.base()))
						{
							mqtt::v5::connect* p = session_ptr->connect_message_.template get_if<mqtt::v5::connect>();
							f(*p);
						}
					}

					// disconnect session
					// In previous versions, the mutex is a global variable, when code reached here,
					// the mutex status is locked already, so if we call session_ptr->stop()
					// directly, the stop maybe blocked forever, beacuse the session1 stop maybe
					// called in the session2 thread, and in the handle disconnect funcion of mqtt
					// session, the global mutex is required to lock again, so it caused deaklock,
					// to solve this problem, we can use session->post([](){session->stop()}).
					// but now, we change the mutex from global variable to member variable for each
					// mqtt class, so now, it won't caused deaklock, even we only use session->stop,
					// but we still use session->post([](){session->stop()}), to enhanced stability.
					session_ptr->post([session_ptr]() mutable { session_ptr->stop(); });

					// 
					bool clean_session = false;

					if /**/ (std::holds_alternative<mqtt::v3::connect>(session_ptr->connect_message_.base()))
					{
						clean_session = session_ptr->connect_message_.template get_if<mqtt::v3::connect>()->clean_session();
					}
					else if (std::holds_alternative<mqtt::v4::connect>(session_ptr->connect_message_.base()))
					{
						clean_session = session_ptr->connect_message_.template get_if<mqtt::v4::connect>()->clean_session();
					}
					else if (std::holds_alternative<mqtt::v5::connect>(session_ptr->connect_message_.base()))
					{
						clean_session = session_ptr->connect_message_.template get_if<mqtt::v5::connect>()->clean_session();
					}

					if (clean_session)
					{
					}
					else
					{
					}

					if (msg.clean_session())
					{

					}
					else
					{
						// copy session state from old session to new session
					}
				}
				else
				{

				}

				// replace old session to new session
				session_ptr = caller_ptr;
			}
		}

		template<class Message, class Response, bool IsClient = args_t::is_client>
		inline std::enable_if_t<IsClient, void>
		_before_connect_callback(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			Message& msg, Response& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);

			ASIO2_ASSERT(false && "client should't recv the connect message");
		}

		// must be server
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v3::connect& msg, mqtt::v3::connack& rep)
		{
			if (_before_connect_callback(ec, caller_ptr, caller, om, msg, rep); ec)
				return;
		}

		// must be server
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v4::connect& msg, mqtt::v4::connack& rep)
		{
			if (_before_connect_callback(ec, caller_ptr, caller, om, msg, rep); ec)
				return;
		}

		// must be server
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v5::connect& msg, mqtt::v5::connack& rep)
		{
			if (_before_connect_callback(ec, caller_ptr, caller, om, msg, rep); ec)
				return;
		}

		// must be server
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v5::connect& msg, mqtt::v5::auth& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);

			//if constexpr (caller_t::is_session())
			//{
			//	// if started already and recvd connect message again, disconnect
			//	state_t expected = state_t::started;
			//	if (caller->state_.compare_exchange_strong(expected, state_t::started))
			//	{
			//		ec = mqtt::make_error_code(mqtt::error::malformed_packet);
			//		rep.set_send_flag(false);
			//		return;
			//	}

			//	caller->connect_message_ = msg;

			//	std::shared_ptr<caller_t> session_ptr = caller->mqtt_sessions().find_mqtt_session(msg.client_id());
			//	if (session_ptr != nullptr)
			//	{

			//	}
			//}
			//else
			//{
			//	std::ignore = true;
			//}
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v3::connect& msg, mqtt::v3::connack& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);

			// if already has error, return
			if (ec)
				return;

			// If a client with the same Client ID is already connected to the server, the "older" client
			// must be disconnected by the server before completing the CONNECT flow of the new client.
			switch(rep.reason_code())
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
			mqtt::v4::connect& msg, mqtt::v4::connack& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);

			// if already has error, return
			if (ec)
				return;

			switch(rep.reason_code())
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
			mqtt::v5::connect& msg, mqtt::v5::connack& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);

			// if already has error, return
			if (ec)
				return;

			ec = mqtt::make_error_code(static_cast<mqtt::error>(rep.reason_code()));

			if (!ec)
			{
				if (rep.properties().has<mqtt::v5::topic_alias_maximum>() == false)
					rep.properties().add(mqtt::v5::topic_alias_maximum{ caller->topic_alias_maximum() });
			}
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v5::connect& msg, mqtt::v5::auth& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);

			// if already has error, return
			if (ec)
				return;

		}
	};
}

#endif // !__ASIO2_MQTT_AOP_CONNECT_HPP__
