/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_MQTT_AOP_PUBLISH_HPP__
#define __ASIO2_MQTT_AOP_PUBLISH_HPP__

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
	class mqtt_aop_publish
	{
		friend caller_t;

	protected:
		template<class Message>
		inline void _do_check_publish_topic_alias(
			error_code& ec, caller_t* caller, Message& msg, std::string& topic_name)
		{
			// << topic_alias >>
			// A Topic Alias of 0 is not permitted. A sender MUST NOT send a PUBLISH packet containing a Topic Alias
			// which has the value 0 [MQTT-3.3.2-8].
			// << topic_alias_maximum >>
			// This value indicates the highest value that the Client will accept as a Topic Alias sent by the Server.
			// The Client uses this value to limit the number of Topic Aliases that it is willing to hold on this Connection.
			// The Server MUST NOT send a Topic Alias in a PUBLISH packet to the Client greater than Topic Alias Maximum
			// [MQTT-3.1.2-26]. A value of 0 indicates that the Client does not accept any Topic Aliases on this connection.
			// If Topic Alias Maximum is absent or zero, the Server MUST NOT send any Topic Aliases to the Client [MQTT-3.1.2-27].
			mqtt::v5::topic_alias* topic_alias = msg.properties().template get_if<mqtt::v5::topic_alias>();
			if (topic_alias)
			{
				auto alias_value = topic_alias->value();
				if (alias_value == 0 || alias_value > caller->topic_alias_maximum())
				{
					ec = mqtt::make_error_code(mqtt::error::malformed_packet);
					return;
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
						return;
					}
				}
			}
		}

		template<class Message>
		inline void _check_publish_topic_alias(
			error_code& ec, caller_t* caller, Message& msg, std::string& topic_name)
		{
			using message_type = typename detail::remove_cvref_t<Message>;

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
				_do_check_publish_topic_alias(ec, caller, msg, topic_name);
			}
			else
			{
				detail::ignore_unused(ec, caller, msg, topic_name);
			}
		}

		// server or client
		template<class Message, class Response>
		inline void _before_publish_callback(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			Message& msg, Response& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);

			using message_type  [[maybe_unused]] = typename detail::remove_cvref_t<Message>;
			using response_type [[maybe_unused]] = typename detail::remove_cvref_t<Response>;

			// A PUBACK, PUBREC , PUBREL, or PUBCOMP packet MUST contain the same Packet Identifier
			// as the PUBLISH packet that was originally sent [MQTT-2.2.1-5]. 
			if (msg.has_packet_id())
			{
				rep.packet_id(msg.packet_id());
			}

			mqtt::qos_type qos = msg.qos();

			// the qos 0 publish messgae don't need response
			if (qos == mqtt::qos_type::at_most_once)
				rep.set_send_flag(false);

			if (!mqtt::is_valid_qos(qos))
			{
				ec = mqtt::make_error_code(mqtt::error::malformed_packet);
				return;
			}

			if (detail::to_underlying(qos) > caller->maximum_qos())
			{
				ec = mqtt::make_error_code(mqtt::error::qos_not_supported);
				return;
			}

			if (msg.retain() && caller->retain_available() == false)
			{
				ec = mqtt::make_error_code(mqtt::error::retain_not_supported);
				return;
			}

			// The Packet Identifier field is only present in PUBLISH Packets where the QoS level is 1 or 2.
			if (detail::to_underlying(qos) > 0 && msg.has_packet_id() == false)
			{
				ec = mqtt::make_error_code(mqtt::error::malformed_packet); // error code : broker.hivemq.com
				return;
			}

			if (detail::to_underlying(qos) == 0 && msg.has_packet_id() == true)
			{
				ec = mqtt::make_error_code(mqtt::error::malformed_packet); // error code : broker.hivemq.com
				return;
			}

			std::string topic_name{ msg.topic_name() };

			// must first determine whether topic_name is empty, beacuse v5::publish's topic_name maybe empty.
			if (!topic_name.empty())
			{
				if (mqtt::is_topic_name_valid(topic_name) == false)
				{
					ec = mqtt::make_error_code(mqtt::error::topic_name_invalid);
					return;
				}
			}

			if (_check_publish_topic_alias(ec, caller, msg, topic_name); ec)
				return;

			// All Topic Names and Topic Filters MUST be at least one character long [MQTT-4.7.3-1]
			if (topic_name.empty())
			{
				ec = mqtt::make_error_code(mqtt::error::topic_name_invalid);
				return;
			}

			//// Potentially allow write access for bridge status, otherwise explicitly deny.
			//// rc = mosquitto_topic_matches_sub("$SYS/broker/connection/+/state", topic, std::addressof(match));
			//if (topic_name.compare(0, 4, "$SYS") == 0)
			//{
			//	ec = mqtt::make_error_code(mqtt::error::topic_name_invalid);
			//	return;
			//}

			// Only allow sub/unsub to shared subscriptions
			if (topic_name.compare(0, 6, "$share") == 0)
			{
				ec = mqtt::make_error_code(mqtt::error::topic_name_invalid);
				return;
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

				// return, then the pubrec will be sent directly
				return;
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

			_multicast_publish(caller_ptr, caller, msg, std::move(topic_name));
		}

		template<class Message, bool IsClient = args_t::is_client>
		inline std::enable_if_t<!IsClient, void>
		_multicast_publish(
			std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, Message&& msg, std::string topic_name)
		{
			using message_type [[maybe_unused]] = typename detail::remove_cvref_t<Message>;

			// use post and push_event to ensure the publish message is sent to clients must
			// after mqtt response is sent already.
			asio::post(caller->io_->context(), make_allocator(caller->wallocator(),
			[this, caller, caller_ptr, msg = std::move(msg), topic_name = std::move(topic_name)]
			() mutable
			{
				caller->push_event(
				[this, caller, caller_ptr = std::move(caller_ptr), msg = std::move(msg),
					topic_name = std::move(topic_name)]
				(event_queue_guard<caller_t> g) mutable
				{
					detail::ignore_unused(g);

					this->_do_multicast_publish(caller_ptr, caller, std::move(msg), std::move(topic_name));
				});
			}));
		}

		template<class Message, bool IsClient = args_t::is_client>
		inline std::enable_if_t<IsClient, void>
		_multicast_publish(
			std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, Message&& msg, std::string topic_name)
		{
			detail::ignore_unused(caller_ptr, caller, msg, topic_name);
		}

		template<class Message>
		inline void _do_multicast_publish(
			std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, Message&& msg, std::string topic_name)
		{
			detail::ignore_unused(caller_ptr, caller, msg);

			using message_type [[maybe_unused]] = typename detail::remove_cvref_t<Message>;

			//                  share_name   topic_filter
			std::set<std::tuple<std::string, std::string>> sent;

			if (topic_name.empty())
				topic_name = msg.topic_name();

			ASIO2_ASSERT(!topic_name.empty());

			caller->subs_map().match(topic_name,
			[this, caller, &msg, &sent](std::string_view key, auto& node) mutable
			{
				detail::ignore_unused(this, key);

				mqtt::subscription& sub = node.sub;

				std::string_view share_name   = sub.share_name();
				std::string_view topic_filter = sub.topic_filter();

				if (share_name.empty())
				{
					// Non shared subscriptions

					auto session_ptr = node.caller.lock();

					if (!session_ptr)
						return;

					// If NL (no local) subscription option is set and
					// publisher is the same as subscriber, then skip it.
					if (sub.no_local() && session_ptr->hash_key() == caller->hash_key())
						return;

					// send message
					_send_publish_to_subscriber(std::move(session_ptr), node.sub, node.props, msg);
				}
				else
				{
					// Shared subscriptions
					bool inserted;
					std::tie(std::ignore, inserted) = sent.emplace(share_name, topic_filter);
					if (inserted)
					{
						auto session_ptr = caller->shared_targets().get_target(share_name, topic_filter);
						if (session_ptr)
						{
							_send_publish_to_subscriber(std::move(session_ptr), node.sub, node.props, msg);
						}
					}
				}
			});

			if (msg.retain())
			{
				_do_retain_publish(caller_ptr, caller, msg, topic_name);
			}
		}

		template<class Message>
		inline void _do_retain_publish(
			std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, Message&& msg, std::string topic_name)
		{
			detail::ignore_unused(caller_ptr, caller, msg);

			using message_type  = typename detail::remove_cvref_t<Message>;

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
			if (msg.payload().empty())
			{
				caller->retained_messages().erase(topic_name);
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
							caller->io_->context(), std::chrono::seconds(mei->value()));
						expiry_timer->async_wait(
						[caller, topic_name, wp = std::weak_ptr<asio::steady_timer>(expiry_timer)]
						(error_code const& ec) mutable
						{
							if (auto sp = wp.lock())
							{
								if (!ec)
								{
									caller->retained_messages().erase(topic_name);
								}
							}
						});
					}
				}
				else
				{
					std::ignore = true;
				}

				caller->retained_messages().insert_or_assign(topic_name,
					mqtt::rmnode{ msg, std::move(expiry_timer) });
			}
		}

		template<class session_t, class Message>
		inline void _send_publish_to_subscriber(
			std::shared_ptr<session_t> session, mqtt::subscription& sub, mqtt::v5::properties_set& props,
			Message& msg)
		{
			if (!session)
				return;

			mqtt::version ver = session->version();

			// https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901026
			// The Client and Server assign Packet Identifiers independently of each other. 

			if /**/ (ver == mqtt::version::v3)
			{
				_prepare_send_publish(std::move(session), sub, props, msg, mqtt::v3::publish{});
			}
			else if (ver == mqtt::version::v4)
			{
				_prepare_send_publish(std::move(session), sub, props, msg, mqtt::v4::publish{});
			}
			else if (ver == mqtt::version::v5)
			{
				_prepare_send_publish(std::move(session), sub, props, msg, mqtt::v5::publish{});
			}
		}

		template<class session_t, class Message, class Response>
		inline void _prepare_send_publish(
			std::shared_ptr<session_t> session, mqtt::subscription& sub, mqtt::v5::properties_set& props,
			Message& msg, Response&& rep)
		{
			using message_type  = typename detail::remove_cvref_t<Message>;
			using response_type = typename detail::remove_cvref_t<Response>;

			// dup
			rep.dup(msg.dup());

			// qos
			rep.qos((std::min)(msg.qos(), sub.qos()));

			// retaion

			// Bit 3 of the Subscription Options represents the Retain As Published option.
			// Retained messages sent when the subscription is established have the RETAIN flag set to 1.

			// If 1, Application Messages forwarded using this subscription keep the RETAIN
			// flag they were published with.
			if (sub.rap())
			{
				rep.retain(msg.retain());
			}
			// If 0, Application Messages forwarded using this subscription have the RETAIN
			// flag set to 0.
			else
			{
				rep.retain(false);
			}

			// topic, payload
			rep.topic_name(msg.topic_name());
			rep.payload(msg.payload());

			// properties
			if constexpr (std::is_same_v<response_type, mqtt::v5::publish>)
			{
				if constexpr (std::is_same_v<message_type, mqtt::v5::publish>)
				{
					rep.properties() = msg.properties();
				}
				else
				{
					std::ignore = true;
				}

				props.for_each([&rep](auto& prop) mutable
				{
					rep.properties().erase(prop);
					rep.properties().add(prop);
				});
			}
			else
			{
				std::ignore = true;
			}

			// prepare send
			session_t* p = session.get();
			p->dispatch([this, session = std::move(session), rep = std::forward<Response>(rep)]() mutable
			{
				this->_check_send_publish(std::move(session), std::move(rep));
			});
		}

		template<class session_t, class Response>
		inline void _check_send_publish(std::shared_ptr<session_t> session, Response&& rep)
		{
			using response_type = typename detail::remove_cvref_t<Response>;

			if (session->is_started())
			{
				if (session->offline_messages_.empty())
				{
					auto pub_qos = rep.qos();
					if (pub_qos == mqtt::qos_type::at_least_once || pub_qos == mqtt::qos_type::exactly_once)
					{
						if (auto pid = session->idmgr_.get())
						{
							// TODO: Probably this should be switched to async_publish?
							//       Given the async_client / sync_client seperation
							//       and the way they have different function names,
							//       it wouldn't be possible for broker.hpp to be
							//       used with some hypothetical "async_server" in the future.
							rep.packet_id(pid);

							_do_send_publish(session, std::forward<Response>(rep));
						}
						else
						{
							// no packet id available
							ASIO2_ASSERT(false);

							// offline_messages_ is not empty or packet_id_exhausted
							session->offline_messages_.push_back(session->io_->context(),
								std::forward<Response>(rep));
						}
					}
					else
					{
						// A PUBLISH Packet MUST NOT contain a Packet Identifier if its QoS value is set to 0
						ASIO2_ASSERT(rep.has_packet_id() == false);

						_do_send_publish(session, std::forward<Response>(rep));
					}
				}
				else
				{
					// send all offline messages first
					_send_all_offline_message(session);

					_do_send_publish(session, std::forward<Response>(rep));
				}
			}
			else
			{
				session->offline_messages_.push_back(session->io_->context(), std::forward<Response>(rep));
			}
		}

		template<class session_t, class Response>
		inline void _do_send_publish(std::shared_ptr<session_t> session, Response&& rep)
		{
			session->push_event([session, id = session->life_id(), rep = std::forward<Response>(rep)]
			(event_queue_guard<caller_t> g) mutable
			{
				if (id != session->life_id())
				{
					set_last_error(asio::error::operation_aborted);
					return;
				}

				session->_do_send(rep, [session, &rep, g = std::move(g)]
				(const error_code& ec, std::size_t) mutable
				{
					// send failed, add it to offline messages
					if (ec)
					{
						session->offline_messages_.push_back(session->io_->context(), std::move(rep));
					}
				});
			});
		}

		template<class session_t>
		inline void _send_all_offline_message(std::shared_ptr<session_t> session)
		{
			session->offline_messages_.for_each([this, session](mqtt::omnode& node) mutable
			{
				std::visit([this, session](auto&& pub) mutable
				{
					this->_do_send_publish(session, std::move(pub));
				}, node.message.base());
			});

			session->offline_messages_.clear();
		}

		// server or client
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v3::publish& msg, mqtt::v3::puback& rep)
		{
			if (_before_publish_callback(ec, caller_ptr, caller, om, msg, rep); ec)
				return;
		}

		// server or client
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v4::publish& msg, mqtt::v4::puback& rep)
		{
			if (_before_publish_callback(ec, caller_ptr, caller, om, msg, rep); ec)
				return;
		}

		// server or client
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v5::publish& msg, mqtt::v5::puback& rep)
		{
			if (_before_publish_callback(ec, caller_ptr, caller, om, msg, rep); ec)
				return;
		}

		// server or client
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v3::publish& msg, mqtt::v3::pubrec& rep)
		{
			if (_before_publish_callback(ec, caller_ptr, caller, om, msg, rep); ec)
				return;
		}

		// server or client
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v4::publish& msg, mqtt::v4::pubrec& rep)
		{
			if (_before_publish_callback(ec, caller_ptr, caller, om, msg, rep); ec)
				return;
		}

		// server or client
		inline void _before_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v5::publish& msg, mqtt::v5::pubrec& rep)
		{
			if (_before_publish_callback(ec, caller_ptr, caller, om, msg, rep); ec)
				return;
		}

		template<class Message, class Response, bool IsClient = args_t::is_client>
		inline std::enable_if_t<!IsClient, void>
		_do_publish_router(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			Message& msg, Response& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);
		}

		template<class Message, class Response, bool IsClient = args_t::is_client>
		inline std::enable_if_t<IsClient, void>
		_do_publish_router(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			Message& msg, Response& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);

			using message_type  [[maybe_unused]] = typename detail::remove_cvref_t<Message>;
			using response_type [[maybe_unused]] = typename detail::remove_cvref_t<Response>;

			std::string_view topic_name = msg.topic_name();

			// client don't need lock
			caller->subs_map().match(topic_name, [this, caller, &om](std::string_view key, auto& node) mutable
			{
				detail::ignore_unused(this, caller, key);

				mqtt::subscription& sub = node.sub;

				[[maybe_unused]] std::string_view share_name   = sub.share_name();
				[[maybe_unused]] std::string_view topic_filter = sub.topic_filter();

				if (share_name.empty())
				{
					if (node.callback)
						node.callback(om);
				}
				else
				{
				}
			});
		}

		// server or client
		template<class Message, class Response>
		inline void _after_publish_callback(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			Message& msg, Response& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, om, msg, rep);

			using message_type  [[maybe_unused]] = typename detail::remove_cvref_t<Message>;
			using response_type [[maybe_unused]] = typename detail::remove_cvref_t<Response>;

			[[maybe_unused]] std::string topic_name{ msg.topic_name() };

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
					caller->find_topic_alias(topic_alias->value(), topic_name);
				}
			}
			else
			{
				std::ignore = true;
			}

			_do_publish_router(ec, caller_ptr, caller, om, msg, rep);
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v3::publish& msg, mqtt::v3::puback& rep)
		{
			if (_after_publish_callback(ec, caller_ptr, caller, om, msg, rep); ec)
				return;
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v4::publish& msg, mqtt::v4::puback& rep)
		{
			if (_after_publish_callback(ec, caller_ptr, caller, om, msg, rep); ec)
				return;
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v5::publish& msg, mqtt::v5::puback& rep)
		{
			if (_after_publish_callback(ec, caller_ptr, caller, om, msg, rep); ec)
				return;
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v3::publish& msg, mqtt::v3::pubrec& rep)
		{
			if (_after_publish_callback(ec, caller_ptr, caller, om, msg, rep); ec)
				return;
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v4::publish& msg, mqtt::v4::pubrec& rep)
		{
			if (_after_publish_callback(ec, caller_ptr, caller, om, msg, rep); ec)
				return;
		}

		inline void _after_user_callback_impl(
			error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::message& om,
			mqtt::v5::publish& msg, mqtt::v5::pubrec& rep)
		{
			if (_after_publish_callback(ec, caller_ptr, caller, om, msg, rep); ec)
				return;
		}
	};
}

#endif // !__ASIO2_MQTT_AOP_PUBLISH_HPP__
