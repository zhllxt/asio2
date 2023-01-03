/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_MQTT_ERROR_HPP__
#define __ASIO2_MQTT_ERROR_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/error.hpp>

namespace asio2::mqtt
{
	///// The type of error category used by the library
	//using error_category = asio::error_category;

	///// The type of error condition used by the library
	//using error_condition = std::error_condition;

	//enum class error
	//{
	//	success = 0,
	//	failure,
	//	persistence_error,
	//	disconnected,
	//	max_messages_inflight,
	//	bad_utf8_string,
	//	null_parameter,
	//	topicname_truncated,
	//	bad_structure,
	//	bad_qos,
	//	no_more_msgids,
	//	operation_incomplete,
	//	max_buffered_messages,
	//	ssl_not_supported,
	//	bad_mqtt_version,
	//	bad_protocol,
	//	bad_mqtt_option,
	//	wrong_mqtt_version,
	//	zero_len_will_topic,
	//};

	//enum class condition
	//{
	//	success = 0,
	//	failure,
	//	persistence_error, // unspecified_error
	//	disconnected, // disconnect_with_will_message
	//	max_messages_inflight, // message_rate_too_high
	//	bad_utf8_string, // topic_filter_invalid topic_name_invalid topic_alias_invalid
	//	null_parameter, // malformed_packet
	//	topicname_truncated, // topic_name_invalid
	//	bad_structure, // protocol_error implementation_specific_error
	//	bad_qos, // qos_not_supported
	//	no_more_msgids, // packet_identifier_not_found  no_matching_subscribers  no_subscription_existed
	//	operation_incomplete, // server_busy
	//	max_buffered_messages, // packet_too_large
	//	ssl_not_supported, // unsupported_protocol_version server_unavailable
	//	bad_mqtt_version, // unsupported_protocol_version
	//	bad_protocol, // protocol_error
	//	bad_mqtt_option, // payload_format_invalid protocol_error malformed_packet
	//	wrong_mqtt_version, // unsupported_protocol_version
	//	zero_len_will_topic, //  topic_filter_invalid topic_name_invalid topic_alias_invalid
	//};

	//class mqtt_error_category : public error_category
	//{
	//public:
	//	const char* name() const noexcept override
	//	{
	//		return "asio2.mqtt";
	//	}

	//	inline std::string message(int ev) const override
	//	{
	//		switch (static_cast<error>(ev))
	//		{
	//		case error::success               : return "Success";
	//		case error::failure               : return "Failure";
	//		case error::persistence_error     : return "Persistence error";
	//		case error::disconnected          : return "Disconnected";
	//		case error::max_messages_inflight : return "Maximum in-flight messages amount reached";
	//		case error::bad_utf8_string       : return "Invalid UTF8 string";
	//		case error::null_parameter        : return "Invalid (NULL) parameter";
	//		case error::topicname_truncated   : return "Topic containing NULL characters has been truncated";
	//		case error::bad_structure         : return "Bad structure";
	//		case error::bad_qos               : return "Invalid QoS value";
	//		case error::no_more_msgids        : return "Too many pending commands";
	//		case error::operation_incomplete  : return "Operation discarded before completion";
	//		case error::max_buffered_messages : return "No more messages can be buffered";
	//		case error::ssl_not_supported     : return "SSL is not supported";
	//		case error::bad_mqtt_version      : return "Unrecognized MQTT version";
	//		case error::bad_protocol          : return "Invalid protocol scheme";
	//		case error::bad_mqtt_option       : return "Options for wrong MQTT version";
	//		case error::wrong_mqtt_version    : return "Client created for another version of MQTT";
	//		case error::zero_len_will_topic   : return "Zero length will topic on connect";
	//		default                           : return "Unknown";
	//		}
	//	}

	//	inline error_condition default_error_condition(int ev) const noexcept override
	//	{
	//		//switch (static_cast<error>(ev))
	//		//{
	//		//default:
	//		//case error::disconnected          : return condition::disconnected          ;
	//		//case error::max_messages_inflight : return condition::max_messages_inflight ;
	//		//case error::bad_utf8_string       : return condition::bad_utf8_string       ;
	//		//case error::null_parameter        : return condition::null_parameter        ;
	//		//case error::topicname_truncated   : return condition::topicname_truncated   ;
	//		//case error::bad_structure         : return condition::bad_structure         ;
	//		//case error::bad_qos               : return condition::bad_qos               ;
	//		//case error::ssl_not_supported     : return condition::ssl_not_supported     ;
	//		//case error::bad_mqtt_version      : return condition::bad_mqtt_version      ;
	//		//case error::bad_protocol          : return condition::bad_protocol          ;
	//		//case error::bad_mqtt_option       : return condition::bad_mqtt_option       ;
	//		//case error::wrong_mqtt_version    : return condition::wrong_mqtt_version    ;
	//		//}
	//		return error_condition{ ev, *this };
	//	}
	//};

	//inline const mqtt_error_category& mqtt_category() noexcept
	//{
	//	static mqtt_error_category const cat{};
	//	return cat;
	//}

	//inline error_code make_error_code(error e)
	//{
	//	return error_code{ static_cast<std::underlying_type<error>::type>(e), mqtt_category() };
	//}

	//inline error_condition make_error_condition(condition c)
	//{
	//	return error_condition{ static_cast<std::underlying_type<condition>::type>(c), mqtt_category() };
	//}

	/**
	 * Reason Code
	 *
	 * A Reason Code is a one byte unsigned value that indicates the result of an operation. Reason Codes
	 * less than 0x80 indicate successful completion of an operation. The normal Reason Code for success
	 * is 0. Reason Code values of 0x80 or greater indicate failure.
	 *
	 * The Reason Codes share a common set of values as shown below.
	 * 
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031
	 */
	enum class error : std::uint8_t
	{
		// The operation completed successfully. 
		success                                  = 0,

		// Close the connection normally. Do not send the Will Message.
		normal_disconnection                     = 0,

		// Granted the PUBLISH Quality of Service is 0
		granted_qos_0                            = 0,

		// Granted the PUBLISH Quality of Service is 1
		granted_qos_1                            = 1,

		// Granted the PUBLISH Quality of Service is 2
		granted_qos_2                            = 2,

		// The Client wishes to disconnect but requires that the Server also publishes its Will Message.
		disconnect_with_will_message             = 4,

		// The message is accepted but there are no subscribers. This is sent only by the Server. If the Server knows that there are no matching subscribers, it MAY use this Reason Code instead of 0x00 (Success).
		no_matching_subscribers                  = 16,

		// No subscription existed
		no_subscription_existed                  = 17,

		// Continue the authentication with another step
		continue_authentication                  = 24,

		// Initiate a re-authentication
		reauthenticate                           = 25,

		// The Server or Client does not wish to reveal the reason for the failure, or none of the other Reason Codes apply.
		unspecified_error                        = 128,

		// The received packet does not conform to this specification.
		malformed_packet                         = 129,

		// An unexpected or out of order packet was received.
		protocol_error                           = 130,

		// The packet received is valid but cannot be processed by this implementation.
		implementation_specific_error            = 131,

		// The Server does not support the version of the MQTT protocol requested by the Client.
		unsupported_protocol_version             = 132,

		// The Client Identifier is a valid string but is not allowed by the Server.
		client_identifier_not_valid              = 133,

		// The Server does not accept the User Name or Password specified by the Client
		bad_user_name_or_password                = 134,

		// The request is not authorized.
		not_authorized                           = 135,

		// The MQTT Server is not available.
		server_unavailable                       = 136,

		// The Server is busy and cannot continue processing requests from this Client.
		server_busy                              = 137,

		// This Client has been banned by administrative action. Contact the server administrator.
		banned                                   = 138,

		// The Server is shutting down.
		server_shutting_down                     = 139,

		// The authentication method is not supported or does not match the authentication method currently in use.
		bad_authentication_method                = 140,

		// The Connection is closed because no packet has been received for 1.5 times the Keepalive time.
		keep_alive_timeout                       = 141,

		// Another Connection using the same ClientID has connected causing this Connection to be closed.
		session_taken_over                       = 142,

		// The Topic Filter is correctly formed, but is not accepted by this Sever.
		topic_filter_invalid                     = 143,

		// The Topic Name is not malformed, but is not accepted by this Client or Server.
		topic_name_invalid                       = 144,

		// The Packet Identifier is already in use. This might indicate a mismatch in the Session State between the Client and Server.
		packet_identifier_in_use                 = 145,

		// The Packet Identifier is not known. This is not an error during recovery, but at other times indicates a mismatch between the Session State on the Client and Server.
		packet_identifier_not_found              = 146,

		// The Client or Server has received more than Receive Maximum publication for which it has not sent PUBACK or PUBCOMP.
		receive_maximum_exceeded                 = 147,

		// The Client or Server has received a PUBLISH packet containing a Topic Alias which is greater than the Maximum Topic Alias it sent in the CONNECT or CONNACK packet.
		topic_alias_invalid                      = 148,

		// The packet size is greater than Maximum Packet Size for this Client or Server.
		packet_too_large                         = 149,

		// The received data rate is too high.
		message_rate_too_high                    = 150,

		// An implementation or administrative imposed limit has been exceeded.
		quota_exceeded                           = 151,

		// The Connection is closed due to an administrative action.
		administrative_action                    = 152,

		// The payload format does not match the specified Payload Format Indicator.
		payload_format_invalid                   = 153,

		// The Server has does not support retained messages.
		retain_not_supported                     = 154,

		// The Server does not support the QoS set in Will QoS.
		qos_not_supported                        = 155,

		// The Client should temporarily use another server.
		use_another_server                       = 156,

		// The Server is moved and the Client should permanently use another server.
		server_moved                             = 157,

		// The Server does not support Shared Subscriptions.
		shared_subscriptions_not_supported       = 158,

		// The connection rate limit has been exceeded.
		connection_rate_exceeded                 = 159,

		// The maximum connection time authorized for this connection has been exceeded.
		maximum_connect_time                     = 160,

		// The Server does not support Subscription Identifiers; the subscription is not accepted.
		subscription_identifiers_not_supported   = 161,

		// The Server does not support Wildcard Subscriptions; the subscription is not accepted.
		wildcard_subscriptions_not_supported     = 162,
	};

	enum class condition : std::uint8_t
	{
		// The operation completed successfully. 
		success                                  = 0,

		// Close the connection normally. Do not send the Will Message.
		normal_disconnection                     = 0,

		// Granted the PUBLISH Quality of Service is 0
		granted_qos_0                            = 0,

		// Granted the PUBLISH Quality of Service is 1
		granted_qos_1                            = 1,

		// Granted the PUBLISH Quality of Service is 2
		granted_qos_2                            = 2,

		// The Client wishes to disconnect but requires that the Server also publishes its Will Message.
		disconnect_with_will_message             = 4,

		// The message is accepted but there are no subscribers. This is sent only by the Server. If the Server knows that there are no matching subscribers, it MAY use this Reason Code instead of 0x00 (Success).
		no_matching_subscribers                  = 16,

		// No subscription existed
		no_subscription_existed                  = 17,

		// Continue the authentication with another step
		continue_authentication                  = 24,

		// Initiate a re-authentication
		reauthenticate                           = 25,

		// The Server or Client does not wish to reveal the reason for the failure, or none of the other Reason Codes apply.
		unspecified_error                        = 128,

		// The received packet does not conform to this specification.
		malformed_packet                         = 129,

		// An unexpected or out of order packet was received.
		protocol_error                           = 130,

		// The packet received is valid but cannot be processed by this implementation.
		implementation_specific_error            = 131,

		// The Server does not support the version of the MQTT protocol requested by the Client.
		unsupported_protocol_version             = 132,

		// The Client Identifier is a valid string but is not allowed by the Server.
		client_identifier_not_valid              = 133,

		// The Server does not accept the User Name or Password specified by the Client
		bad_user_name_or_password                = 134,

		// The request is not authorized.
		not_authorized                           = 135,

		// The MQTT Server is not available.
		server_unavailable                       = 136,

		// The Server is busy and cannot continue processing requests from this Client.
		server_busy                              = 137,

		// This Client has been banned by administrative action. Contact the server administrator.
		banned                                   = 138,

		// The Server is shutting down.
		server_shutting_down                     = 139,

		// The authentication method is not supported or does not match the authentication method currently in use.
		bad_authentication_method                = 140,

		// The Connection is closed because no packet has been received for 1.5 times the Keepalive time.
		keep_alive_timeout                       = 141,

		// Another Connection using the same ClientID has connected causing this Connection to be closed.
		session_taken_over                       = 142,

		// The Topic Filter is correctly formed, but is not accepted by this Sever.
		topic_filter_invalid                     = 143,

		// The Topic Name is not malformed, but is not accepted by this Client or Server.
		topic_name_invalid                       = 144,

		// The Packet Identifier is already in use. This might indicate a mismatch in the Session State between the Client and Server.
		packet_identifier_in_use                 = 145,

		// The Packet Identifier is not known. This is not an error during recovery, but at other times indicates a mismatch between the Session State on the Client and Server.
		packet_identifier_not_found              = 146,

		// The Client or Server has received more than Receive Maximum publication for which it has not sent PUBACK or PUBCOMP.
		receive_maximum_exceeded                 = 147,

		// The Client or Server has received a PUBLISH packet containing a Topic Alias which is greater than the Maximum Topic Alias it sent in the CONNECT or CONNACK packet.
		topic_alias_invalid                      = 148,

		// The packet size is greater than Maximum Packet Size for this Client or Server.
		packet_too_large                         = 149,

		// The received data rate is too high.
		message_rate_too_high                    = 150,

		// An implementation or administrative imposed limit has been exceeded.
		quota_exceeded                           = 151,

		// The Connection is closed due to an administrative action.
		administrative_action                    = 152,

		// The payload format does not match the specified Payload Format Indicator.
		payload_format_invalid                   = 153,

		// The Server has does not support retained messages.
		retain_not_supported                     = 154,

		// The Server does not support the QoS set in Will QoS.
		qos_not_supported                        = 155,

		// The Client should temporarily use another server.
		use_another_server                       = 156,

		// The Server is moved and the Client should permanently use another server.
		server_moved                             = 157,

		// The Server does not support Shared Subscriptions.
		shared_subscriptions_not_supported       = 158,

		// The connection rate limit has been exceeded.
		connection_rate_exceeded                 = 159,

		// The maximum connection time authorized for this connection has been exceeded.
		maximum_connect_time                     = 160,

		// The Server does not support Subscription Identifiers; the subscription is not accepted.
		subscription_identifiers_not_supported   = 161,

		// The Server does not support Wildcard Subscriptions; the subscription is not accepted.
		wildcard_subscriptions_not_supported     = 162,
	};

	/// The type of error category used by the library
	using error_category  = asio::error_category;

	/// The type of error condition used by the library
	using error_condition = asio::error_condition;

	class mqtt_error_category : public error_category
	{
	public:
		const char* name() const noexcept override
		{
			return "asio2.mqtt";
		}

		inline std::string message(int ev) const override
		{
			switch (static_cast<error>(ev))
			{
			case error::success                               : return "The operation completed successfully.";
			//case error::normal_disconnection                  : return "Close the connection normally. Do not send the Will Message.";
			//case error::granted_qos_0                         : return "Granted the PUBLISH Quality of Service is 0";
			case error::granted_qos_1                         : return "Granted the PUBLISH Quality of Service is 1";
			case error::granted_qos_2                         : return "Granted the PUBLISH Quality of Service is 2";
			case error::disconnect_with_will_message          : return "The Client wishes to disconnect but requires that the Server also publishes its Will Message.";
			case error::no_matching_subscribers               : return "The message is accepted but there are no subscribers. This is sent only by the Server. If the Server knows that there are no matching subscribers, it MAY use this Reason Code instead of 0x00 (Success).";
			case error::no_subscription_existed               : return "No subscription existed";
			case error::continue_authentication               : return "Continue the authentication with another step";
			case error::reauthenticate                        : return "Initiate a re-authentication";
			case error::unspecified_error                     : return "The Server or Client does not wish to reveal the reason for the failure, or none of the other Reason Codes apply.";
			case error::malformed_packet                      : return "The received packet does not conform to this specification.";
			case error::protocol_error                        : return "An unexpected or out of order packet was received.";
			case error::implementation_specific_error         : return "The packet received is valid but cannot be processed by this implementation.";
			case error::unsupported_protocol_version          : return "The Server does not support the version of the MQTT protocol requested by the Client.";
			case error::client_identifier_not_valid           : return "The Client Identifier is a valid string but is not allowed by the Server.";
			case error::bad_user_name_or_password             : return "The Server does not accept the User Name or Password specified by the Client";
			case error::not_authorized                        : return "The request is not authorized.";
			case error::server_unavailable                    : return "The MQTT Server is not available.";
			case error::server_busy                           : return "The Server is busy and cannot continue processing requests from this Client.";
			case error::banned                                : return "This Client has been banned by administrative action. Contact the server administrator.";
			case error::server_shutting_down                  : return "The Server is shutting down.";
			case error::bad_authentication_method             : return "The authentication method is not supported or does not match the authentication method currently in use.";
			case error::keep_alive_timeout                    : return "The Connection is closed because no packet has been received for 1.5 times the Keepalive time.";
			case error::session_taken_over                    : return "Another Connection using the same ClientID has connected causing this Connection to be closed.";
			case error::topic_filter_invalid                  : return "The Topic Filter is correctly formed, but is not accepted by this Sever.";
			case error::topic_name_invalid                    : return "The Topic Name is not malformed, but is not accepted by this Client or Server.";
			case error::packet_identifier_in_use              : return "The Packet Identifier is already in use. This might indicate a mismatch in the Session State between the Client and Server.";
			case error::packet_identifier_not_found           : return "The Packet Identifier is not known. This is not an error during recovery, but at other times indicates a mismatch between the Session State on the Client and Server.";
			case error::receive_maximum_exceeded              : return "The Client or Server has received more than Receive Maximum publication for which it has not sent PUBACK or PUBCOMP.";
			case error::topic_alias_invalid                   : return "The Client or Server has received a PUBLISH packet containing a Topic Alias which is greater than the Maximum Topic Alias it sent in the CONNECT or CONNACK packet.";
			case error::packet_too_large                      : return "The packet size is greater than Maximum Packet Size for this Client or Server.";
			case error::message_rate_too_high                 : return "The received data rate is too high.";
			case error::quota_exceeded                        : return "An implementation or administrative imposed limit has been exceeded.";
			case error::administrative_action                 : return "The Connection is closed due to an administrative action.";
			case error::payload_format_invalid                : return "The payload format does not match the specified Payload Format Indicator.";
			case error::retain_not_supported                  : return "The Server has does not support retained messages.";
			case error::qos_not_supported                     : return "The Server does not support the QoS set in Will QoS.";
			case error::use_another_server                    : return "The Client should temporarily use another server.";
			case error::server_moved                          : return "The Server is moved and the Client should permanently use another server.";
			case error::shared_subscriptions_not_supported    : return "The Server does not support Shared Subscriptions.";
			case error::connection_rate_exceeded              : return "The connection rate limit has been exceeded.";
			case error::maximum_connect_time                  : return "The maximum connection time authorized for this connection has been exceeded.";
			case error::subscription_identifiers_not_supported: return "The Server does not support Subscription Identifiers; the subscription is not accepted.";
			case error::wildcard_subscriptions_not_supported  : return "The Server does not support Wildcard Subscriptions; the subscription is not accepted.";
			default                                           : return "Unknown error";
			}
		}

		inline error_condition default_error_condition(int ev) const noexcept override
		{
			return error_condition{ ev, *this };
		}
	};

	inline const mqtt_error_category& mqtt_category() noexcept
	{
		static mqtt_error_category const cat{};
		return cat;
	}

	inline asio::error_code make_error_code(error e)
	{
		return asio::error_code{ static_cast<std::underlying_type<error>::type>(e), mqtt_category() };
	}

	inline error_condition make_error_condition(condition c)
	{
		return error_condition{ static_cast<std::underlying_type<condition>::type>(c), mqtt_category() };
	}

	template<typename = void>
	inline constexpr std::string_view to_string(error e)
	{
		using namespace std::string_view_literals;
		switch (e)
		{
		case error::success                               : return "The operation completed successfully.";
		//case error::normal_disconnection                  : return "Close the connection normally. Do not send the Will Message.";
		//case error::granted_qos_0                         : return "Granted the PUBLISH Quality of Service is 0";
		case error::granted_qos_1                         : return "Granted the PUBLISH Quality of Service is 1";
		case error::granted_qos_2                         : return "Granted the PUBLISH Quality of Service is 2";
		case error::disconnect_with_will_message          : return "The Client wishes to disconnect but requires that the Server also publishes its Will Message.";
		case error::no_matching_subscribers               : return "The message is accepted but there are no subscribers. This is sent only by the Server. If the Server knows that there are no matching subscribers, it MAY use this Reason Code instead of 0x00 (Success).";
		case error::no_subscription_existed               : return "No subscription existed";
		case error::continue_authentication               : return "Continue the authentication with another step";
		case error::reauthenticate                        : return "Initiate a re-authentication";
		case error::unspecified_error                     : return "The Server or Client does not wish to reveal the reason for the failure, or none of the other Reason Codes apply.";
		case error::malformed_packet                      : return "The received packet does not conform to this specification.";
		case error::protocol_error                        : return "An unexpected or out of order packet was received.";
		case error::implementation_specific_error         : return "The packet received is valid but cannot be processed by this implementation.";
		case error::unsupported_protocol_version          : return "The Server does not support the version of the MQTT protocol requested by the Client.";
		case error::client_identifier_not_valid           : return "The Client Identifier is a valid string but is not allowed by the Server.";
		case error::bad_user_name_or_password             : return "The Server does not accept the User Name or Password specified by the Client";
		case error::not_authorized                        : return "The request is not authorized.";
		case error::server_unavailable                    : return "The MQTT Server is not available.";
		case error::server_busy                           : return "The Server is busy and cannot continue processing requests from this Client.";
		case error::banned                                : return "This Client has been banned by administrative action. Contact the server administrator.";
		case error::server_shutting_down                  : return "The Server is shutting down.";
		case error::bad_authentication_method             : return "The authentication method is not supported or does not match the authentication method currently in use.";
		case error::keep_alive_timeout                    : return "The Connection is closed because no packet has been received for 1.5 times the Keepalive time.";
		case error::session_taken_over                    : return "Another Connection using the same ClientID has connected causing this Connection to be closed.";
		case error::topic_filter_invalid                  : return "The Topic Filter is correctly formed, but is not accepted by this Sever.";
		case error::topic_name_invalid                    : return "The Topic Name is not malformed, but is not accepted by this Client or Server.";
		case error::packet_identifier_in_use              : return "The Packet Identifier is already in use. This might indicate a mismatch in the Session State between the Client and Server.";
		case error::packet_identifier_not_found           : return "The Packet Identifier is not known. This is not an error during recovery, but at other times indicates a mismatch between the Session State on the Client and Server.";
		case error::receive_maximum_exceeded              : return "The Client or Server has received more than Receive Maximum publication for which it has not sent PUBACK or PUBCOMP.";
		case error::topic_alias_invalid                   : return "The Client or Server has received a PUBLISH packet containing a Topic Alias which is greater than the Maximum Topic Alias it sent in the CONNECT or CONNACK packet.";
		case error::packet_too_large                      : return "The packet size is greater than Maximum Packet Size for this Client or Server.";
		case error::message_rate_too_high                 : return "The received data rate is too high.";
		case error::quota_exceeded                        : return "An implementation or administrative imposed limit has been exceeded.";
		case error::administrative_action                 : return "The Connection is closed due to an administrative action.";
		case error::payload_format_invalid                : return "The payload format does not match the specified Payload Format Indicator.";
		case error::retain_not_supported                  : return "The Server has does not support retained messages.";
		case error::qos_not_supported                     : return "The Server does not support the QoS set in Will QoS.";
		case error::use_another_server                    : return "The Client should temporarily use another server.";
		case error::server_moved                          : return "The Server is moved and the Client should permanently use another server.";
		case error::shared_subscriptions_not_supported    : return "The Server does not support Shared Subscriptions.";
		case error::connection_rate_exceeded              : return "The connection rate limit has been exceeded.";
		case error::maximum_connect_time                  : return "The maximum connection time authorized for this connection has been exceeded.";
		case error::subscription_identifiers_not_supported: return "The Server does not support Subscription Identifiers; the subscription is not accepted.";
		case error::wildcard_subscriptions_not_supported  : return "The Server does not support Wildcard Subscriptions; the subscription is not accepted.";
		default                                           : return "Unknown error";
		}
		return "Unknown error";
	};
}

namespace mqtt = ::asio2::mqtt;

namespace std
{
	template<>
	struct is_error_code_enum<::asio2::mqtt::error>
	{
		static bool const value = true;
	};
	template<>
	struct is_error_condition_enum<::asio2::mqtt::condition>
	{
		static bool const value = true;
	};
}

#endif // !__ASIO2_MQTT_ERROR_HPP__
