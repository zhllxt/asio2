/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_MQTT_HANDLER_HPP__
#define __ASIO2_MQTT_HANDLER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

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
#include <asio2/mqtt/detail/mqtt_offline_message.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;

	template<class caller_t>
	class mqtt_handler_t
	{
		friend caller_t;

		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;

	public:
		using self = mqtt_handler_t<caller_t>;

	protected:
		//template<class... Args>
		//inline void _before_user_callback(Args&&...)
		//{
		//	std::ignore = true;
		//}
		//template<class... Args>
		//inline void _after_user_callback(Args&&...)
		//{
		//	std::ignore = true;
		//}

		// must be server
		template<class Message, class Response>
		inline bool _before_connect_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, Message& msg, Response& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);

			using message_type  [[maybe_unused]] = typename detail::remove_cvref_t<Message>;
			using response_type [[maybe_unused]] = typename detail::remove_cvref_t<Response>;

			// if started already and recvd connect message again, disconnect
			state_t expected = state_t::started;
			if (caller->state_.compare_exchange_strong(expected, state_t::started))
			{
				ec = mqtt::make_error_code(mqtt::error::malformed_packet);
				response.send_flag(false);
				return false;
			}

			// A Server MAY allow a Client to supply a ClientId that has a length of zero bytes, 
			// however if it does so the Server MUST treat this as a special case and assign a 
			// unique ClientId to that Client. It MUST then process the CONNECT packet as if the
			// Client had provided that unique ClientId [MQTT-3.1.3-6].
			// If the Client supplies a zero-byte ClientId, the Client MUST also set CleanSession 
			// to 1[MQTT-3.1.3-7].
			// If the Client supplies a zero-byte ClientId with CleanSession set to 0, the Server 
			// MUST respond to the CONNECT Packet with a CONNACK return code 0x02 (Identifier rejected) 
			// and then close the Network Connection[MQTT-3.1.3-8].
			// If the Server rejects the ClientId it MUST respond to the CONNECT Packet with a CONNACK
			// return code 0x02 (Identifier rejected) and then close the Network Connection[MQTT-3.1.3-9].
			if (msg.client_id().empty() && msg.clean_session() == false)
			{
				ec = mqtt::make_error_code(mqtt::error::client_identifier_not_valid);

				if constexpr /**/ (std::is_same_v<response_type, mqtt::v3::connack>)
				{
					response.reason_code(mqtt::v3::connect_reason_code::identifier_rejected);
				}
				else if constexpr (std::is_same_v<response_type, mqtt::v4::connack>)
				{
					response.reason_code(mqtt::v4::connect_reason_code::identifier_rejected);
				}
				else if constexpr (std::is_same_v<response_type, mqtt::v5::connack>)
				{
					response.reason_code(mqtt::error::client_identifier_not_valid);
				}
				else
				{
					ASIO2_ASSERT(false);
				}

				return false;
			}
			else
			{
				response.reason_code(0);
			}

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

			// assign a unique ClientId to that Client.
			if (msg.client_id().empty())
			{
				msg.client_id(std::to_string(reinterpret_cast<std::size_t>(caller)));
			}

			caller->connect_message_ = msg;

			asio2_unique_lock lock{ caller->mqttid_sessions_mtx_ };

			auto iter = caller->mqttid_sessions_.find(caller->client_id());

			// If the Server accepts a connection with Clean Start set to 1, the Server MUST set Session
			// Present to 0 in the CONNACK packet in addition to setting a 0x00 (Success) Reason Code in
			// the CONNACK packet [MQTT-3.2.2-2].
			if (msg.clean_session())
				response.session_present(false);
			// If the Server accepts a connection with Clean Start set to 0 and the Server has Session 
			// State for the ClientID, it MUST set Session Present to 1 in the CONNACK packet, otherwise
			// it MUST set Session Present to 0 in the CONNACK packet. In both cases it MUST set a 0x00
			// (Success) Reason Code in the CONNACK packet [MQTT-3.2.2-3].
			else
				response.session_present(iter != caller->mqttid_sessions_.end());

			if (iter == caller->mqttid_sessions_.end())
			{
				iter = caller->mqttid_sessions_.emplace(caller->client_id(), caller_ptr).first;
			}
			else
			{
				auto& session_ptr = iter->second;

				if (session_ptr->is_started())
				{
					// send will message
					std::visit([this, caller_ptr, caller](auto& conn) mutable
					{
						if (conn.will_flag())
						{
							mqtt::v5::publish pub;
							pub.qos(conn.will_qos());
							pub.retain(conn.will_retain());
							pub.topic_name(conn.will_topic());
							pub.payload(conn.will_payload());

							caller->push_event(
							[this, caller_ptr, caller, pub = std::move(pub)]
							(event_queue_guard<caller_t> g) mutable
							{
								detail::ignore_unused(g);

								this->_multicast_publish(caller_ptr, caller, std::move(pub), std::string{});
							});
						}
					}, session_ptr->connect_message_);

					// disconnect session
					session_ptr->stop();

					// 
					bool clean_session;

					std::visit([&clean_session](auto& conn)
					{
						clean_session = conn.clean_session();
					}, session_ptr->connect_message_);

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

			return true;
		}

		// must be server
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::connect& msg, mqtt::v3::connack& response)
		{
			if (!_before_connect_callback(ec, caller_ptr, caller, msg, response))
				return;
		}

		// must be server
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::connect& msg, mqtt::v4::connack& response)
		{
			if (!_before_connect_callback(ec, caller_ptr, caller, msg, response))
				return;
		}

		// must be server
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::connect& msg, mqtt::v5::connack& response)
		{
			if (!_before_connect_callback(ec, caller_ptr, caller, msg, response))
				return;
		}

		// must be server
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::connect& msg, mqtt::v5::auth& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);

			// if started already and recvd connect message again, disconnect
			state_t expected = state_t::started;
			if (caller->state_.compare_exchange_strong(expected, state_t::started))
			{
				ec = mqtt::make_error_code(mqtt::error::malformed_packet);
				response.send_flag(false);
				return;
			}

			caller->connect_message_ = msg;

			asio2_unique_lock lock{ caller->mqttid_sessions_mtx_ };

			auto iter = caller->mqttid_sessions_.find(msg.client_id());
			if (iter != caller->mqttid_sessions_.end())
			{

			}
		}

		// must be client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::connack& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);

			// if started already and recvd connack message again, disconnect
			state_t expected = state_t::started;
			if (caller->state_.compare_exchange_strong(expected, state_t::started))
			{
				ec = mqtt::make_error_code(mqtt::error::malformed_packet);
				return;
			}

			caller->connack_message_ = msg;

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
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::connack& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);

			// if started already and recvd connack message again, disconnect
			state_t expected = state_t::started;
			if (caller->state_.compare_exchange_strong(expected, state_t::started))
			{
				ec = mqtt::make_error_code(mqtt::error::malformed_packet);
				return;
			}

			caller->connack_message_ = msg;

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
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::connack& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);

			// if started already and recvd connack message again, disconnect
			state_t expected = state_t::started;
			if (caller->state_.compare_exchange_strong(expected, state_t::started))
			{
				ec = mqtt::make_error_code(mqtt::error::malformed_packet);
				return;
			}

			caller->connack_message_ = msg;

			ec = mqtt::make_error_code(static_cast<mqtt::error>(msg.reason_code()));
		}

		// server or client
		template<class Message, class Response>
		inline bool _before_publish_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, Message& msg, Response& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);

			using message_type  = typename detail::remove_cvref_t<Message>;
			using response_type = typename detail::remove_cvref_t<Response>;

			if (msg.has_packet_id())
			{
				response.packet_id(msg.packet_id());
			}

			mqtt::qos_type qos = msg.qos();

			// the qos 0 publish messgae don't need response
			if (qos == mqtt::qos_type::at_most_once)
				response.send_flag(false);

			if (detail::to_underlying(qos) > 2)
			{
				ec = mqtt::make_error_code(mqtt::error::malformed_packet);
				return false;
			}

			if (detail::to_underlying(qos) > caller->maximum_qos())
			{
				ec = mqtt::make_error_code(mqtt::error::qos_not_supported);
				return false;
			}

			if (msg.retain() && caller->retain_available() == false)
			{
				ec = mqtt::make_error_code(mqtt::error::retain_not_supported);
				return false;
			}

			// The Packet Identifier field is only present in PUBLISH Packets where the QoS level is 1 or 2.
			if (detail::to_underlying(qos) > 0 && msg.has_packet_id() == false)
			{
				ec = mqtt::make_error_code(mqtt::error::malformed_packet); // error code : broker.hivemq.com
				return false;
			}

			if (detail::to_underlying(qos) == 0 && msg.has_packet_id() == true)
			{
				ec = mqtt::make_error_code(mqtt::error::malformed_packet); // error code : broker.hivemq.com
				return false;
			}

			std::string_view topic_name = msg.topic_name();

			// must first determine whether topic_name is empty, beacuse v5::publish's topic_name maybe empty.
			if (!topic_name.empty())
			{
				if (mqtt::is_topic_name_valid(topic_name) == false)
				{
					ec = mqtt::make_error_code(mqtt::error::topic_name_invalid);
					return false;
				}
			}

			// << topic_alias >>
			// A Topic Alias of 0 is not permitted. A sender MUST NOT send a PUBLISH packet containing a Topic Alias
			// which has the value 0 [MQTT-3.3.2-8].
			// << topic_alias_maximum >>
			// This value indicates the highest value that the Client will accept as a Topic Alias sent by the Server.
			// The Client uses this value to limit the number of Topic Aliases that it is willing to hold on this Connection.
			// The Server MUST NOT send a Topic Alias in a PUBLISH packet to the Client greater than Topic Alias Maximum
			// [MQTT-3.1.2-26]. A value of 0 indicates that the Client does not accept any Topic Aliases on this connection.
			// If Topic Alias Maximum is absent or zero, the Server MUST NOT send any Topic Aliases to the Client [MQTT-3.1.2-27].
			if constexpr (std::is_same_v<message_type, mqtt::v5::publish>)
			{
				mqtt::v5::topic_alias* topic_alias = msg.properties().template get_if<mqtt::v5::topic_alias>();
				if (topic_alias)
				{
					auto alias_value = topic_alias->value();
					if (alias_value == 0 || alias_value > caller->topic_alias_maximum())
					{
						ec = mqtt::make_error_code(mqtt::error::malformed_packet);
						return false;
					}

					if (!topic_name.empty())
					{
						caller->push_topic_alias(alias_value, topic_name);
					}
					else
					{
						if (!caller->find_topic_alias(alias_value, topic_name))
						{
							ec = mqtt::make_error_code(mqtt::error::topic_alias_invalid);
							return false;
						}
					}
				}
			}
			else
			{
				std::ignore = true;
			}

			// All Topic Names and Topic Filters MUST be at least one character long [MQTT-4.7.3-1]
			if (topic_name.empty())
			{
				ec = mqtt::make_error_code(mqtt::error::topic_name_invalid);
				return false;
			}

			//// Potentially allow write access for bridge status, otherwise explicitly deny.
			//// rc = mosquitto_topic_matches_sub("$SYS/broker/connection/+/state", topic, &match);
			//if (topic_name.compare(0, 4, "$SYS") == 0)
			//{
			//	ec = mqtt::make_error_code(mqtt::error::topic_name_invalid);
			//	return false;
			//}

			// Only allow sub/unsub to shared subscriptions
			if (topic_name.compare(0, 6, "$share") == 0)
			{
				ec = mqtt::make_error_code(mqtt::error::topic_name_invalid);
				return false;
			}

			constexpr bool is_pubrec =
				std::is_same_v<response_type, mqtt::v3::pubrec> ||
				std::is_same_v<response_type, mqtt::v4::pubrec> ||
				std::is_same_v<response_type, mqtt::v5::pubrec>;

			// the client or session sent publish with qos 2 but don't recvd pubrec, and it sent publish
			// a later again, so we need sent pubrec directly and return directly
			if (msg.qos() == mqtt::qos_type::exactly_once && caller->exactly_once_processing(msg.packet_id()))
			{
				ASIO2_ASSERT(msg.has_packet_id());
				ASIO2_ASSERT(is_pubrec);

				// return true, then the pubrec will be sent directly
				return true;
			}

			if constexpr (is_pubrec)
			{
				ASIO2_ASSERT(msg.has_packet_id());
				ASIO2_ASSERT(msg.qos() == mqtt::qos_type::exactly_once);

				caller->exactly_once_start(msg.packet_id());
			}
			else
			{
				std::ignore = true;
			}

			if constexpr(caller_t::is_session())
			{
				// use push_event to ensure the publish message is sent to clients must after mqtt 
				// response is sent already.
				caller->push_event(
				[this, caller_ptr, caller, msg = std::move(msg), topic_name = std::string{ topic_name }]
				(event_queue_guard<caller_t> g) mutable
				{
					detail::ignore_unused(g);

					this->_multicast_publish(caller_ptr, caller, std::move(msg), std::move(topic_name));
				});
			}
			else
			{
				std::ignore = true;
			}

			return true;
		}

		template<class Message>
		inline void _multicast_publish(std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, Message&& msg, std::string topic_name)
		{
			detail::ignore_unused(caller_ptr, caller, msg);

			using message_type  = typename detail::remove_cvref_t<Message>;

			//                  share_name   topic_filter
			std::set<std::tuple<std::string_view, std::string_view>> sent;

			if (topic_name.empty())
				topic_name = msg.topic_name();

			ASIO2_ASSERT(!topic_name.empty());

			caller->subs_map_.modify(topic_name, [this, caller, &msg, &sent]
			(std::string_view key, mqtt::subscription_entry<caller_t>& entry) mutable
			{
				detail::ignore_unused(key);

				mqtt::subscription& sub = entry.sub;

				std::string_view share_name   = sub.share_name();
				std::string_view topic_filter = sub.topic_filter();

				if (share_name.empty())
				{
					// Non shared subscriptions

					// If NL (no local) subscription option is set and
					// publisher is the same as subscriber, then skip it.
					if (sub.no_local() && entry.session->hash_key() == caller->hash_key())
						return;

					// send message
					_send_publish_to_subscriber(entry.session, entry.sub, entry.props, msg);
				}
				else
				{
					// Shared subscriptions
					bool inserted;
					std::tie(std::ignore, inserted) = sent.emplace(share_name, topic_filter);
					if (inserted)
					{
						if (auto session = caller->shared_targets_.get_target(share_name, topic_filter))
						{
							_send_publish_to_subscriber(session, entry.sub, entry.props, msg);
						}
					}
				}
			});

			/*
			 * If the message is marked as being retained, then we
			 * keep it in case a new subscription is added that matches
			 * this topic.
			 *
			 * @note: The MQTT standard 3.3.1.3 RETAIN makes it clear that
			 *        retained messages are global based on the topic, and
			 *        are not scoped by the client id. So any client may
			 *        publish a retained message on any topic, and the most
			 *        recently published retained message on a particular
			 *        topic is the message that is stored on the server.
			 *
			 * @note: The standard doesn't make it clear that publishing
			 *        a message with zero length, but the retain flag not
			 *        set, does not result in any existing retained message
			 *        being removed. However, internet searching indicates
			 *        that most brokers have opted to keep retained messages
			 *        when receiving contents of zero bytes, unless the so
			 *        received message has the retain flag set, in which case
			 *        the retained message is removed.
			 */
			if (msg.retain())
			{
				if (msg.payload().empty())
				{
					caller->retained_messages_.erase(topic_name);
				}
				else
				{
					std::shared_ptr<asio::steady_timer> expiry_timer;

					if constexpr (std::is_same_v<message_type, mqtt::v5::publish>)
					{
						mqtt::v5::message_expiry_interval* mei =
							msg.properties().template get_if<mqtt::v5::message_expiry_interval>();
						if (mei)
						{
							expiry_timer = std::make_shared<asio::steady_timer>(
								caller->io().context(), std::chrono::seconds(mei->value()));
							expiry_timer->async_wait(
							[caller, topic_name, wp = std::weak_ptr<asio::steady_timer>(expiry_timer)]
							(error_code const& ec) mutable
							{
								if (auto sp = wp.lock())
								{
									if (!ec)
									{
										caller->retained_messages_.erase(topic_name);
									}
								}
							});
						}
					}
					else
					{
						std::ignore = true;
					}

					caller->retained_messages_.insert_or_assign(topic_name,
						mqtt::retained_entry{ msg, std::move(expiry_timer) });
				}
			}
		}

		template<class session_t, class Message>
		inline void _send_publish_to_subscriber(session_t* session, mqtt::subscription& sub, mqtt::v5::properties_set& props, Message& msg)
		{
			mqtt::version ver = session->version();

			if /**/ (ver == mqtt::version::v3)
			{
				_prepare_send_publish(session, sub, props, msg, mqtt::v3::publish{});
			}
			else if (ver == mqtt::version::v4)
			{
				_prepare_send_publish(session, sub, props, msg, mqtt::v4::publish{});
			}
			else if (ver == mqtt::version::v5)
			{
				_prepare_send_publish(session, sub, props, msg, mqtt::v5::publish{});
			}
		}

		template<class session_t, class Message, class Response>
		inline void _prepare_send_publish(session_t* session, mqtt::subscription& sub, mqtt::v5::properties_set& props, Message& msg, Response&& response)
		{
			using message_type  = typename detail::remove_cvref_t<Message>;
			using response_type = typename detail::remove_cvref_t<Response>;

			// dup
			response.dup(msg.dup());

			// qos
			response.qos((std::min)(msg.qos(), sub.qos()));

			// retaion

			// Bit 3 of the Subscription Options represents the Retain As Published option.
			// Retained messages sent when the subscription is established have the RETAIN flag set to 1.

			// If 1, Application Messages forwarded using this subscription keep the RETAIN
			// flag they were published with.
			if (sub.rap())
			{
				response.retain(msg.retain());
			}
			// If 0, Application Messages forwarded using this subscription have the RETAIN
			// flag set to 0.
			else
			{
				response.retain(false);
			}

			// topic, payload
			response.topic_name(msg.topic_name());
			response.payload(msg.payload());

			// properties
			if constexpr (std::is_same_v<response_type, mqtt::v5::publish>)
			{
				if constexpr (std::is_same_v<message_type, mqtt::v5::publish>)
				{
					response.properties() = msg.properties();
				}
				else
				{
					std::ignore = true;
				}

				props.for_each([&response](auto& prop) mutable
				{
					response.properties().erase(prop);
					response.properties().add(prop);
				});
			}
			else
			{
				std::ignore = true;
			}

			// prepare send
			if (session->is_started())
			{
				if (session->offline_messages_.empty())
				{
					auto pub_qos = response.qos();
					if (pub_qos == mqtt::qos_type::at_least_once || pub_qos == mqtt::qos_type::exactly_once)
					{
						if (auto pid = session->acquire_id())
						{
							// TODO: Probably this should be switched to async_publish?
							//       Given the async_client / sync_client seperation
							//       and the way they have different function names,
							//       it wouldn't be possible for broker.hpp to be
							//       used with some hypothetical "async_server" in the future.
							response.packet_id(pid);

							_do_send_publish(session, std::forward<Response>(response));
						}
						else
						{
							// no packet id available
							ASIO2_ASSERT(false);

							// offline_messages_ is not empty or packet_id_exhausted
							session->offline_messages_.push_back(session->io().context(),
								std::forward<Response>(response));
						}
					}
					else
					{
						// A PUBLISH Packet MUST NOT contain a Packet Identifier if its QoS value is set to 0
						ASIO2_ASSERT(response.has_packet_id() == false);

						_do_send_publish(session, std::forward<Response>(response));
					}
				}
				else
				{
					// send all offline messages first
					_send_all_offline_message(session);

					_do_send_publish(session, std::forward<Response>(response));
				}
			}
			else
			{
				session->offline_messages_.push_back(session->io().context(), std::forward<Response>(response));
			}
		}

		template<class session_t, class Response>
		inline void _do_send_publish(session_t* session, Response&& response)
		{
			session->push_event(
			[session, sptr = session->selfptr(), rep = std::forward<Response>(response)]
			(event_queue_guard<caller_t> g) mutable
			{
				detail::ignore_unused(sptr);

				session->_do_send(rep, [session, &rep, g = std::move(g)]
				(const error_code& ec, std::size_t) mutable
				{
					// send failed, add it to offline messages
					if (ec)
					{
						session->offline_messages_.push_back(session->io().context(), std::move(rep));
					}
				});
			});
		}

		template<class session_t>
		inline void _send_all_offline_message(session_t* session)
		{
			session->offline_messages_.for_each([this, session](mqtt::offline_entry& entry) mutable
			{
				std::visit([this, session](auto&& pub) mutable
				{
					this->_do_send_publish(session, std::move(pub));
				}, entry.message);
			});

			session->offline_messages_.clear();
		}

		// server or client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::publish& msg, mqtt::v3::puback& response)
		{
			if (!_before_publish_callback(ec, caller_ptr, caller, msg, response))
				return;
		}

		// server or client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::publish& msg, mqtt::v4::puback& response)
		{
			if (!_before_publish_callback(ec, caller_ptr, caller, msg, response))
				return;
		}

		// server or client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::publish& msg, mqtt::v5::puback& response)
		{
			if (!_before_publish_callback(ec, caller_ptr, caller, msg, response))
				return;
		}

		// server or client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::publish& msg, mqtt::v3::pubrec& response)
		{
			if (!_before_publish_callback(ec, caller_ptr, caller, msg, response))
				return;
		}

		// server or client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::publish& msg, mqtt::v4::pubrec& response)
		{
			if (!_before_publish_callback(ec, caller_ptr, caller, msg, response))
				return;
		}

		// server or client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::publish& msg, mqtt::v5::pubrec& response)
		{
			if (!_before_publish_callback(ec, caller_ptr, caller, msg, response))
				return;
		}

		// server or client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::puback& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		// server or client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::puback& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		// server or client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::puback& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		// server or client
		template<class Message, class Response>
		inline bool _before_pubrec_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, Message& msg, Response& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);

			using message_type  [[maybe_unused]] = typename detail::remove_cvref_t<Message>;
			using response_type [[maybe_unused]] = typename detail::remove_cvref_t<Response>;

			response.packet_id(msg.packet_id());

			// PUBREL Reason Code
			// https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901144
			if constexpr (std::is_same_v<response_type, mqtt::v5::pubrel>)
			{
				response.reason_code(detail::to_underlying(mqtt::error::success));
			}
			else
			{
				std::ignore = true;
			}

			return true;
		}

		// server or client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::pubrec& msg, asio2::mqtt::v3::pubrel& response)
		{
			if (!_before_pubrec_callback(ec, caller_ptr, caller, msg, response))
				return;
		}

		// server or client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::pubrec& msg, asio2::mqtt::v4::pubrel& response)
		{
			if (!_before_pubrec_callback(ec, caller_ptr, caller, msg, response))
				return;
		}

		// server or client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::pubrec& msg, asio2::mqtt::v5::pubrel& response)
		{
			if (!_before_pubrec_callback(ec, caller_ptr, caller, msg, response))
				return;
		}

		// server or client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::pubrel& msg, asio2::mqtt::v3::pubcomp& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);

			response.packet_id(msg.packet_id());
		}

		// server or client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::pubrel& msg, asio2::mqtt::v4::pubcomp& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);

			response.packet_id(msg.packet_id());
		}

		// server or client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::pubrel& msg, asio2::mqtt::v5::pubcomp& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);

			response.packet_id(msg.packet_id());
		}

		// server or client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::pubcomp& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		// server or client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::pubcomp& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		// server or client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::pubcomp& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);

		}

		// must be server
		template<class Message, class Response>
		inline bool _before_subscribe_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, Message& msg, Response& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);

			using message_type = typename detail::remove_cvref_t<Message>;

			bool flag = true;

			constexpr bool is_v5 = std::is_same_v<message_type, mqtt::v5::subscribe>;

			response.packet_id(msg.packet_id());

			// subscription properties
			mqtt::v5::properties_set props;

			if constexpr (std::is_same_v<message_type, mqtt::v5::subscribe>)
			{
				props = msg.properties();

				// Get subscription identifier
				mqtt::v5::subscription_identifier* sub_id =
					msg.properties().template get_if<mqtt::v5::subscription_identifier>();
				if (sub_id)
				{
					// The Subscription Identifier can have the value of 1 to 268,435,455.
					// It is a Protocol Error if the Subscription Identifier has a value of 0
					auto v = sub_id->value();
					if (v < 1 || v > 268435455)
					{
						ASIO2_ASSERT(false);
						ec = mqtt::make_error_code(mqtt::error::protocol_error);
						flag = false;
					}
					else
					{
						std::ignore = true;
					}
				}
			}
			else
			{
				std::ignore = true;
			}

			for (mqtt::subscription& sub : msg.subscriptions().value())
			{
				// Reason Codes
				// https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901178

				// error, the session will be disconnect a later
				if (sub.topic_filter().empty())
				{
					ec = mqtt::make_error_code(mqtt::error::topic_filter_invalid);
					response.add_reason_codes(detail::to_underlying(is_v5 ?
						mqtt::error::topic_filter_invalid : mqtt::error::unspecified_error));
					flag = false;
					continue;
				}

				mqtt::qos_type qos = sub.qos();

				// error, the session will be disconnect a later
				if (detail::to_underlying(qos) > 2)
				{
					ec = mqtt::make_error_code(mqtt::error::unspecified_error);
					response.add_reason_codes(detail::to_underlying(mqtt::error::unspecified_error));
					flag = false;
					continue;
				}

				// not error, but not supported, and the session will not be disconnect
				if (detail::to_underlying(qos) > caller->maximum_qos())
				{
					ec = mqtt::make_error_code(mqtt::error::qos_not_supported);
					response.add_reason_codes(detail::to_underlying(is_v5 ?
						mqtt::error::quota_exceeded : mqtt::error::unspecified_error));
					flag = false;
					continue;
				}

				// not error, and supported too
				response.add_reason_codes(detail::to_underlying(qos));

				mqtt::subscription_entry<caller_t> entry{ caller, sub, std::move(props) };

				std::string_view share_name   = entry.share_name();
				std::string_view topic_filter = entry.topic_filter();

				if (!share_name.empty())
				{
					caller->shared_targets_.insert(caller, share_name, topic_filter);
				}

				auto[handle, insert] = caller->subs_map_.insert_or_assign(
					topic_filter, caller->client_id(), std::move(entry));

				mqtt::retain_handling_type rh = sub.retain_handling();

				if /**/ (rh == mqtt::retain_handling_type::send)
				{
					_send_retained_messages(caller_ptr, caller, sub);
				}
				else if (rh == mqtt::retain_handling_type::send_only_new_subscription)
				{
					if (insert)
					{
						_send_retained_messages(caller_ptr, caller, sub);
					}
				}
			}

			return flag;
		}

		inline void _send_retained_messages(std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::subscription& sub)
		{
			detail::ignore_unused(caller_ptr, caller, sub);

			// use push_event to ensure the publish message is sent to clients must after mqtt 
			// response is sent already.
			caller->push_event(
			[this, caller_ptr, caller, sub = std::move(sub), topic_filter = std::string{ sub.topic_filter() }]
			(event_queue_guard<caller_t> g) mutable
			{
				detail::ignore_unused(g);

				mqtt::v5::properties_set props;

				caller->retained_messages_.find(topic_filter, [this, caller, &sub, &props]
				(mqtt::retained_entry& entry) mutable
				{
					std::visit([this, caller, &sub, &props](auto& pub) mutable
					{
						this->_send_publish_to_subscriber(caller, sub, props, pub);
					}, entry.message);
				});
			});
		}

		// must be server
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::subscribe& msg, mqtt::v3::suback& response)
		{
			if (!_before_subscribe_callback(ec, caller_ptr, caller, msg, response))
				return;


		}

		// must be server
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::subscribe& msg, mqtt::v4::suback& response)
		{
			if (!_before_subscribe_callback(ec, caller_ptr, caller, msg, response))
				return;
		}

		// must be server
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::subscribe& msg, mqtt::v5::suback& response)
		{
			if (!_before_subscribe_callback(ec, caller_ptr, caller, msg, response))
				return;
		}

		// must be client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::suback& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		// must be client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::suback& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		// must be client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::suback& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		// must be server
		template<class Message, class Response>
		inline bool _before_unsubscribe_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, Message& msg, Response& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);

			using message_type  [[maybe_unused]] = typename detail::remove_cvref_t<Message>;
			using response_type [[maybe_unused]] = typename detail::remove_cvref_t<Response>;

			response.packet_id(msg.packet_id());

			mqtt::utf8_string_set& topic_filters = msg.topic_filters();

			topic_filters.for_each([&](/*mqtt::utf8_string*/auto& str)
			{
				auto[share_name, topic_filter] = mqtt::parse_topic_filter(str.data_view());

				if (!share_name.empty())
				{
					caller->shared_targets_.erase(caller, share_name, topic_filter);
				}

				std::size_t removed = 0;

				auto handle = caller->subs_map_.lookup(topic_filter);
				if (handle)
				{
					//handles_.erase(handle.value());
					removed = caller->subs_map_.erase(handle.value(), caller->client_id());
				}

				// The Payload contains a list of Reason Codes.
				// https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901194
				if constexpr (std::is_same_v<response_type, mqtt::v5::unsuback>)
				{
					if /**/ (topic_filter.empty())
						response.add_reason_codes(detail::to_underlying(mqtt::error::topic_filter_invalid));
					else if (removed == 0)
						response.add_reason_codes(detail::to_underlying(mqtt::error::no_subscription_existed));
					else
						response.add_reason_codes(detail::to_underlying(mqtt::error::success));
				}
				else
				{
					std::ignore = removed;
				}
			});

			return true;
		}

		// must be server
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::unsubscribe& msg, mqtt::v3::unsuback& response)
		{
			if (!_before_unsubscribe_callback(ec, caller_ptr, caller, msg, response))
				return;
		}

		// must be server
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::unsubscribe& msg, mqtt::v4::unsuback& response)
		{
			if (!_before_unsubscribe_callback(ec, caller_ptr, caller, msg, response))
				return;
		}

		// must be server
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::unsubscribe& msg, mqtt::v5::unsuback& response)
		{
			if (!_before_unsubscribe_callback(ec, caller_ptr, caller, msg, response))
				return;
		}

		// must be client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::unsuback& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		// must be client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::unsuback& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		// must be client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::unsuback& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		// must be server
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::pingreq& msg, mqtt::v3::pingresp& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}

		// must be server
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::pingreq& msg, mqtt::v4::pingresp& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}

		// must be server
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::pingreq& msg, mqtt::v5::pingresp& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}

		// must be client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::pingresp& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		// must be client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::pingresp& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		// must be client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::pingresp& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		// server or client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::disconnect& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);

			if constexpr /**/ (caller_t::is_session())
			{
				detail::ignore_unused(ec, caller_ptr, caller, msg);
			}
			else if constexpr (caller_t::is_client())
			{
				detail::ignore_unused(ec, caller_ptr, caller, msg);
			}
		}

		// server or client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::disconnect& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);

			if constexpr /**/ (caller_t::is_session())
			{
				detail::ignore_unused(ec, caller_ptr, caller, msg);
			}
			else if constexpr (caller_t::is_client())
			{
				detail::ignore_unused(ec, caller_ptr, caller, msg);
			}
		}

		// server or client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::disconnect& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);

			if constexpr /**/ (caller_t::is_session())
			{
				detail::ignore_unused(ec, caller_ptr, caller, msg);
			}
			else if constexpr (caller_t::is_client())
			{
				detail::ignore_unused(ec, caller_ptr, caller, msg);
			}
		}

		// must be server
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::auth& msg, mqtt::v5::connack& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}

		// server or client
		inline void _before_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::auth& msg, mqtt::v5::auth& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::connect& msg, mqtt::v3::connack& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
			// If a client with the same Client ID is already connected to the server, the "older" client
			// must be disconnected by the server before completing the CONNECT flow of the new client.
			switch(response.reason_code())
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

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::connect& msg, mqtt::v4::connack& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
			switch(response.reason_code())
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

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::connect& msg, mqtt::v5::connack& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);

			ec = mqtt::make_error_code(static_cast<mqtt::error>(response.reason_code()));

			if (!ec)
			{
				if (response.properties().has<mqtt::v5::topic_alias_maximum>() == false)
					response.properties().add(mqtt::v5::topic_alias_maximum{ caller->topic_alias_maximum() });
			}
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::connect& msg, mqtt::v5::auth& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::connack& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
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

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::connack& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
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

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::connack& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
			ec = mqtt::make_error_code(static_cast<mqtt::error>(msg.reason_code()));
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::publish& msg, mqtt::v3::puback& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::publish& msg, mqtt::v4::puback& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::publish& msg, mqtt::v5::puback& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::publish& msg, mqtt::v3::pubrec& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::publish& msg, mqtt::v4::pubrec& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::publish& msg, mqtt::v5::pubrec& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::puback& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::puback& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::puback& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::pubrec& msg, asio2::mqtt::v3::pubrel& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::pubrec& msg, asio2::mqtt::v4::pubrel& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::pubrec& msg, asio2::mqtt::v5::pubrel& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::pubrel& msg, asio2::mqtt::v3::pubcomp& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::pubrel& msg, asio2::mqtt::v4::pubcomp& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::pubrel& msg, asio2::mqtt::v5::pubcomp& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::pubcomp& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);

		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::pubcomp& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::pubcomp& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::subscribe& msg, mqtt::v3::suback& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::subscribe& msg, mqtt::v4::suback& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::subscribe& msg, mqtt::v5::suback& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::suback& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::suback& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::suback& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::unsubscribe& msg, mqtt::v3::unsuback& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::unsubscribe& msg, mqtt::v4::unsuback& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::unsubscribe& msg, mqtt::v5::unsuback& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);

		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::unsuback& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);

		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::unsuback& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);

		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::unsuback& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::pingreq& msg, mqtt::v3::pingresp& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::pingreq& msg, mqtt::v4::pingresp& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::pingreq& msg, mqtt::v5::pingresp& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::pingresp& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::pingresp& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::pingresp& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::disconnect& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::disconnect& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::disconnect& msg)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::auth& msg, mqtt::v5::connack& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}

		inline void _after_user_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::auth& msg, mqtt::v5::auth& response)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, response);
		}
	};
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_MQTT_HANDLER_HPP__
