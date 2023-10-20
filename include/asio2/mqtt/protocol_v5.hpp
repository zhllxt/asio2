/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * chinese : http://mqtt.p2hp.com/mqtt-5-0
 * english : https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_MQTT_PROTOCOL_V5_HPP__
#define __ASIO2_MQTT_PROTOCOL_V5_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/mqtt/core.hpp>

namespace asio2::mqtt::v5
{
	static constexpr std::uint8_t version_number = asio2::detail::to_underlying(mqtt::version::v5);

	/**
	 * Property
	 *
	 * A Property consists of an Identifier which defines its usage and data type, followed by a value. 
	 * The Identifier is encoded as a Variable Byte Integer. A Control Packet which contains an Identifier
	 * which is not valid for its packet type, or contains a value not of the specified data type, is a
	 * Malformed Packet. If received, use a CONNACK or DISCONNECT packet with Reason Code 0x81 (Malformed Packet)
	 * as described in section 4.13 Handling errors. There is no significance in the order of Properties with
	 * different Identifiers.
	 *
	 * Although the Property Identifier is defined as a Variable Byte Integer, in this version of the
	 * specification all of the Property Identifiers are one byte long.
	 *
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901027
	 */
	enum class property_type : std::int32_t
	{
		payload_format_indicator          =  1,	// Byte
		message_expiry_interval           =  2,	// Four Byte Integer
		content_type                      =  3,	// UTF-8 Encoded String
		response_topic                    =  8,	// UTF-8 Encoded String
		correlation_data                  =  9,	// Binary Data
		subscription_identifier           = 11,	// Variable Byte Integer
		session_expiry_interval           = 17,	// Four Byte Integer
		assigned_client_identifier        = 18,	// UTF-8 Encoded String
		server_keep_alive                 = 19,	// Two Byte Integer
		authentication_method             = 21,	// UTF-8 Encoded String
		authentication_data               = 22,	// Binary Data
		request_problem_information       = 23,	// Byte
		will_delay_interval               = 24,	// Four Byte Integer
		request_response_information      = 25,	// Byte
		response_information              = 26,	// UTF-8 Encoded String
		server_reference                  = 28,	// UTF-8 Encoded String
		reason_string                     = 31,	// UTF-8 Encoded String
		receive_maximum                   = 33,	// Two Byte Integer
		topic_alias_maximum               = 34,	// Two Byte Integer
		topic_alias                       = 35,	// Two Byte Integer
		maximum_qos                       = 36,	// Byte
		retain_available                  = 37,	// Byte
		user_property                     = 38,	// UTF-8 String Pair
		maximum_packet_size               = 39,	// Four Byte Integer
		wildcard_subscription_available   = 40,	// Byte
		subscription_identifier_available = 41,	// Byte
		shared_subscription_available     = 42,	// Byte
	};
}

namespace asio2::mqtt
{
	template<typename = void>
	inline constexpr std::string_view to_string(v5::property_type type)
	{
		using namespace std::string_view_literals;
		switch (type)
		{
		case v5::property_type::payload_format_indicator          : return "payload_format_indicator"sv;
		case v5::property_type::message_expiry_interval           : return "message_expiry_interval"sv;
		case v5::property_type::content_type                      : return "content_type"sv;
		case v5::property_type::response_topic                    : return "response_topic"sv;
		case v5::property_type::correlation_data                  : return "correlation_data"sv;
		case v5::property_type::subscription_identifier           : return "subscription_identifier"sv;
		case v5::property_type::session_expiry_interval           : return "session_expiry_interval"sv;
		case v5::property_type::assigned_client_identifier        : return "assigned_client_identifier"sv;
		case v5::property_type::server_keep_alive                 : return "server_keep_alive"sv;
		case v5::property_type::authentication_method             : return "authentication_method"sv;
		case v5::property_type::authentication_data               : return "authentication_data"sv;
		case v5::property_type::request_problem_information       : return "request_problem_information"sv;
		case v5::property_type::will_delay_interval               : return "will_delay_interval"sv;
		case v5::property_type::request_response_information      : return "request_response_information"sv;
		case v5::property_type::response_information              : return "response_information"sv;
		case v5::property_type::server_reference                  : return "server_reference"sv;
		case v5::property_type::reason_string                     : return "reason_string"sv;
		case v5::property_type::receive_maximum                   : return "receive_maximum"sv;
		case v5::property_type::topic_alias_maximum               : return "topic_alias_maximum"sv;
		case v5::property_type::topic_alias                       : return "topic_alias"sv;
		case v5::property_type::maximum_qos                       : return "maximum_qos"sv;
		case v5::property_type::retain_available                  : return "retain_available"sv;
		case v5::property_type::user_property                     : return "user_property"sv;
		case v5::property_type::maximum_packet_size               : return "maximum_packet_size"sv;
		case v5::property_type::wildcard_subscription_available   : return "wildcard_subscription_available"sv;
		case v5::property_type::subscription_identifier_available : return "subscription_identifier_available"sv;
		case v5::property_type::shared_subscription_available     : return "shared_subscription_available"sv;
		}
		return ""sv;
	};
}

namespace asio2::mqtt::v5
{
	// forward declared
	template<class> struct property_ops;
	// forward declared
	class properties_set;

	/**
	 * A Property consists of an Identifier which defines its usage and data type, followed by a value.
	 * The Identifier is encoded as a Variable Byte Integer.
	 * A Control Packet which contains an Identifier which is not valid for its packet type, or contains
	 * a value not of the specified data type, is a Malformed Packet. If received, use a CONNACK or 
	 * DISCONNECT packet with Reason Code 0x81 (Malformed Packet) as described in section 4.13 Handling 
	 * errors. There is no significance in the order of Properties with different Identifiers.
	 * 
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901029
	 */
	template<property_type T>
	struct basic_property
	{
		template<class> friend struct property_ops; friend class properties_set;

		inline variable_byte_integer::value_type id() const { return id_.value();  }

		inline constexpr property_type         type() const { return T;            }

		inline constexpr std::string_view      name() const { return mqtt::to_string(T); }

	protected:
		// Property Identifier
		// Although the Property Identifier is defined as a Variable Byte Integer, in this version of the
		// specification all of the Property Identifiers are one byte long.
		variable_byte_integer id_{ asio2::detail::to_underlying(T) };
	};

	/**
	 * Payload Format Indicator
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901063
	 */
	template<class derived_t>
	struct property_ops
	{
		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline derived_t& serialize(Container& buffer)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			derive.id_   .serialize(buffer);
			derive.value_.serialize(buffer);

			return derive;
		}

		inline derived_t& deserialize(std::string_view& data)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			derive.id_   .deserialize(data);
			derive.value_.deserialize(data);

			return derive;
		}

		inline std::size_t required_size()
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return (derive.id_.required_size() + derive.value_.required_size());
		}

		inline std::size_t required_size() const
		{
			derived_t const& derive = static_cast<derived_t const&>(*this);

			return (derive.id_.required_size() + derive.value_.required_size());
		}
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Properties
	// https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901027
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	 * Payload Format Indicator
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901063
	 */
	struct payload_format_indicator
		: public basic_property<property_type::payload_format_indicator>
		, public property_ops<payload_format_indicator>
	{
		// template<class> friend struct property_ops; friend class properties_set;
		// can't use the code previous line, it will cause compile error under gcc 8.2.0.
		friend struct property_ops<payload_format_indicator>; friend class properties_set;

		enum class format : std::uint8_t
		{
			binary,
			string
		};

		payload_format_indicator() = default;

		payload_format_indicator(format v) : value_(asio2::detail::to_underlying(v)) {}

		/**
		 * 0 (0x00) Indicates that the Will Message is unspecified bytes.
		 * 1 (0x01) Indicates that the Will Message is UTF-8 Encoded Character Data.
		 */
		payload_format_indicator(std::uint8_t v) : value_(v) {}

		inline one_byte_integer::value_type value() const { return value_.value(); }

	protected:
		// Followed by the value of the Payload Format Indicator, either of: 0 or 1
		one_byte_integer value_{};
	};

	/**
	 * Message Expiry Interval
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901112
	 */
	struct message_expiry_interval
		: public basic_property<property_type::message_expiry_interval>
		, public property_ops<message_expiry_interval>
	{
		friend struct property_ops<message_expiry_interval>; friend class properties_set;

		message_expiry_interval() = default;
		message_expiry_interval(std::uint32_t v) : value_(v) {}

		inline four_byte_integer::value_type value() const { return value_.value(); }

	protected:
		// The Four Byte value is the lifetime of the Application Message in seconds.
		four_byte_integer value_{};
	};

	/**
	 * Content Type
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901118
	 */
	struct content_type
		: public basic_property<property_type::content_type>
		, public property_ops<content_type>
	{
		friend struct property_ops<content_type>; friend class properties_set;

		content_type() = default;

		template<class String, std::enable_if_t<asio2::detail::is_character_string_v<String>, int> = 0>
		explicit content_type(String&& v) : value_(std::forward<String>(v)) {}

		inline utf8_string::view_type value() const { return value_.data_view(); }

	protected:
		// Followed by a UTF-8 Encoded String describing the content of the Application Message.
		// The Content Type MUST be a UTF-8 Encoded String as defined in section 1.5.4 [MQTT-3.3.2-19].
		// It is a Protocol Error to include the Content Type more than once.The value of the Content Type
		// is defined by the sending and receiving application.
		utf8_string value_{};
	};

	/**
	 * Response Topic
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901114
	 */
	struct response_topic
		: public basic_property<property_type::response_topic>
		, public property_ops<response_topic>
	{
		friend struct property_ops<response_topic>; friend class properties_set;

		response_topic() = default;

		template<class String, std::enable_if_t<asio2::detail::is_character_string_v<String>, int> = 0>
		response_topic(String&& v) : value_(std::forward<String>(v)) {}

		inline utf8_string::view_type value() const { return value_.data_view(); }

	protected:
		// Followed by a UTF-8 Encoded String which is used as the Topic Name for a response message. 
		// The Response Topic MUST be a UTF-8 Encoded String as defined in section 1.5.4 [MQTT-3.3.2-13].
		// The Response Topic MUST NOT contain wildcard characters [MQTT-3.3.2-14].
		// It is a Protocol Error to include the Response Topic more than once.
		// The presence of a Response Topic identifies the Message as a Request.
		utf8_string value_{};
	};

	/**
	 * Correlation Data
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901115
	 */
	struct correlation_data
		: public basic_property<property_type::correlation_data>
		, public property_ops<correlation_data>
	{
		friend struct property_ops<correlation_data>; friend class properties_set;

		correlation_data() = default;

		template<class String, std::enable_if_t<asio2::detail::is_character_string_v<String>, int> = 0>
		correlation_data(String&& v) : value_(std::forward<String>(v)) {}

		inline binary_data::view_type value() const { return value_.data_view(); }

	protected:
		// Followed by Binary Data.
		// The Correlation Data is used by the sender of the Request Message to identify which request the
		// Response Message is for when it is received.
		// It is a Protocol Error to include Correlation Data more than once.
		// If the Correlation Data is not present, the Requester does not require any correlation data.
		// The Server MUST send the Correlation Data unaltered to all subscribers receiving the Application
		// Message[MQTT - 3.3.2 - 16].
		// The value of the Correlation Data only has meaning to the sender of the Request Message and
		// receiver of the Response Message.
		binary_data value_{};
	};

	/**
	 * Subscription Identifier
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901166
	 */
	struct subscription_identifier
		: public basic_property<property_type::subscription_identifier>
		, public property_ops<subscription_identifier>
	{
		friend struct property_ops<subscription_identifier>; friend class properties_set;

		subscription_identifier() = default;
		subscription_identifier(std::int32_t v) : value_(v) {}

		inline variable_byte_integer::value_type value() const { return value_.value(); }

	protected:
		// Followed by a Variable Byte Integer representing the identifier of the subscription.
		// The Subscription Identifier can have the value of 1 to 268,435,455.
		// It is a Protocol Error if the Subscription Identifier has a value of 0. 
		// It is a Protocol Error to include the Subscription Identifier more than once.
		variable_byte_integer value_{};
	};

	/**
	 * Session Expiry Interval
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901048
	 */
	struct session_expiry_interval
		: public basic_property<property_type::session_expiry_interval>
		, public property_ops<session_expiry_interval>
	{
		friend struct property_ops<session_expiry_interval>; friend class properties_set;

		session_expiry_interval() = default;
		session_expiry_interval(std::uint32_t v) : value_(v) {}

		inline four_byte_integer::value_type value() const { return value_.value(); }

	protected:
		// Followed by the Four Byte Integer representing the Session Expiry Interval in seconds. 
		// It is a Protocol Error to include the Session Expiry Interval more than once.
		four_byte_integer value_{};
	};

	/**
	 * Assigned Client Identifier
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901087
	 */
	struct assigned_client_identifier
		: public basic_property<property_type::assigned_client_identifier>
		, public property_ops<assigned_client_identifier>
	{
		friend struct property_ops<assigned_client_identifier>; friend class properties_set;

		assigned_client_identifier() = default;

		template<class String, std::enable_if_t<asio2::detail::is_character_string_v<String>, int> = 0>
		assigned_client_identifier(String&& v) : value_(std::forward<String>(v)) {}

		inline utf8_string::view_type value() const { return value_.data_view(); }

	protected:
		// Followed by the UTF-8 string which is the Assigned Client Identifier. 
		// It is a Protocol Error to include the Assigned Client Identifier more than once.
		utf8_string value_{};
	};

	/**
	 * Server Keep Alive
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901094
	 */
	struct server_keep_alive
		: public basic_property<property_type::server_keep_alive>
		, public property_ops<server_keep_alive>
	{
		friend struct property_ops<server_keep_alive>; friend class properties_set;

		server_keep_alive() = default;
		server_keep_alive(std::uint16_t v) : value_(v) {}

		inline two_byte_integer::value_type value() const { return value_.value(); }

	protected:
		// Followed by a Two Byte Integer with the Keep Alive time assigned by the Server. 
		// If the Server sends a Server Keep Alive on the CONNACK packet, the Client MUST use this value
		// instead of the Keep Alive value the Client sent on CONNECT [MQTT-3.2.2-21]. 
		// If the Server does not send the Server Keep Alive, the Server MUST use the Keep Alive value
		// set by the Client on CONNECT [MQTT-3.2.2-22].
		// It is a Protocol Error to include the Server Keep Alive more than once.
		two_byte_integer value_{};
	};

	/**
	 * Authentication Method
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901055
	 */
	struct authentication_method
		: public basic_property<property_type::authentication_method>
		, public property_ops<authentication_method>
	{
		friend struct property_ops<authentication_method>; friend class properties_set;

		authentication_method() = default;

		template<class String, std::enable_if_t<asio2::detail::is_character_string_v<String>, int> = 0>
		authentication_method(String&& v) : value_(std::forward<String>(v)) {}

		inline utf8_string::view_type value() const { return value_.data_view(); }

	protected:
		// Followed by a UTF-8 Encoded String containing the name of the authentication method used
		// for extended authentication .
		// It is a Protocol Error to include Authentication Method more than once.
		// If Authentication Method is absent, extended authentication is not performed.Refer to section 4.12.
		utf8_string value_{};
	};

	/**
	 * Authentication Data
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901056
	 */
	struct authentication_data
		: public basic_property<property_type::authentication_data>
		, public property_ops<authentication_data>
	{
		friend struct property_ops<authentication_data>; friend class properties_set;

		authentication_data() = default;

		template<class String, std::enable_if_t<asio2::detail::is_character_string_v<String>, int> = 0>
		authentication_data(String&& v) : value_(std::forward<String>(v)) {}

		inline binary_data::view_type value() const { return value_.data_view(); }

	protected:
		// Followed by Binary Data containing authentication data.
		// It is a Protocol Error to include Authentication Data if there is no Authentication Method.
		// It is a Protocol Error to include Authentication Data more than once.
		binary_data value_{};
	};

	/**
	 * Request Problem Information
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901053
	 */
	struct request_problem_information
		: public basic_property<property_type::request_problem_information>
		, public property_ops<request_problem_information>
	{
		friend struct property_ops<request_problem_information>; friend class properties_set;

		request_problem_information() = default;
		request_problem_information(std::uint8_t v) : value_(v) {}

		inline one_byte_integer::value_type value() const { return value_.value(); }

	protected:
		// Followed by a Byte with a value of either 0 or 1.
		// It is a Protocol Error to include Request Problem Information more than once, or to have a
		// value other than 0 or 1. If the Request Problem Information is absent, the value of 1 is used.
		one_byte_integer value_{};
	};

	/**
	 * Will Delay Interval
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901062
	 */
	struct will_delay_interval
		: public basic_property<property_type::will_delay_interval>
		, public property_ops<will_delay_interval>
	{
		friend struct property_ops<will_delay_interval>; friend class properties_set;

		will_delay_interval() = default;
		will_delay_interval(std::uint32_t v) : value_(v) {}

		inline four_byte_integer::value_type value() const { return value_.value(); }

	protected:
		// Followed by the Four Byte Integer representing the Will Delay Interval in seconds.
		// It is a Protocol Error to include the Will Delay Interval more than once.
		// If the Will Delay Interval is absent, the default value is 0 and there is no delay before the
		// Will Message is published.
		four_byte_integer value_{};
	};

	/**
	 * Request Response Information
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901052
	 */
	struct request_response_information
		: public basic_property<property_type::request_response_information>
		, public property_ops<request_response_information>
	{
		friend struct property_ops<request_response_information>; friend class properties_set;

		request_response_information() = default;
		request_response_information(std::uint8_t v) : value_(v) {}

		inline one_byte_integer::value_type value() const { return value_.value(); }

	protected:
		// Followed by a Byte with a value of either 0 or 1. 
		// It is Protocol Error to include the Request Response Information more than once, or to have a 
		// value other than 0 or 1. If the Request Response Information is absent, the value of 0 is used.
		one_byte_integer value_{};
	};

	/**
	 * Response Information
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901095
	 */
	struct response_information
		: public basic_property<property_type::response_information>
		, public property_ops<response_information>
	{
		friend struct property_ops<response_information>; friend class properties_set;

		response_information() = default;

		template<class String, std::enable_if_t<asio2::detail::is_character_string_v<String>, int> = 0>
		response_information(String&& v) : value_(std::forward<String>(v)) {}

		inline utf8_string::view_type value() const { return value_.data_view(); }

	protected:
		// Followed by a UTF-8 Encoded String which is used as the basis for creating a Response Topic.
		// The way in which the Client creates a Response Topic from the Response Information is not 
		// defined by this specification. It is a Protocol Error to include the Response Information
		// more than once.
		utf8_string value_{};
	};

	/**
	 * Server Reference
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901214
	 */
	struct server_reference
		: public basic_property<property_type::server_reference>
		, public property_ops<server_reference>
	{
		friend struct property_ops<server_reference>; friend class properties_set;

		server_reference() = default;

		template<class String, std::enable_if_t<asio2::detail::is_character_string_v<String>, int> = 0>
		server_reference(String&& v) : value_(std::forward<String>(v)) {}

		inline utf8_string::view_type value() const { return value_.data_view(); }

	protected:
		// Followed by a UTF-8 Encoded String which can be used by the Client to identify another Server to use.
		// It is a Protocol Error to include the Server Reference more than once.
		utf8_string value_{};
	};

	/**
	 * Reason String
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901089
	 */
	struct reason_string
		: public basic_property<property_type::reason_string>
		, public property_ops<reason_string>
	{
		friend struct property_ops<reason_string>; friend class properties_set;

		reason_string() = default;

		template<class String, std::enable_if_t<asio2::detail::is_character_string_v<String>, int> = 0>
		reason_string(String&& v) : value_(std::forward<String>(v)) {}

		inline utf8_string::view_type value() const { return value_.data_view(); }

	protected:
		// Followed by the UTF-8 Encoded String representing the reason associated with this response.
		// This Reason String is a human readable string designed for diagnostics and SHOULD NOT be
		// parsed by the Client.
		utf8_string value_{};
	};

	/**
	 * Receive Maximum
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901049
	 */
	struct receive_maximum
		: public basic_property<property_type::receive_maximum>
		, public property_ops<receive_maximum>
	{
		friend struct property_ops<receive_maximum>; friend class properties_set;

		receive_maximum() = default;
		receive_maximum(std::uint16_t v) : value_(v) {}

		inline two_byte_integer::value_type value() const { return value_.value(); }

	protected:
		// Followed by the Two Byte Integer representing the Receive Maximum value.
		// It is a Protocol Error to include the Receive Maximum value more than once or for it to have the value 0.
		two_byte_integer value_{};
	};

	/**
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901051
	 */
	struct topic_alias_maximum
		: public basic_property<property_type::topic_alias_maximum>
		, public property_ops<topic_alias_maximum>
	{
		friend struct property_ops<topic_alias_maximum>; friend class properties_set;

		topic_alias_maximum() = default;
		topic_alias_maximum(std::uint16_t v) : value_(v) {}

		inline two_byte_integer::value_type value() const { return value_.value(); }

	protected:
		// Followed by the Two Byte Integer representing the Topic Alias Maximum value.
		// It is a Protocol Error to include the Topic Alias Maximum value more than once.
		// If the Topic Alias Maximum property is absent, the default value is 0.
		two_byte_integer value_{};
	};

	/**
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901113
	 */
	struct topic_alias
		: public basic_property<property_type::topic_alias>
		, public property_ops<topic_alias>
	{
		friend struct property_ops<topic_alias>; friend class properties_set;

		topic_alias() = default;
		topic_alias(std::uint16_t v) : value_(v) {}

		inline two_byte_integer::value_type value() const { return value_.value(); }

	protected:
		// Followed by the Two Byte integer representing the Topic Alias value.
		// It is a Protocol Error to include the Topic Alias value more than once.
		two_byte_integer value_{};
	};

	/**
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901084
	 */
	struct maximum_qos
		: public basic_property<property_type::maximum_qos>
		, public property_ops<maximum_qos>
	{
		friend struct property_ops<maximum_qos>; friend class properties_set;

		maximum_qos() = default;
		maximum_qos(std::uint8_t v) : value_(v) {}

		inline one_byte_integer::value_type value() const { return value_.value(); }

	protected:
		// Followed by a Byte with a value of either 0 or 1.
		// It is a Protocol Error to include Maximum QoS more than once, or to have a value other than 0 or 1.
		// If the Maximum QoS is absent, the Client uses a Maximum QoS of 2.
		one_byte_integer value_{};
	};

	/**
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901085
	 */
	struct retain_available
		: public basic_property<property_type::retain_available>
		, public property_ops<retain_available>
	{
		friend struct property_ops<retain_available>; friend class properties_set;

		retain_available() = default;
		retain_available(std::uint8_t v) : value_(v) {}

		inline one_byte_integer::value_type value() const { return value_.value(); }

	protected:
		// Followed by a Byte field.
		// If present, this byte declares whether the Server supports retained messages.
		// A value of 0 means that retained messages are not supported.
		// A value of 1 means retained messages are supported.
		// If not present, then retained messages are supported.
		// It is a Protocol Error to include Retain Available more than once or to use a value other than 0 or 1.
		one_byte_integer value_{};
	};

	/**
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901054
	 */
	struct user_property
		: public basic_property<property_type::user_property>
		, public property_ops<user_property>
	{
		friend struct property_ops<user_property>; friend class properties_set;

		user_property() = default;

		template<class String1, class String2,
			std::enable_if_t<
			asio2::detail::is_character_string_v<String1>&&
			asio2::detail::is_character_string_v<String2>, int> = 0>
		user_property(String1&& k, String2&& v) : value_(std::forward<String1>(k), std::forward<String2>(v)) {}

		inline utf8_string_pair::view_type value() const { return { value_.key_view(), value_.val_view() }; }

	protected:
		// Followed by a UTF-8 String Pair.
		utf8_string_pair value_{};
	};

	/**
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901050
	 */
	struct maximum_packet_size
		: public basic_property<property_type::maximum_packet_size>
		, public property_ops<maximum_packet_size>
	{
		friend struct property_ops<maximum_packet_size>; friend class properties_set;

		maximum_packet_size() = default;
		maximum_packet_size(std::uint32_t v) : value_(v) {}

		inline four_byte_integer::value_type value() const { return value_.value(); }

	protected:
		// Followed by a Four Byte Integer representing the Maximum Packet Size the Client is willing to accept.
		// If the Maximum Packet Size is not present, no limit on the packet size is imposed beyond the
		// limitations in the protocol as a result of the remaining length encoding and the protocol header sizes.
		four_byte_integer value_{};
	};

	/**
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901091
	 */
	struct wildcard_subscription_available
		: public basic_property<property_type::wildcard_subscription_available>
		, public property_ops<wildcard_subscription_available>
	{
		friend struct property_ops<wildcard_subscription_available>; friend class properties_set;

		wildcard_subscription_available() = default;
		wildcard_subscription_available(std::uint8_t v) : value_(v) {}

		inline one_byte_integer::value_type value() const { return value_.value(); }

	protected:
		// Followed by a Byte field.
		// If present, this byte declares whether the Server supports Wildcard Subscriptions.
		// A value is 0 means that Wildcard Subscriptions are not supported.
		// A value of 1 means Wildcard Subscriptions are supported.
		// If not present, then Wildcard Subscriptions are supported.
		// It is a Protocol Error to include the Wildcard Subscription Available more than once or to
		// send a value other than 0 or 1.
		one_byte_integer value_{};
	};

	/**
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901092
	 */
	struct subscription_identifier_available
		: public basic_property<property_type::subscription_identifier_available>
		, public property_ops<subscription_identifier_available>
	{
		friend struct property_ops<subscription_identifier_available>; friend class properties_set;

		subscription_identifier_available() = default;
		subscription_identifier_available(std::uint8_t v) : value_(v) {}

		inline one_byte_integer::value_type value() const { return value_.value(); }

	protected:
		// Followed by a Byte field.
		// If present, this byte declares whether the Server supports Subscription Identifiers.
		// A value is 0 means that Subscription Identifiers are not supported.
		// A value of 1 means Subscription Identifiers are supported.
		// If not present, then Subscription Identifiers are supported.
		// It is a Protocol Error to include the Subscription Identifier Available more than once, or to
		// send a value other than 0 or 1.
		one_byte_integer value_{};
	};

	/**
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901093
	 */
	struct shared_subscription_available
		: public basic_property<property_type::shared_subscription_available>
		, public property_ops<shared_subscription_available>
	{
		friend struct property_ops<shared_subscription_available>; friend class properties_set;

		shared_subscription_available() = default;
		shared_subscription_available(std::uint8_t v) : value_(v) {}

		inline one_byte_integer::value_type value() const { return value_.value(); }

	protected:
		// Followed by a Byte field.
		// If present, this byte declares whether the Server supports Shared Subscriptions.
		// A value is 0 means that Shared Subscriptions are not supported.
		// A value of 1 means Shared Subscriptions are supported.
		// If not present, then Shared Subscriptions are supported.
		// It is a Protocol Error to include the Shared Subscription Available more than once or to
		// send a value other than 0 or 1.
		one_byte_integer value_{};
	};

	using property_variant = std::variant<
		v5::payload_format_indicator            ,
		v5::message_expiry_interval             ,
		v5::content_type                        ,
		v5::response_topic                      ,
		v5::correlation_data                    ,
		v5::subscription_identifier             ,
		v5::session_expiry_interval             ,
		v5::assigned_client_identifier          ,
		v5::server_keep_alive                   ,
		v5::authentication_method               ,
		v5::authentication_data                 ,
		v5::request_problem_information         ,
		v5::will_delay_interval                 ,
		v5::request_response_information        ,
		v5::response_information                ,
		v5::server_reference                    ,
		v5::reason_string                       ,
		v5::receive_maximum                     ,
		v5::topic_alias_maximum                 ,
		v5::topic_alias                         ,
		v5::maximum_qos                         ,
		v5::retain_available                    ,
		v5::user_property                       ,
		v5::maximum_packet_size                 ,
		v5::wildcard_subscription_available     ,
		v5::subscription_identifier_available   ,
		v5::shared_subscription_available       
	>;

	template<typename T>
	inline constexpr bool is_v5_property() noexcept
	{
		using type = asio2::detail::remove_cvref_t<T>;
		if constexpr (
			std::is_same_v<type, v5::payload_format_indicator            > ||
			std::is_same_v<type, v5::message_expiry_interval             > ||
			std::is_same_v<type, v5::content_type                        > ||
			std::is_same_v<type, v5::response_topic                      > ||
			std::is_same_v<type, v5::correlation_data                    > ||
			std::is_same_v<type, v5::subscription_identifier             > ||
			std::is_same_v<type, v5::session_expiry_interval             > ||
			std::is_same_v<type, v5::assigned_client_identifier          > ||
			std::is_same_v<type, v5::server_keep_alive                   > ||
			std::is_same_v<type, v5::authentication_method               > ||
			std::is_same_v<type, v5::authentication_data                 > ||
			std::is_same_v<type, v5::request_problem_information         > ||
			std::is_same_v<type, v5::will_delay_interval                 > ||
			std::is_same_v<type, v5::request_response_information        > ||
			std::is_same_v<type, v5::response_information                > ||
			std::is_same_v<type, v5::server_reference                    > ||
			std::is_same_v<type, v5::reason_string                       > ||
			std::is_same_v<type, v5::receive_maximum                     > ||
			std::is_same_v<type, v5::topic_alias_maximum                 > ||
			std::is_same_v<type, v5::topic_alias                         > ||
			std::is_same_v<type, v5::maximum_qos                         > ||
			std::is_same_v<type, v5::retain_available                    > ||
			std::is_same_v<type, v5::user_property                       > ||
			std::is_same_v<type, v5::maximum_packet_size                 > ||
			std::is_same_v<type, v5::wildcard_subscription_available     > ||
			std::is_same_v<type, v5::subscription_identifier_available   > ||
			std::is_same_v<type, v5::shared_subscription_available       > 
			)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	class property : public property_variant
	{
	public:
		using super = property_variant;

		property()
		{
		}

		template<class T, std::enable_if_t<is_v5_property<T>(), int> = 0>
		property(T&& v) : property_variant(std::forward<T>(v))
		{
		}

		template<class T, std::enable_if_t<is_v5_property<T>(), int> = 0>
		property& operator=(T&& v)
		{
			this->base() = std::forward<T>(v);
			return (*this);
		}

		property(property&&) noexcept = default;
		property(property const&) = default;
		property& operator=(property&&) noexcept = default;
		property& operator=(property const&) = default;

		template<class T, std::enable_if_t<is_v5_property<T>(), int> = 0>
		operator T&()
		{
			return std::get<T>(this->base());
		}

		template<class T, std::enable_if_t<is_v5_property<T>(), int> = 0>
		operator const T&()
		{
			return std::get<T>(this->base());
		}

		template<class T, std::enable_if_t<is_v5_property<T>(), int> = 0>
		operator const T&() const
		{
			return std::get<T>(this->base());
		}

		template<class T, std::enable_if_t<is_v5_property<T>(), int> = 0>
		operator T*() noexcept
		{
			return std::get_if<T>(std::addressof(this->base()));
		}

		template<class T, std::enable_if_t<is_v5_property<T>(), int> = 0>
		operator const T*() noexcept
		{
			return std::get_if<T>(std::addressof(this->base()));
		}

		template<class T, std::enable_if_t<is_v5_property<T>(), int> = 0>
		operator const T*() const noexcept
		{
			return std::get_if<T>(std::addressof(this->base()));
		}

		// this overload will cause compile error on gcc, see mqtt/message.hpp   operator T()
		//template<class T, std::enable_if_t<is_v5_property<T>(), int> = 0>
		//operator T()
		//{
		//	return std::get<T>(this->base());
		//}

		inline variable_byte_integer::value_type id() const
		{
			variable_byte_integer::value_type r = 0;

			if (this->base().index() != std::variant_npos)
			{
				asio2::clear_last_error();
				r = std::visit([](auto& prop) mutable { return prop.id(); }, this->base());
			}
			else
			{
				asio2::set_last_error(asio::error::no_data);
			}

			return r;
		}

		inline property_type type() const
		{
			property_type r = static_cast<property_type>(0);

			if (this->base().index() != std::variant_npos)
			{
				asio2::clear_last_error();
				r = std::visit([](auto& prop) mutable { return prop.type(); }, this->base());
			}
			else
			{
				asio2::set_last_error(asio::error::no_data);
			}

			return r;
		}

		inline std::string_view name() const
		{
			std::string_view r{};

			if (this->base().index() != std::variant_npos)
			{
				asio2::clear_last_error();
				r = std::visit([](auto& prop) mutable { return prop.name(); }, this->base());
			}
			else
			{
				asio2::set_last_error(asio::error::no_data);
			}

			return r;
		}

		/// Returns the base variant of the message
		inline super const& base()    const noexcept { return *this; }

		/// Returns the base variant of the message
		inline super&       base()          noexcept { return *this; }

		/// Returns the base variant of the message
		inline super const& variant() const noexcept { return *this; }

		/// Returns the base variant of the message
		inline super&       variant()       noexcept { return *this; }

		/**
		 * @brief Checks if the variant holds anyone of the alternative Types...
		 */
		template<class... Types>
		inline bool has() const noexcept
		{
			return (std::holds_alternative<Types>(this->base()) || ...);
		}

		/**
		 * @brief Checks if the variant holds anyone of the alternative Types...
		 */
		template<class... Types>
		inline bool holds() const noexcept
		{
			return (std::holds_alternative<Types>(this->base()) || ...);
		}

		/**
		 * @brief If this holds the alternative T, returns a pointer to the value stored in the variant.
		 * Otherwise, returns a null pointer value.
		 */
		template<class T>
		inline std::add_pointer_t<T> get_if() noexcept
		{
			return std::get_if<T>(std::addressof(this->base()));
		}

		/**
		 * @brief If this holds the alternative T, returns a pointer to the value stored in the variant.
		 * Otherwise, returns a null pointer value.
		 */
		template<class T>
		inline std::add_pointer_t<std::add_const_t<T>> get_if() const noexcept
		{
			return std::get_if<T>(std::addressof(this->base()));
		}

		/**
		 * @brief If this holds the alternative T, returns a reference to the value stored in the variant.
		 * Otherwise, throws std::bad_variant_access.
		 */
		template<class T>
		inline T& get()
		{
			return std::get<T>(this->base());
		}

		/**
		 * @brief If this holds the alternative T, returns a reference to the value stored in the variant.
		 * Otherwise, throws std::bad_variant_access.
		 */
		template<class T>
		inline T const& get() const
		{
			return std::get<T>(this->base());
		}

	protected:
	};

	template<class... Args>
	static constexpr bool is_property() noexcept
	{
		if constexpr (sizeof...(Args) == std::size_t(0))
			return false;
		else
			return ((
				std::is_same_v<asio2::detail::remove_cvref_t<Args>, v5::property> ||
				is_v5_property<asio2::detail::remove_cvref_t<Args>>()) && ...);
	}

	/**
	 * The set of Properties is composed of a Property Length followed by the Properties.
	 * 
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901027
	 */
	class properties_set
	{
	public:
		properties_set() = default;

		template<class... Properties, std::enable_if_t<is_property<Properties...>(), int> = 0>
		explicit properties_set(Properties&&... Props)
		{
			set(std::forward<Properties>(Props)...);
		}

		properties_set(properties_set&&) noexcept = default;
		properties_set(properties_set const&) = default;
		properties_set& operator=(properties_set&&) noexcept = default;
		properties_set& operator=(properties_set const&) = default;

		template<class... Properties, std::enable_if_t<is_property<Properties...>(), int> = 0>
		inline properties_set& set(Properties&&... Props)
		{
			data_.clear();

			(data_.emplace_back(std::forward<Properties>(Props)), ...);

			update_length();

			return (*this);
		}

		template<class... Properties, std::enable_if_t<is_property<Properties...>(), int> = 0>
		inline properties_set& add(Properties&&... Props)
		{
			(data_.emplace_back(std::forward<Properties>(Props)), ...);

			update_length();

			return (*this);
		}

		template<class Propertie, std::enable_if_t<is_property<Propertie>(), int> = 0>
		inline properties_set& erase(Propertie&& Prop)
		{
			for (auto it = data_.begin(); it != data_.end();)
			{
				std::visit([this, &it, &Prop](auto&& prop) mutable
				{
					asio2::detail::ignore_unused(this, it, Prop);

					using T1 = std::decay_t<decltype(Prop)>;
					using T2 = std::decay_t<decltype(prop)>;
					if constexpr (std::is_same_v<T1, T2>)
						it = data_.erase(it);
					else
						++it;
				}, (*it).base());
			}

			update_length();

			return (*this);
		}

		template<class Propertie, std::enable_if_t<is_property<Propertie>(), int> = 0>
		inline properties_set& erase()
		{
			for (auto it = data_.begin(); it != data_.end();)
			{
				std::visit([this, &it](auto&& prop) mutable
				{
					using T1 = std::decay_t<Propertie>;
					using T2 = std::decay_t<decltype(prop)>;
					if constexpr (std::is_same_v<T1, T2>)
						it = data_.erase(it);
					else
						++it;
				}, (*it).base());
			}

			update_length();

			return (*this);
		}

		inline properties_set& clear() noexcept
		{
			data_.clear();

			update_length();

			return (*this);
		}

		inline std::size_t required_size() const
		{
			return (length_.required_size() + length_.value());
		}

		inline std::size_t count() const noexcept
		{
			return data_.size();
		}

		/**
		 * @brief Checks if the properties holds the alternative property T.
		 */
		template<class T>
		inline bool has() const noexcept
		{
			for (auto& v : data_)
			{
				if (std::holds_alternative<T>(v))
					return true;
			}
			return false;
		}

		template<class T>
		inline std::add_pointer_t<T> get_if() noexcept
		{
			for (auto& v : data_)
			{
				if (auto pval = std::get_if<T>(std::addressof(v)))
					return pval;
			}
			return nullptr;
		}

		template<class T>
		inline std::add_pointer_t<std::add_const_t<T>> get_if() const noexcept
		{
			for (auto& v : data_)
			{
				if (auto pval = std::get_if<T>(std::addressof(v)))
					return pval;
			}
			return nullptr;
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline properties_set& serialize(Container& buffer)
		{
			update_length();

			length_.serialize(buffer);

			for (auto& v : data_)
			{
				std::visit([&buffer](auto& prop) mutable { prop.serialize(buffer); }, v.base());
			}

			return (*this);
		}

		inline properties_set& deserialize(std::string_view& data)
		{
			asio2::clear_last_error();

			length_.deserialize(data);

			if (asio2::get_last_error())
			{
				asio2::set_last_error(mqtt::make_error_code(mqtt::error::malformed_packet));
				return (*this);
			}

			std::int32_t length = length_.value();

			std::string_view props_data = data.substr(0, length);

			data.remove_prefix(length);

			while (!props_data.empty())
			{
				variable_byte_integer id{};

				id.deserialize(props_data);

				if (asio2::get_last_error())
				{
					asio2::set_last_error(mqtt::make_error_code(mqtt::error::malformed_packet));
					return (*this);
				}

				// It is a Protocol Error to include the Session Expiry Interval more than once.
				for (auto& prop : data_)
				{
					if (prop.id() == id)
					{
						asio2::set_last_error(mqtt::make_error_code(mqtt::error::protocol_error));
						break;
					}
				}

				switch (static_cast<property_type>(id.value()))
				{
				case property_type::payload_format_indicator          : data_.emplace_back(payload_format_indicator         {}); break;
				case property_type::message_expiry_interval           : data_.emplace_back(message_expiry_interval          {}); break;
				case property_type::content_type                      : data_.emplace_back(content_type                     {}); break;
				case property_type::response_topic                    : data_.emplace_back(response_topic                   {}); break;
				case property_type::correlation_data                  : data_.emplace_back(correlation_data                 {}); break;
				case property_type::subscription_identifier           : data_.emplace_back(subscription_identifier          {}); break;
				case property_type::session_expiry_interval           : data_.emplace_back(session_expiry_interval          {}); break;
				case property_type::assigned_client_identifier        : data_.emplace_back(assigned_client_identifier       {}); break;
				case property_type::server_keep_alive                 : data_.emplace_back(server_keep_alive                {}); break;
				case property_type::authentication_method             : data_.emplace_back(authentication_method            {}); break;
				case property_type::authentication_data               : data_.emplace_back(authentication_data              {}); break;
				case property_type::request_problem_information       : data_.emplace_back(request_problem_information      {}); break;
				case property_type::will_delay_interval               : data_.emplace_back(will_delay_interval              {}); break;
				case property_type::request_response_information      : data_.emplace_back(request_response_information     {}); break;
				case property_type::response_information              : data_.emplace_back(response_information             {}); break;
				case property_type::server_reference                  : data_.emplace_back(server_reference                 {}); break;
				case property_type::reason_string                     : data_.emplace_back(reason_string                    {}); break;
				case property_type::receive_maximum                   : data_.emplace_back(receive_maximum                  {}); break;
				case property_type::topic_alias_maximum               : data_.emplace_back(topic_alias_maximum              {}); break;
				case property_type::topic_alias                       : data_.emplace_back(topic_alias                      {}); break;
				case property_type::maximum_qos                       : data_.emplace_back(maximum_qos                      {}); break;
				case property_type::retain_available                  : data_.emplace_back(retain_available                 {}); break;
				case property_type::user_property                     : data_.emplace_back(user_property                    {}); break;
				case property_type::maximum_packet_size               : data_.emplace_back(maximum_packet_size              {}); break;
				case property_type::wildcard_subscription_available   : data_.emplace_back(wildcard_subscription_available  {}); break;
				case property_type::subscription_identifier_available : data_.emplace_back(subscription_identifier_available{}); break;
				case property_type::shared_subscription_available     : data_.emplace_back(shared_subscription_available    {}); break;
				default:
					// A Control Packet which contains an Identifier which is not valid for its packet type,
					// or contains a value not of the specified data type, is a Malformed Packet. If received,
					// use a CONNACK or DISCONNECT packet with Reason Code 0x81 (Malformed Packet) as described
					// in section 4.13 Handling errors.
					asio2::set_last_error(mqtt::make_error_code(mqtt::error::malformed_packet));
					return (*this);
				}

				std::visit([&props_data](auto& prop) mutable
				{
					prop.value_.deserialize(props_data);
				}, data_.back().base());

				// aboved deserialize maybe failed.
				if (asio2::get_last_error())
				{
					asio2::set_last_error(mqtt::make_error_code(mqtt::error::malformed_packet));
					return (*this);
				}
			}

			return (*this);
		}

		inline std::vector<property>& data() { return data_; }
		inline std::vector<property> const& data() const { return data_; }

		inline properties_set& update_length()
		{
			std::size_t size = 0;
			for (auto& v : data_)
			{
				size += std::visit([](auto& prop) mutable { return prop.required_size(); }, v.base());
			}

			length_ = static_cast<std::int32_t>(size);

			return (*this);
		}

		/**
		 * function signature : void(auto& prop)
		 */
		template<class Function>
		inline properties_set& for_each(Function&& f)
		{
			for (auto& v : data_)
			{
				std::visit([&f](auto& prop) mutable { f(prop); }, v.base());
			}

			return (*this);
		}

	protected:
		// The Property Length is encoded as a Variable Byte Integer.
		// The Property Length does not include the bytes used to encode itself, but includes the
		// length of the Properties.
		// If there are no properties, this MUST be indicated by including a Property Length of zero [MQTT-2.2.2-1].
		variable_byte_integer         length_{};

		// propertie list
		std::vector<property>         data_ {};
	};

	namespace detail
	{
		template<class derived_t>
		struct reason_string_ops
		{
			inline bool has_reason_string() const
			{
				derived_t& derive = static_cast<derived_t&>(*this);

				return derive.properties().template has<v5::reason_string>();
			}

			inline std::string_view reason_string() const
			{
				derived_t& derive = static_cast<derived_t&>(*this);

				v5::reason_string* prs = derive.properties().template get_if<v5::reason_string>();

				return (prs ? prs->value() : std::string_view{});
			}
		};
	}

	/**
	 * CONNECT - Connection Request
	 * 
	 * After a Network Connection is established by a Client to a Server, the first packet sent from the
	 * Client to the Server MUST be a CONNECT packet [MQTT-3.1.0-1].
	 * 
	 * A Client can only send the CONNECT packet once over a Network Connection. The Server MUST process
	 * a second CONNECT packet sent from a Client as a Protocol Error and close the Network Connection [MQTT-3.1.0-2].
	 * 
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901033
	 */
	class connect : public fixed_header<version_number>
	{
	public:
		connect() : fixed_header(control_packet_type::connect)
		{
			update_remain_length();
		}

		connect(utf8_string::value_type clientid) : fixed_header(control_packet_type::connect)
		{
			client_id(std::move(clientid));
			update_remain_length();
		}

		inline std::size_t required_size() const
		{
			return (fixed_header::required_size() + fixed_header::remain_length());
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline connect& serialize(Container& buffer)
		{
			update_remain_length();

			fixed_header::serialize(buffer);

			protocol_name_                   .serialize(buffer);
			protocol_version_                .serialize(buffer);
			connect_flags_.byte              .serialize(buffer);
			keep_alive_                      .serialize(buffer);
			properties_                      .serialize(buffer);
			client_id_                       .serialize(buffer);
			if (will_props_  ) will_props_  ->serialize(buffer);
			if (will_topic_  ) will_topic_  ->serialize(buffer);
			if (will_payload_) will_payload_->serialize(buffer);
			if (username_    ) username_    ->serialize(buffer);
			if (password_    ) password_    ->serialize(buffer);

			return (*this);
		}

		inline connect& deserialize(std::string_view& data)
		{
			fixed_header::deserialize(data);

			protocol_name_          .deserialize(data);
			protocol_version_       .deserialize(data);
			connect_flags_.byte     .deserialize(data);
			keep_alive_             .deserialize(data);
			properties_             .deserialize(data);
			client_id_              .deserialize(data);

			if (has_will())
			{
				properties_set will_props{};
				will_props.deserialize(data);
				will_props_ = std::move(will_props);

				utf8_string will_topic{};
				will_topic.deserialize(data);
				will_topic_ = std::move(will_topic);

				binary_data will_payload{};
				will_payload.deserialize(data);
				will_payload_ = std::move(will_payload);
			}

			if (has_username())
			{
				utf8_string username{};
				username.deserialize(data);
				username_ = std::move(username);
			}

			if (has_password())
			{
				binary_data password{};
				password.deserialize(data);
				password_ = std::move(password);
			}

			update_remain_length();

			return (*this);
		}

		inline std::uint8_t protocol_version() const { return                      (protocol_version_                ); }
		inline bool         clean_start     () const { return                      (connect_flags_.bits.clean_start  ); }
		inline bool         clean_session   () const { return                      (connect_flags_.bits.clean_start  ); }
		inline bool         has_will        () const { return                      (connect_flags_.bits.will_flag    ); }
		inline qos_type     will_qos        () const { return static_cast<qos_type>(connect_flags_.bits.will_qos     ); }
		inline bool         will_retain     () const { return                      (connect_flags_.bits.will_retain  ); }
		inline bool         has_password    () const { return                      (connect_flags_.bits.password_flag); }
		inline bool         has_username    () const { return                      (connect_flags_.bits.username_flag); }

		inline connect&     clean_start     (bool v) { connect_flags_.bits.clean_start = v; return (*this); }
		inline connect&     clean_session   (bool v) { connect_flags_.bits.clean_start = v; return (*this); }

		inline two_byte_integer::value_type keep_alive        () const { return keep_alive_   .value()    ; }
		inline properties_set&              properties        ()       { return properties_               ; }
		inline properties_set const&        properties        () const { return properties_               ; }
		inline utf8_string::view_type       client_id         () const { return client_id_    .data_view(); }
		inline properties_set&              will_properties   ()       { return will_props_   .value()    ; }
		inline properties_set const&        will_properties   () const { return will_props_   .value()    ; }
		inline utf8_string::view_type       will_topic        () const { return will_topic_   ? will_topic_  ->data_view() : ""; }
		inline binary_data::view_type       will_payload      () const { return will_payload_ ? will_payload_->data_view() : ""; }
		inline utf8_string::view_type       username          () const { return username_     ? username_    ->data_view() : ""; }
		inline binary_data::view_type       password          () const { return password_     ? password_    ->data_view() : ""; }

		inline connect& keep_alive(two_byte_integer::value_type   v)
		{
			keep_alive_ = std::move(v);
			return (*this);
		}
		template<class String>
		inline connect& client_id(String&&  v)
		{
			client_id_ = std::forward<String>(v);
			update_remain_length();
			return (*this);
		}
		template<class String>
		inline connect& username(String&&   v)
		{
			username_ = std::forward<String>(v);
			connect_flags_.bits.username_flag = true;
			update_remain_length();
			return (*this);
		}

		inline connect& password(binary_data::value_type   v)
		{
			password_ = std::move(v);
			connect_flags_.bits.password_flag = true;
			update_remain_length();
			return (*this);
		}
		template<class... Properties>
		inline connect& properties(Properties&&... Props)
		{
			properties_.set(std::forward<Properties>(Props)...);
			update_remain_length();
			return (*this);
		}
		template<class String1, class String2, class QosOrInt, class... Properties>
		inline connect& will_attributes(String1&& topic, String2&& payload, QosOrInt qos,
			bool retain, Properties&&... Props)
		{
			will_props_   = properties_set{ std::forward<Properties>(Props)... };
			will_topic_   = std::forward<String1>(topic);
			will_payload_ = std::forward<String2>(payload);
			connect_flags_.bits.will_flag   = true;
			connect_flags_.bits.will_qos    = static_cast<std::uint8_t>(qos);
			connect_flags_.bits.will_retain = retain;
			update_remain_length();
			return (*this);
		}

		inline bool will_topic_has_value() const noexcept { return will_props_.has_value(); }
		inline bool username_has_value  () const noexcept { return username_  .has_value(); }
		inline bool password_has_value  () const noexcept { return password_  .has_value(); }

		inline connect& update_remain_length()
		{
			remain_length_ = static_cast<std::int32_t>(0
				+ protocol_name_                 .required_size()
				+ protocol_version_              .required_size()
				+ connect_flags_.byte            .required_size()
				+ keep_alive_                    .required_size()
				+ properties_                    .required_size()
				+ client_id_                     .required_size()
				+ (will_props_   ? will_props_  ->required_size() : 0)
				+ (will_topic_   ? will_topic_  ->required_size() : 0)
				+ (will_payload_ ? will_payload_->required_size() : 0)
				+ (username_     ? username_    ->required_size() : 0)
				+ (password_     ? password_    ->required_size() : 0)
				);

			return (*this);
		}

	protected:
		// The Protocol Name is a UTF-8 Encoded String that represents the protocol name "MQTT". 
		// The string, its offset and length will not be changed by future versions of the MQTT specification.
		utf8_string      protocol_name_{ "MQTT" };

		// The one byte unsigned value that represents the revision level of the protocol used by the Client.
		// The value of the Protocol Version field for version 5.0 of the protocol is 5 (0x05).
		one_byte_integer protocol_version_{ 0x05 };

		union
		{
			one_byte_integer byte{ 0 };	// all connect flags
		#if ASIO2_ENDIAN_BIG_BYTE
			struct
			{
				bool         username_flag : 1; // User Name Flag
				bool         password_flag : 1; // Password Flag
				bool         will_retain   : 1; // will retain setting
				std::uint8_t will_qos      : 2; // will QoS value
				bool         will_flag     : 1; // will flag
				bool         clean_start   : 1; // Clean Start flag
				std::uint8_t reserved      : 1; // unused
			} bits;
		#else
			struct
			{
				std::uint8_t reserved      : 1; // unused
				bool         clean_start   : 1; // Clean Start flag
				bool         will_flag     : 1; // will flag
				std::uint8_t will_qos      : 2; // will QoS value
				bool         will_retain   : 1; // will retain setting
				bool         password_flag : 1; // Password Flag
				bool         username_flag : 1; // User Name Flag
			} bits;
		#endif
		} connect_flags_{};                     // connect flags byte

		// The Keep Alive is a Two Byte Integer which is a time interval measured in seconds.
		// Default to 60 seconds
		two_byte_integer               keep_alive_  { 60 };

		// CONNECT Properties
		properties_set                 properties_  {};

		// The Client Identifier (ClientID) identifies the Client to the Server. 
		// Each Client connecting to the Server has a unique ClientID. 
		utf8_string                    client_id_   {};

		// If the Will Flag is set to 1, the Will Properties is the next field in the Payload. 
		std::optional<properties_set>  will_props_  {};

		// If the Will Flag is set to 1, the Will Topic is the next field in the Payload.
		std::optional<utf8_string>     will_topic_  {};

		// If the Will Flag is set to 1 the Will Payload is the next field in the Payload. 
		std::optional<binary_data>     will_payload_{};

		// If the User Name Flag is set to 1, the User Name is the next field in the Payload.
		std::optional<utf8_string>     username_    {};

		// If the Password Flag is set to 1, the Password is the next field in the Payload.
		std::optional<binary_data>     password_    {};
	};

	/**
	 * CONNACK - Connect acknowledgement
	 * 
	 * The CONNACK packet is the packet sent by the Server in response to a CONNECT packet received from a Client.
	 * The Server MUST send a CONNACK with a 0x00 (Success) Reason Code before sending any Packet other than AUTH [MQTT-3.2.0-1].
	 * The Server MUST NOT send more than one CONNACK in a Network Connection [MQTT-3.2.0-2].
	 * If the Client does not receive a CONNACK packet from the Server within a reasonable amount of time,
	 * the Client SHOULD close the Network Connection.
	 * A "reasonable" amount of time depends on the type of application and the communications infrastructure.
	 * 
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901074
	 */
	class connack : public fixed_header<version_number>, public detail::reason_string_ops<connack>
	{
	public:
		connack() : fixed_header(control_packet_type::connack)
		{
			update_remain_length();
		}

		template<class... Properties, std::enable_if_t<is_property<Properties...>(), int> = 0>
		explicit connack(bool session_present, std::uint8_t reason_code, Properties&&... Props)
			: fixed_header(control_packet_type::connack)
			, reason_code_(reason_code)
			, properties_(std::forward<Properties>(Props)...)
		{
			connack_flags_.bits.session_present = session_present;
			update_remain_length();
		}

		inline std::size_t required_size() const
		{
			return (fixed_header::required_size() + fixed_header::remain_length());
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline connack& serialize(Container& buffer)
		{
			update_remain_length();

			fixed_header::serialize(buffer);

			connack_flags_.byte      .serialize(buffer);
			reason_code_             .serialize(buffer);
			properties_              .serialize(buffer);

			return (*this);
		}

		inline connack& deserialize(std::string_view& data)
		{
			fixed_header::deserialize(data);

			connack_flags_.byte     .deserialize(data);
			reason_code_            .deserialize(data);
			properties_             .deserialize(data);

			update_remain_length();

			return (*this);
		}

		inline bool            session_present() const { return connack_flags_.bits.session_present; }
		inline mqtt::error     reason_code    () const { return static_cast<mqtt::error>(reason_code_.value()); }
		inline properties_set& properties     ()       { return properties_                        ; }
		inline properties_set const& properties() const { return properties_                        ; }

		inline connack       & session_present(bool         v) { connack_flags_.bits.session_present = v; return (*this); }
		inline connack       & reason_code    (std::uint8_t v) { reason_code_                        = v; return (*this); }
		inline connack       & reason_code    (mqtt::error  v)
		{ reason_code_ = asio2::detail::to_underlying(v); return (*this); }
		template<class... Properties>
		inline connack       & properties(Properties&&... Props)
		{
			properties_.set(std::forward<Properties>(Props)...);
			update_remain_length();
			return (*this);
		}

		inline connack& update_remain_length()
		{
			remain_length_ = static_cast<std::int32_t>(0
				+ connack_flags_.byte    .required_size()
				+ reason_code_           .required_size()
				+ properties_            .required_size()
				);

			return (*this);
		}
	protected:
		union
		{
			one_byte_integer byte{ 0 }; // all connack flags
		#if ASIO2_ENDIAN_BIG_BYTE
			struct
			{
				std::uint8_t reserved : 7;
				bool  session_present : 1; // session found on the server?
			} bits;
		#else
			struct
			{
				bool  session_present : 1; // session found on the server?
				std::uint8_t reserved : 7;
			} bits;
		#endif
		} connack_flags_{};	               // connack flags

		// Byte 2 in the Variable Header is the Connect Reason Code.
		one_byte_integer reason_code_{ 0 };
		
		// CONNACK Properties
		properties_set   properties_ {   };
	};

	/**
	 * PUBLISH - Publish message
	 * 
	 * A PUBLISH packet is sent from a Client to a Server or from a Server to a Client to transport an Application Message.
	 * 
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901100
	 */
	class publish : public fixed_header<version_number>
	{
	public:
		publish() : fixed_header(control_packet_type::publish)
		{
			update_remain_length();
		}

		template<class String1, class String2, class QosOrInt, std::enable_if_t<
			asio2::detail::is_character_string_v<String1>, int> = 0>
		explicit publish(String1&& topic_name, String2&& payload, QosOrInt qos,
			bool dup = false, bool retain = false)
			: fixed_header(control_packet_type::publish)
			, topic_name_ (std::forward<String1>(topic_name))
			, payload_    (std::forward<String2>(payload   ))
		{
			type_and_flags_.bits.dup    = dup;
			type_and_flags_.bits.qos    = static_cast<std::uint8_t>(qos);
			type_and_flags_.bits.retain = retain;

			update_remain_length();
		}

		template<class String1, class String2, class QosOrInt, std::enable_if_t<
			asio2::detail::is_character_string_v<String1>, int> = 0>
		explicit publish(std::uint16_t pid, String1&& topic_name, String2&& payload, QosOrInt qos,
			bool dup = false, bool retain = false)
			: fixed_header(control_packet_type::publish)
			, topic_name_ (std::forward<String1>(topic_name))
			, packet_id_  (pid)
			, payload_    (std::forward<String2>(payload   ))
		{
			type_and_flags_.bits.dup    = dup;
			type_and_flags_.bits.qos    = static_cast<std::uint8_t>(qos);
			type_and_flags_.bits.retain = retain;

			ASIO2_ASSERT(type_and_flags_.bits.qos > std::uint8_t(0));

			update_remain_length();
		}

		inline std::size_t required_size() const
		{
			return (fixed_header::required_size() + fixed_header::remain_length());
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline publish& serialize(Container& buffer)
		{
			update_remain_length();

			fixed_header::serialize(buffer);

			// The Packet Identifier field is only present in PUBLISH packets where the QoS level is 1 or 2.
			// A PUBLISH packet MUST NOT contain a Packet Identifier if its QoS value is set to 0
			if ((type_and_flags_.bits.qos == std::uint8_t(0) &&  packet_id_.has_value()) ||
				(type_and_flags_.bits.qos >  std::uint8_t(0) && !packet_id_.has_value()))
			{
				ASIO2_ASSERT(false);
				asio2::set_last_error(mqtt::make_error_code(mqtt::error::malformed_packet));
			}

			topic_name_.serialize(buffer);
			if (type_and_flags_.bits.qos > std::uint8_t(0) && packet_id_.has_value())
			{
				packet_id_->serialize(buffer);
			}
			properties_.serialize(buffer);
			payload_   .serialize(buffer);

			return (*this);
		}

		inline publish& deserialize(std::string_view& data)
		{
			fixed_header::deserialize(data);

			topic_name_.deserialize(data);
			if (type_and_flags_.bits.qos == 1 || type_and_flags_.bits.qos == 2)
			{
				two_byte_integer packet_id{};
				packet_id.deserialize(data);
				packet_id_ = packet_id;
			}
			properties_.deserialize(data);
			payload_   .deserialize(data);

			update_remain_length();

			return (*this);
		}

		inline bool                dup   () const { return                      (type_and_flags_.bits.dup   ); }
		inline qos_type            qos   () const { return static_cast<qos_type>(type_and_flags_.bits.qos   ); }
		inline bool                retain() const { return                      (type_and_flags_.bits.retain); }

		inline publish       &     dup   (bool     v) { type_and_flags_.bits.dup    = v;                            return (*this); }
		template<class QosOrInt>
		inline publish       &     qos   (QosOrInt v) { type_and_flags_.bits.qos    = static_cast<std::uint8_t>(v); return (*this); }
		inline publish       &     retain(bool     v) { type_and_flags_.bits.retain = v;                            return (*this); }

		inline utf8_string::view_type          topic_name() const { return topic_name_.data_view(); }
		inline two_byte_integer::value_type    packet_id () const { return packet_id_ ? packet_id_->value() : 0; }
		inline properties_set&                 properties()       { return properties_            ; }
		inline properties_set const&           properties() const { return properties_            ; }
		inline application_message::view_type  payload   () const { return payload_.data_view()   ; }

		inline publish       &  packet_id (std::uint16_t    v) { packet_id_  = v                     ;  update_remain_length(); return (*this); }
		template<class String>
		inline publish       &  topic_name(String&&         v) { topic_name_ = std::forward<String>(v); update_remain_length(); return (*this); }
		template<class String>
		inline publish       &  payload   (String&&         v) { payload_    = std::forward<String>(v); update_remain_length(); return (*this); }
		
		template<class... Properties>
		inline publish       &  properties(Properties&&... Props)
		{
			properties_.set(std::forward<Properties>(Props)...);
			update_remain_length();
			return (*this);
		}

		inline bool has_packet_id() const noexcept { return packet_id_.has_value(); }

		inline publish& update_remain_length()
		{
			remain_length_ = static_cast<std::int32_t>(0
				+               topic_name_.required_size()
				+ (packet_id_ ? packet_id_->required_size() : 0)
				+               properties_.required_size()
				+               payload_   .required_size()
				);

			return (*this);
		}
	protected:
		// The Topic Name identifies the information channel to which Payload data is published.
		utf8_string                     topic_name_{};

		// The Packet Identifier field is only present in PUBLISH packets where the QoS level is 1 or 2.
		// a Two Byte Integer Packet Identifier.
		std::optional<two_byte_integer> packet_id_ {};
		
		// PUBLISH Properties
		properties_set                  properties_{};

		// The Payload contains the Application Message that is being published.
		// The content and format of the data is application specific.
		// The length of the Payload can be calculated by subtracting the length of the Variable Header
		// from the Remaining Length field that is in the Fixed Header.
		// It is valid for a PUBLISH packet to contain a zero length Payload.
		application_message             payload_   {};
	};

	/**
	 * PUBACK - Publish acknowledgement
	 * 
	 * A PUBACK packet is the response to a PUBLISH packet with QoS 1.
	 * 
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901121
	 */
	class puback : public fixed_header<version_number>, public detail::reason_string_ops<puback>
	{
	public:
		puback() : fixed_header(control_packet_type::puback)
		{
			update_remain_length();
		}

		template<class... Properties, std::enable_if_t<is_property<Properties...>(), int> = 0>
		explicit puback(std::uint16_t packet_id, std::uint8_t reason_code, Properties&&... Props)
			: fixed_header(control_packet_type::puback)
			, packet_id_  (packet_id)
			, reason_code_(reason_code)
			, properties_ (std::forward<Properties>(Props)...)
		{
			update_remain_length();
		}

		inline std::size_t required_size() const
		{
			return (fixed_header::required_size() + fixed_header::remain_length());
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline puback& serialize(Container& buffer)
		{
			update_remain_length();

			fixed_header::serialize(buffer);

			packet_id_  .serialize(buffer);
			reason_code_.serialize(buffer);
			properties_ .serialize(buffer);

			return (*this);
		}

		inline puback& deserialize(std::string_view& data)
		{
			fixed_header::deserialize(data);

			// The Reason Code and Property Length can be omitted if the Reason Code is 0x00 (Success)
			// and there are no Properties. In this case the PUBACK has a Remaining Length of 2.
			packet_id_  .deserialize(data);
			if (!data.empty())
			{
				reason_code_.deserialize(data);

				// If the Remaining Length is less than 4 there is no Property Length and the value of 0 is used.
				if (!data.empty())
					properties_.deserialize(data);
			}

			update_remain_length();

			return (*this);
		}

		inline two_byte_integer::value_type    packet_id  () const { return packet_id_  .value()   ; }
		inline std::uint8_t                    reason_code() const { return reason_code_.value()   ; }
		inline properties_set&                 properties ()       { return properties_            ; }
		inline properties_set const&           properties () const { return properties_            ; }

		inline puback       &  packet_id  (std::uint16_t v) { packet_id_   = v; return (*this); }
		inline puback       &  reason_code(std::uint8_t  v) { reason_code_ = v; return (*this); }
		template<class... Properties>
		inline puback       &  properties (Properties&&... Props)
		{
			properties_.set(std::forward<Properties>(Props)...);
			update_remain_length();
			return (*this);
		}

		inline puback& update_remain_length()
		{
			remain_length_ = static_cast<std::int32_t>(0
				+ packet_id_  .required_size()
				+ reason_code_.required_size()
				+ properties_ .required_size()
				);

			return (*this);
		}
	protected:
		// The Variable Header of the PUBACK Packet contains the following fields in the order:
		// Packet Identifier from the PUBLISH packet that is being acknowledged, PUBACK Reason Code,
		// Property Length, and the Properties. The rules for encoding Properties are described in section 2.2.2.
		two_byte_integer                packet_id_ {};

		// Byte 3 in the Variable Header is the PUBACK Reason Code. 
		// If the Remaining Length is 2, then there is no Reason Code and the value of 0x00 (Success) is used.
		one_byte_integer                reason_code_{ 0 };
		
		// PUBACK Properties
		properties_set                  properties_{};
	};

	/**
	 * PUBREC - Publish received (QoS 2 delivery part 1)
	 * 
	 * A PUBREC packet is the response to a PUBLISH packet with QoS 2.
	 * It is the second packet of the QoS 2 protocol exchange.
	 * 
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901131
	 */
	class pubrec : public fixed_header<version_number>, public detail::reason_string_ops<pubrec>
	{
	public:
		pubrec() : fixed_header(control_packet_type::pubrec)
		{
			update_remain_length();
		}

		template<class... Properties, std::enable_if_t<is_property<Properties...>(), int> = 0>
		explicit pubrec(std::uint16_t packet_id, std::uint8_t reason_code, Properties&&... Props)
			: fixed_header(control_packet_type::pubrec)
			, packet_id_  (packet_id)
			, reason_code_(reason_code)
			, properties_ (std::forward<Properties>(Props)...)
		{
			update_remain_length();
		}

		inline std::size_t required_size() const
		{
			return (fixed_header::required_size() + fixed_header::remain_length());
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline pubrec& serialize(Container& buffer)
		{
			update_remain_length();

			fixed_header::serialize(buffer);

			packet_id_  .serialize(buffer);
			reason_code_.serialize(buffer);
			properties_ .serialize(buffer);

			return (*this);
		}

		inline pubrec& deserialize(std::string_view& data)
		{
			fixed_header::deserialize(data);

			// The Reason Code and Property Length can be omitted if the Reason Code is 0x00 (Success)
			// and there are no Properties. In this case the PUBREC has a Remaining Length of 2.
			packet_id_  .deserialize(data);
			if (!data.empty())
			{
				reason_code_.deserialize(data);

				// If the Remaining Length is less than 4 there is no Property Length and the value of 0 is used.
				if (!data.empty())
					properties_.deserialize(data);
			}

			update_remain_length();

			return (*this);
		}

		inline two_byte_integer::value_type    packet_id  () const { return packet_id_  .value()   ; }
		inline std::uint8_t                    reason_code() const { return reason_code_.value()   ; }
		inline properties_set&                 properties ()       { return properties_            ; }
		inline properties_set const&           properties () const { return properties_            ; }

		inline pubrec       &  packet_id  (std::uint16_t v) { packet_id_   = v; return (*this); }
		inline pubrec       &  reason_code(std::uint8_t  v) { reason_code_ = v; return (*this); }
		template<class... Properties>
		inline pubrec       &  properties (Properties&&... Props)
		{
			properties_.set(std::forward<Properties>(Props)...);
			update_remain_length();
			return (*this);
		}

		inline pubrec& update_remain_length()
		{
			remain_length_ = static_cast<std::int32_t>(0
				+ packet_id_  .required_size()
				+ reason_code_.required_size()
				+ properties_ .required_size()
				);

			return (*this);
		}
	protected:
		// The Variable Header of the PUBREC Packet consists of the following fields in the order:
		// the Packet Identifier from the PUBLISH packet that is being acknowledged, PUBREC Reason Code,
		// and Properties. The rules for encoding Properties are described in section 2.2.2.
		two_byte_integer                packet_id_ {};

		// Byte 3 in the Variable Header is the PUBREC Reason Code.
		// If the Remaining Length is 2, then the Publish Reason Code has the value 0x00 (Success).
		one_byte_integer                reason_code_{ 0 };
		
		// PUBREC Properties
		properties_set                  properties_{};
	};

	/**
	 * PUBREL - Publish release (QoS 2 delivery part 2)
	 * 
	 * A PUBREL packet is the response to a PUBREC packet.
	 * It is the third packet of the QoS 2 protocol exchange.
	 * 
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901141
	 */
	class pubrel : public fixed_header<version_number>, public detail::reason_string_ops<pubrel>
	{
	public:
		pubrel() : fixed_header(control_packet_type::pubrel)
		{
			// Bits 3,2,1 and 0 of the Fixed Header in the PUBREL packet are reserved and MUST be
			// set to 0,0,1 and 0 respectively.
			// The Server MUST treat any other value as malformed and close the Network Connection [MQTT-3.6.1-1].
			type_and_flags_.reserved.bit1 = 1;

			update_remain_length();
		}

		template<class... Properties, std::enable_if_t<is_property<Properties...>(), int> = 0>
		explicit pubrel(std::uint16_t packet_id, std::uint8_t reason_code, Properties&&... Props)
			: fixed_header(control_packet_type::pubrel)
			, packet_id_  (packet_id)
			, reason_code_(reason_code)
			, properties_ (std::forward<Properties>(Props)...)
		{
			type_and_flags_.reserved.bit1 = 1;

			update_remain_length();
		}

		inline std::size_t required_size() const
		{
			return (fixed_header::required_size() + fixed_header::remain_length());
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline pubrel& serialize(Container& buffer)
		{
			update_remain_length();

			fixed_header::serialize(buffer);

			packet_id_  .serialize(buffer);
			reason_code_.serialize(buffer);
			properties_ .serialize(buffer);

			return (*this);
		}

		inline pubrel& deserialize(std::string_view& data)
		{
			fixed_header::deserialize(data);

			// The Reason Code and Property Length can be omitted if the Reason Code is 0x00 (Success)
			// and there are no Properties. In this case the PUBREL has a Remaining Length of 2.
			packet_id_  .deserialize(data);
			if (!data.empty())
			{
				reason_code_.deserialize(data);

				// If the Remaining Length is less than 4 there is no Property Length and the value of 0 is used.
				if (!data.empty())
					properties_.deserialize(data);
			}

			update_remain_length();

			return (*this);
		}

		inline two_byte_integer::value_type    packet_id  () const { return packet_id_  .value()   ; }
		inline std::uint8_t                    reason_code() const { return reason_code_.value()   ; }
		inline properties_set&                 properties ()       { return properties_            ; }
		inline properties_set const&           properties () const { return properties_            ; }

		inline pubrel       &  packet_id  (std::uint16_t v) { packet_id_   = v; return (*this); }
		inline pubrel       &  reason_code(std::uint8_t  v) { reason_code_ = v; return (*this); }
		template<class... Properties>
		inline pubrel       &  properties (Properties&&... Props)
		{
			properties_.set(std::forward<Properties>(Props)...);
			update_remain_length();
			return (*this);
		}

		inline pubrel& update_remain_length()
		{
			remain_length_ = static_cast<std::int32_t>(0
				+ packet_id_  .required_size()
				+ reason_code_.required_size()
				+ properties_ .required_size()
				);

			return (*this);
		}
	protected:
		// The Variable Header of the PUBREL Packet contains the following fields in the order:
		// the Packet Identifier from the PUBREC packet that is being acknowledged, PUBREL Reason Code,
		// and Properties. The rules for encoding Properties are described in section 2.2.2.
		two_byte_integer                packet_id_ {};

		// Byte 3 in the Variable Header is the PUBREL Reason Code.
		// If the Remaining Length is 2, the value of 0x00 (Success) is used.
		one_byte_integer                reason_code_{ 0 };
		
		// PUBREL Properties
		properties_set                  properties_{};
	};

	/**
	 * PUBCOMP - Publish complete (QoS 2 delivery part 3)
	 * 
	 * The PUBCOMP packet is the response to a PUBREL packet.
	 * It is the fourth and final packet of the QoS 2 protocol exchange.
	 * 
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901151
	 */
	class pubcomp : public fixed_header<version_number>, public detail::reason_string_ops<pubcomp>
	{
	public:
		pubcomp() : fixed_header(control_packet_type::pubcomp)
		{
			update_remain_length();
		}

		template<class... Properties, std::enable_if_t<is_property<Properties...>(), int> = 0>
		explicit pubcomp(std::uint16_t packet_id, std::uint8_t reason_code, Properties&&... Props)
			: fixed_header(control_packet_type::pubcomp)
			, packet_id_  (packet_id)
			, reason_code_(reason_code)
			, properties_ (std::forward<Properties>(Props)...)
		{
			update_remain_length();
		}

		inline std::size_t required_size() const
		{
			return (fixed_header::required_size() + fixed_header::remain_length());
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline pubcomp& serialize(Container& buffer)
		{
			update_remain_length();

			fixed_header::serialize(buffer);

			packet_id_  .serialize(buffer);
			reason_code_.serialize(buffer);
			properties_ .serialize(buffer);

			return (*this);
		}

		inline pubcomp& deserialize(std::string_view& data)
		{
			fixed_header::deserialize(data);

			// The Reason Code and Property Length can be omitted if the Reason Code is 0x00 (Success)
			// and there are no Properties. In this case the PUBCOMP has a Remaining Length of 2.
			packet_id_  .deserialize(data);
			if (!data.empty())
			{
				reason_code_.deserialize(data);

				// If the Remaining Length is less than 4 there is no Property Length and the value of 0 is used.
				if (!data.empty())
					properties_.deserialize(data);
			}

			update_remain_length();

			return (*this);
		}

		inline two_byte_integer::value_type    packet_id  () const { return packet_id_  .value()   ; }
		inline std::uint8_t                    reason_code() const { return reason_code_.value()   ; }
		inline properties_set&                 properties ()       { return properties_            ; }
		inline properties_set const&           properties () const { return properties_            ; }

		inline pubcomp       &  packet_id  (std::uint16_t v) { packet_id_   = v; return (*this); }
		inline pubcomp       &  reason_code(std::uint8_t  v) { reason_code_ = v; return (*this); }
		template<class... Properties>
		inline pubcomp       &  properties (Properties&&... Props)
		{
			properties_.set(std::forward<Properties>(Props)...);
			update_remain_length();
			return (*this);
		}

		inline pubcomp& update_remain_length()
		{
			remain_length_ = static_cast<std::int32_t>(0
				+ packet_id_  .required_size()
				+ reason_code_.required_size()
				+ properties_ .required_size()
				);

			return (*this);
		}
	protected:
		// The Variable Header of the PUBCOMP Packet contains the following fields in the order:
		// Packet Identifier from the PUBREL packet that is being acknowledged, PUBCOMP Reason Code,
		// and Properties. The rules for encoding Properties are described in section 2.2.2.
		two_byte_integer                packet_id_ {};

		// Byte 3 in the Variable Header is the PUBCOMP Reason Code.
		// If the Remaining Length is 2, then the value 0x00 (Success) is used.
		one_byte_integer                reason_code_{ 0 };
		
		// PUBCOMP Properties
		properties_set                  properties_{};
	};

	/**
	 * SUBSCRIBE - Subscribe request
	 * 
	 * The SUBSCRIBE packet is sent from the Client to the Server to create one or more Subscriptions.
	 * Each Subscription registers a Client's interest in one or more Topics.
	 * The Server sends PUBLISH packets to the Client to forward Application Messages that were published
	 * to Topics that match these Subscriptions.
	 * The SUBSCRIBE packet also specifies (for each Subscription) the maximum QoS with which the Server
	 * can send Application Messages to the Client.
	 * 
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901161
	 */
	class subscribe : public fixed_header<version_number>
	{
	public:
		subscribe() : fixed_header(control_packet_type::subscribe)
		{
			// Bits 3,2,1 and 0 of the Fixed Header of the SUBSCRIBE packet are reserved and MUST be
			// set to 0,0,1 and 0 respectively. The Server MUST treat any other value as malformed and
			// close the Network Connection [MQTT-3.8.1-1].
			type_and_flags_.reserved.bit1 = 1;

			update_remain_length();
		}

		template<class... Properties, std::enable_if_t<is_property<Properties...>(), int> = 0>
		explicit subscribe(std::uint16_t packet_id, Properties&&... Props)
			: fixed_header(control_packet_type::subscribe)
			, packet_id_  (packet_id)
			, properties_ (std::forward<Properties>(Props)...)
		{
			type_and_flags_.reserved.bit1 = 1;

			update_remain_length();
		}

		inline std::size_t required_size() const
		{
			return (fixed_header::required_size() + fixed_header::remain_length());
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline subscribe& serialize(Container& buffer)
		{
			update_remain_length();

			fixed_header::serialize(buffer);

			packet_id_    .serialize(buffer);
			properties_   .serialize(buffer);
			subscriptions_.serialize(buffer);

			return (*this);
		}

		inline subscribe& deserialize(std::string_view& data)
		{
			fixed_header::deserialize(data);

			packet_id_    .deserialize(data);
			properties_   .deserialize(data);
			subscriptions_.deserialize(data);

			update_remain_length();

			return (*this);
		}

		inline two_byte_integer::value_type    packet_id    () const { return packet_id_  .value()   ; }
		inline properties_set   &              properties   ()       { return properties_            ; }
		inline subscriptions_set&              subscriptions()       { return subscriptions_         ; }
		inline properties_set    const&        properties   () const { return properties_            ; }
		inline subscriptions_set const&        subscriptions() const { return subscriptions_         ; }

		inline subscribe       &  packet_id  (std::uint16_t v) { packet_id_   = v; return (*this); }
		template<class... Properties>
		inline subscribe       &  properties (Properties&&... Props)
		{
			properties_.set(std::forward<Properties>(Props)...);
			update_remain_length();
			return (*this);
		}

		template<class... Subscriptions>
		inline subscribe& add_subscriptions(Subscriptions&&... Subscripts)
		{
			subscriptions_.add(std::forward<Subscriptions>(Subscripts)...);
			update_remain_length();
			return (*this);
		}

		inline subscribe& erase_subscription(std::string_view topic_filter)
		{
			subscriptions_.erase(topic_filter);
			update_remain_length();
			return (*this);
		}

		inline subscribe& update_remain_length()
		{
			remain_length_ = static_cast<std::int32_t>(0
				+ packet_id_    .required_size()
				+ properties_   .required_size()
				+ subscriptions_.required_size()
				);

			return (*this);
		}
	protected:
		// The Variable Header of the SUBSCRIBE Packet contains the following fields in the order:
		// Packet Identifier, and Properties.
		two_byte_integer                packet_id_    {};
		
		// SUBSCRIBE Properties
		properties_set                  properties_   {};

		// The Payload of a SUBSCRIBE packet contains a list of Topic Filters indicating the Topics
		// to which the Client wants to subscribe. The Topic Filters MUST be a UTF-8 Encoded String
		// [MQTT-3.8.3-1]. Each Topic Filter is followed by a Subscription Options byte.
		subscriptions_set               subscriptions_{};
	};

	/**
	 * SUBACK - Subscribe acknowledgement
	 * 
	 * A SUBACK packet is sent by the Server to the Client to confirm receipt and processing of a SUBSCRIBE packet.
	 * 
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901171
	 */
	class suback : public fixed_header<version_number>
	{
	public:
		suback() : fixed_header(control_packet_type::suback)
		{
			update_remain_length();
		}

		template<class... Properties, std::enable_if_t<is_property<Properties...>(), int> = 0>
		explicit suback(std::uint16_t packet_id, Properties&&... Props)
			: fixed_header(control_packet_type::suback)
			, packet_id_  (packet_id)
			, properties_ (std::forward<Properties>(Props)...)
		{
			update_remain_length();
		}

		inline std::size_t required_size() const
		{
			return (fixed_header::required_size() + fixed_header::remain_length());
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline suback& serialize(Container& buffer)
		{
			update_remain_length();

			fixed_header::serialize(buffer);

			packet_id_    .serialize(buffer);
			properties_   .serialize(buffer);
			reason_codes_ .serialize(buffer);

			return (*this);
		}

		inline suback& deserialize(std::string_view& data)
		{
			fixed_header::deserialize(data);

			packet_id_    .deserialize(data);
			properties_   .deserialize(data);
			reason_codes_ .deserialize(data);

			update_remain_length();

			return (*this);
		}

		inline two_byte_integer::value_type    packet_id    () const { return packet_id_  .value()   ; }
		inline properties_set        &         properties   ()       { return properties_            ; }
		inline one_byte_integer_set  &         reason_codes ()       { return reason_codes_          ; }
		inline properties_set        const&    properties   () const { return properties_            ; }
		inline one_byte_integer_set  const&    reason_codes () const { return reason_codes_          ; }

		inline suback       &  packet_id  (std::uint16_t v) { packet_id_   = v; return (*this); }
		template<class... Properties>
		inline suback       &  properties (Properties&&... Props)
		{
			properties_.set(std::forward<Properties>(Props)...);
			update_remain_length();
			return (*this);
		}

		template<class... Integers>
		inline suback& add_reason_codes(Integers... Ints)
		{
			reason_codes_.add(static_cast<one_byte_integer::value_type>(Ints)...);
			update_remain_length();
			return (*this);
		}

		inline suback& erase_reason_code(std::size_t index)
		{
			reason_codes_.erase(index);
			update_remain_length();
			return (*this);
		}

		inline suback& update_remain_length()
		{
			remain_length_ = static_cast<std::int32_t>(0
				+ packet_id_    .required_size()
				+ properties_   .required_size()
				+ reason_codes_ .required_size()
				);

			return (*this);
		}
	protected:
		// The Variable Header of the SUBACK Packet contains the following fields in the order:
		// the Packet Identifier from the SUBSCRIBE Packet that is being acknowledged, and Properties.
		two_byte_integer                packet_id_   {};
		
		// SUBACK Properties
		properties_set                  properties_  {};

		// The Payload contains a list of Reason Codes. Each Reason Code corresponds to a Topic Filter
		// in the SUBSCRIBE packet being acknowledged. The order of Reason Codes in the SUBACK packet
		// MUST match the order of Topic Filters in the SUBSCRIBE packet [MQTT-3.9.3-1].
		one_byte_integer_set            reason_codes_{};
	};

	/**
	 * UNSUBSCRIBE - Unsubscribe request
	 * 
	 * An UNSUBSCRIBE packet is sent by the Client to the Server, to unsubscribe from topics.
	 * 
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901179
	 */
	class unsubscribe : public fixed_header<version_number>
	{
	public:
		unsubscribe() : fixed_header(control_packet_type::unsubscribe)
		{
			// Bits 3,2,1 and 0 of the Fixed Header of the UNSUBSCRIBE packet are reserved and MUST
			// be set to 0,0,1 and 0 respectively. The Server MUST treat any other value as malformed
			// and close the Network Connection [MQTT-3.10.1-1].
			type_and_flags_.reserved.bit1 = 1;
			
			update_remain_length();
		}

		template<class... Strings>
		explicit unsubscribe(std::uint16_t packet_id, Strings&&... topic_filters)
			: fixed_header  (control_packet_type::unsubscribe)
			, packet_id_    (packet_id)
			, topic_filters_(std::forward<Strings>(topic_filters)...)
		{
			type_and_flags_.reserved.bit1 = 1;

			update_remain_length();
		}

		template<class... Properties, std::enable_if_t<is_property<Properties...>(), int> = 0>
		explicit unsubscribe(std::uint16_t packet_id, Properties&&... Props)
			: fixed_header(control_packet_type::unsubscribe)
			, packet_id_  (packet_id)
			, properties_ (std::forward<Properties>(Props)...)
		{
			type_and_flags_.reserved.bit1 = 1;

			update_remain_length();
		}

		inline std::size_t required_size() const
		{
			return (fixed_header::required_size() + fixed_header::remain_length());
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline unsubscribe& serialize(Container& buffer)
		{
			update_remain_length();

			fixed_header::serialize(buffer);

			packet_id_    .serialize(buffer);
			properties_   .serialize(buffer);
			topic_filters_.serialize(buffer);

			return (*this);
		}

		inline unsubscribe& deserialize(std::string_view& data)
		{
			fixed_header::deserialize(data);

			packet_id_    .deserialize(data);
			properties_   .deserialize(data);
			topic_filters_.deserialize(data);

			update_remain_length();

			return (*this);
		}

		inline two_byte_integer::value_type    packet_id    () const { return packet_id_  .value()   ; }
		inline properties_set   &              properties   ()       { return properties_            ; }
		inline utf8_string_set  &              topic_filters()       { return topic_filters_         ; }
		inline properties_set   const&         properties   () const { return properties_            ; }
		inline utf8_string_set  const&         topic_filters() const { return topic_filters_         ; }

		inline unsubscribe       &  packet_id  (std::uint16_t v) { packet_id_   = v; return (*this); }
		template<class... Properties>
		inline unsubscribe       &  properties (Properties&&... Props)
		{
			properties_.set(std::forward<Properties>(Props)...);
			update_remain_length();
			return (*this);
		}

		template<class... Strings>
		inline unsubscribe& add_topic_filters(Strings&&... Strs)
		{
			topic_filters_.add(std::forward<Strings>(Strs)...);
			update_remain_length();
			return (*this);
		}

		inline unsubscribe& erase_topic_filter(std::string_view topic_filter)
		{
			topic_filters_.erase(topic_filter);
			update_remain_length();
			return (*this);
		}

		inline unsubscribe& update_remain_length()
		{
			remain_length_ = static_cast<std::int32_t>(0
				+ packet_id_    .required_size()
				+ properties_   .required_size()
				+ topic_filters_.required_size()
				);

			return (*this);
		}
	protected:
		// The Variable Header of the UNSUBSCRIBE Packet contains the following fields in the order:
		// Packet Identifier, and Properties.
		two_byte_integer                packet_id_    {};
		
		// UNSUBSCRIBE Properties
		properties_set                  properties_   {};

		// The Payload for the UNSUBSCRIBE packet contains the list of Topic Filters that the Client
		// wishes to unsubscribe from. The Topic Filters in an UNSUBSCRIBE packet MUST be UTF-8 
		// Encoded Strings [MQTT-3.10.3-1] as defined in section 1.5.4, packed contiguously.
		// The Payload of an UNSUBSCRIBE packet MUST contain at least one Topic Filter[MQTT - 3.10.3 - 2].
		// An UNSUBSCRIBE packet with no Payload is a Protocol Error.Refer to section 4.13 for
		// information about handling errors.
		utf8_string_set                 topic_filters_{};
	};

	/**
	 * UNSUBACK - Unsubscribe acknowledgement
	 * 
	 * The UNSUBACK packet is sent by the Server to the Client to confirm receipt of an UNSUBSCRIBE packet.
	 * 
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901187
	 */
	class unsuback : public fixed_header<version_number>
	{
	public:
		unsuback() : fixed_header(control_packet_type::unsuback)
		{
			update_remain_length();
		}

		template<class... Properties, std::enable_if_t<is_property<Properties...>(), int> = 0>
		explicit unsuback(std::uint16_t packet_id, Properties&&... Props)
			: fixed_header(control_packet_type::unsuback)
			, packet_id_  (packet_id)
			, properties_ (std::forward<Properties>(Props)...)
		{
			update_remain_length();
		}

		inline std::size_t required_size() const
		{
			return (fixed_header::required_size() + fixed_header::remain_length());
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline unsuback& serialize(Container& buffer)
		{
			update_remain_length();

			fixed_header::serialize(buffer);

			packet_id_    .serialize(buffer);
			properties_   .serialize(buffer);
			reason_codes_ .serialize(buffer);

			return (*this);
		}

		inline unsuback& deserialize(std::string_view& data)
		{
			fixed_header::deserialize(data);

			packet_id_    .deserialize(data);
			properties_   .deserialize(data);
			reason_codes_ .deserialize(data);

			update_remain_length();

			return (*this);
		}

		inline two_byte_integer::value_type    packet_id    () const { return packet_id_  .value()   ; }
		inline properties_set        &         properties   ()       { return properties_            ; }
		inline one_byte_integer_set  &         reason_codes ()       { return reason_codes_          ; }
		inline properties_set        const&    properties   () const { return properties_            ; }
		inline one_byte_integer_set  const&    reason_codes () const { return reason_codes_          ; }

		inline unsuback       &  packet_id  (std::uint16_t v) { packet_id_   = v; return (*this); }
		template<class... Properties>
		inline unsuback       &  properties (Properties&&... Props)
		{
			properties_.set(std::forward<Properties>(Props)...);
			update_remain_length();
			return (*this);
		}

		template<class... Integers>
		inline unsuback& add_reason_codes(Integers... Ints)
		{
			reason_codes_.add(static_cast<one_byte_integer::value_type>(Ints)...);
			update_remain_length();
			return (*this);
		}

		inline unsuback& erase_reason_code(std::size_t index)
		{
			reason_codes_.erase(index);
			update_remain_length();
			return (*this);
		}

		inline unsuback& update_remain_length()
		{
			remain_length_ = static_cast<std::int32_t>(0
				+ packet_id_    .required_size()
				+ properties_   .required_size()
				+ reason_codes_ .required_size()
				);

			return (*this);
		}
	protected:
		// The Variable Header of the UNSUBACK Packet the following fields in the order:
		// the Packet Identifier from the UNSUBSCRIBE Packet that is being acknowledged, and Properties.
		two_byte_integer                packet_id_   {};
		
		// UNSUBACK Properties
		properties_set                  properties_  {};

		// The Payload contains a list of Reason Codes.
		// Each Reason Code corresponds to a Topic Filter in the UNSUBSCRIBE packet being acknowledged. 
		one_byte_integer_set            reason_codes_{};
	};

	/**
	 * PINGREQ - PING request
	 * 
	 * The PINGREQ packet is sent from a Client to the Server.
	 * 
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901195
	 */
	class pingreq : public fixed_header<version_number>
	{
	public:
		pingreq() : fixed_header(control_packet_type::pingreq)
		{
			update_remain_length();
		}

		inline std::size_t required_size() const
		{
			return (fixed_header::required_size() + fixed_header::remain_length());
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline pingreq& serialize(Container& buffer)
		{
			fixed_header::serialize(buffer);

			return (*this);
		}

		inline pingreq& deserialize(std::string_view& data)
		{
			fixed_header::deserialize(data);

			update_remain_length();

			return (*this);
		}

		inline pingreq& update_remain_length()
		{
			remain_length_ = static_cast<std::int32_t>(0
				);

			return (*this);
		}
	protected:
		// The PINGREQ packet has no Variable Header.

		// The PINGREQ packet has no Payload.
	};

	/**
	 * PINGRESP - PING response
	 * 
	 * A PINGRESP Packet is sent by the Server to the Client in response to a PINGREQ packet.
	 * It indicates that the Server is alive.
	 * 
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901200
	 */
	class pingresp : public fixed_header<version_number>
	{
	public:
		pingresp() : fixed_header(control_packet_type::pingresp)
		{
			update_remain_length();
		}

		inline std::size_t required_size() const
		{
			return (fixed_header::required_size() + fixed_header::remain_length());
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline pingresp& serialize(Container& buffer)
		{
			fixed_header::serialize(buffer);

			return (*this);
		}

		inline pingresp& deserialize(std::string_view& data)
		{
			fixed_header::deserialize(data);

			update_remain_length();

			return (*this);
		}

		inline pingresp& update_remain_length()
		{
			remain_length_ = static_cast<std::int32_t>(0
				);

			return (*this);
		}
	protected:
		// The PINGRESP packet has no Variable Header.

		// The PINGRESP packet has no Payload.
	};

	/**
	 * DISCONNECT - Disconnect notification
	 * 
	 * The DISCONNECT packet is the final MQTT Control Packet sent from the Client or the Server.
	 * It indicates the reason why the Network Connection is being closed.
	 * The Client or Server MAY send a DISCONNECT packet before closing the Network Connection.
	 * If the Network Connection is closed without the Client first sending a DISCONNECT packet
	 * with Reason Code 0x00 (Normal disconnection) and the Connection has a Will Message, the
	 * Will Message is published. Refer to section 3.1.2.5 for further details.
	 * A Server MUST NOT send a DISCONNECT until after it has sent a CONNACK with Reason Code of
	 * less than 0x80 [MQTT-3.14.0-1].
	 * 
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901205
	 */
	class disconnect : public fixed_header<version_number>, public detail::reason_string_ops<disconnect>
	{
	public:
		disconnect() : fixed_header(control_packet_type::disconnect)
		{
			update_remain_length();
		}

		template<class... Properties, std::enable_if_t<is_property<Properties...>(), int> = 0>
		explicit disconnect(std::uint8_t reason_code, Properties&&... Props)
			: fixed_header(control_packet_type::disconnect)
			, reason_code_(reason_code)
			, properties_(std::forward<Properties>(Props)...)
		{
			update_remain_length();
		}

		inline std::size_t required_size() const
		{
			return (fixed_header::required_size() + fixed_header::remain_length());
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline disconnect& serialize(Container& buffer)
		{
			update_remain_length();

			fixed_header::serialize(buffer);

			reason_code_             .serialize(buffer);
			properties_              .serialize(buffer);

			return (*this);
		}

		inline disconnect& deserialize(std::string_view& data)
		{
			fixed_header::deserialize(data);

			// If the Remaining Length is less than 1 the value of 0x00 (Normal disconnection) is used.
			if (!data.empty())
			{
				reason_code_            .deserialize(data);
				properties_             .deserialize(data);
			}

			update_remain_length();

			return (*this);
		}

		inline std::uint8_t    reason_code    () const { return reason_code_.value()               ; }
		inline properties_set& properties     ()       { return properties_                        ; }
		inline properties_set const& properties() const { return properties_                        ; }

		inline disconnect       & reason_code    (std::uint8_t v) { reason_code_ = v; return (*this); }
		template<class... Properties>
		inline disconnect       & properties(Properties&&... Props)
		{
			properties_.set(std::forward<Properties>(Props)...);
			update_remain_length();
			return (*this);
		}

		inline disconnect& update_remain_length()
		{
			remain_length_ = static_cast<std::int32_t>(0
				+ reason_code_           .required_size()
				+ properties_            .required_size()
				);

			return (*this);
		}
	protected:
		// The Variable Header of the DISCONNECT Packet contains the following fields in the order:
		// Disconnect Reason Code, and Properties.
		// Byte 1 in the Variable Header is the Disconnect Reason Code.
		// If the Remaining Length is less than 1 the value of 0x00 (Normal disconnection) is used.
		one_byte_integer reason_code_{ 0 };
		
		// DISCONNECT Properties
		properties_set   properties_ {   };

		// The DISCONNECT packet has no Payload.
	};

	/**
	 * AUTH - Authentication exchange
	 * 
	 * An AUTH packet is sent from Client to Server or Server to Client as part of an extended
	 * authentication exchange, such as challenge / response authentication.
	 * It is a Protocol Error for the Client or Server to send an AUTH packet if the CONNECT packet
	 * did not contain the same Authentication Method.
	 * 
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901217
	 */
	class auth : public fixed_header<version_number>, public detail::reason_string_ops<auth>
	{
	public:
		auth() : fixed_header(control_packet_type::auth)
		{
			update_remain_length();
		}

		template<class... Properties, std::enable_if_t<is_property<Properties...>(), int> = 0>
		explicit auth(std::uint8_t reason_code, Properties&&... Props)
			: fixed_header(control_packet_type::auth)
			, reason_code_(reason_code)
			, properties_(std::forward<Properties>(Props)...)
		{
			update_remain_length();
		}

		inline std::size_t required_size() const
		{
			return (fixed_header::required_size() + fixed_header::remain_length());
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline auth& serialize(Container& buffer)
		{
			update_remain_length();

			fixed_header::serialize(buffer);

			reason_code_             .serialize(buffer);
			properties_              .serialize(buffer);

			return (*this);
		}

		inline auth& deserialize(std::string_view& data)
		{
			fixed_header::deserialize(data);

			// The Reason Code and Property Length can be omitted if the Reason Code is 0x00 (Success)
			// and there are no Properties. In this case the AUTH has a Remaining Length of 0.
			if (!data.empty())
			{
				reason_code_            .deserialize(data);
				properties_             .deserialize(data);
			}

			update_remain_length();

			return (*this);
		}

		inline std::uint8_t    reason_code    () const { return reason_code_.value()               ; }
		inline properties_set& properties     ()       { return properties_                        ; }
		inline properties_set const& properties() const { return properties_                        ; }

		inline auth       & reason_code    (std::uint8_t v) { reason_code_ = v; return (*this); }
		template<class... Properties>
		inline auth       & properties(Properties&&... Props)
		{
			properties_.set(std::forward<Properties>(Props)...);
			update_remain_length();
			return (*this);
		}

		inline auth& update_remain_length()
		{
			remain_length_ = static_cast<std::int32_t>(0
				+ reason_code_           .required_size()
				+ properties_            .required_size()
				);

			return (*this);
		}
	protected:
		// The Variable Header of the AUTH Packet contains the following fields in the order:
		// Authenticate Reason Code, and Properties.
		// Byte 0 in the Variable Header is the Authenticate Reason Code.
		// The values for the one byte unsigned Authenticate Reason Code field are shown below.
		// The sender of the AUTH Packet MUST use one of the Authenticate Reason Codes [MQTT-3.15.2-1].
		// 0-Success 24-Continue authentication 25-Re-authenticate
		// The Reason Code and Property Length can be omitted if the Reason Code is 0x00 (Success)
		// and there are no Properties. In this case the AUTH has a Remaining Length of 0.
		one_byte_integer reason_code_{ 0 };
		
		// AUTH Properties
		properties_set   properties_ {   };

		// The AUTH packet has no Payload.
	};
}

namespace asio2::mqtt
{
	template<typename message_type>
	inline constexpr bool is_v5_message()
	{
		using type = asio2::detail::remove_cvref_t<message_type>;
		if constexpr (
			std::is_same_v<type, mqtt::v5::connect     > ||
			std::is_same_v<type, mqtt::v5::connack     > ||
			std::is_same_v<type, mqtt::v5::publish     > ||
			std::is_same_v<type, mqtt::v5::puback      > ||
			std::is_same_v<type, mqtt::v5::pubrec      > ||
			std::is_same_v<type, mqtt::v5::pubrel      > ||
			std::is_same_v<type, mqtt::v5::pubcomp     > ||
			std::is_same_v<type, mqtt::v5::subscribe   > ||
			std::is_same_v<type, mqtt::v5::suback      > ||
			std::is_same_v<type, mqtt::v5::unsubscribe > ||
			std::is_same_v<type, mqtt::v5::unsuback    > ||
			std::is_same_v<type, mqtt::v5::pingreq     > ||
			std::is_same_v<type, mqtt::v5::pingresp    > ||
			std::is_same_v<type, mqtt::v5::disconnect  > ||
			std::is_same_v<type, mqtt::v5::auth        > )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

#endif // !__ASIO2_MQTT_PROTOCOL_V5_HPP__
