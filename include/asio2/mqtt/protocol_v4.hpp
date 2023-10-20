/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * chinese : http://blog.mcxiaoke.com/mqtt/protocol/MQTT-3.1.1-CN.html
 * english : http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_MQTT_PROTOCOL_V4_HPP__
#define __ASIO2_MQTT_PROTOCOL_V4_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/mqtt/core.hpp>

namespace asio2::mqtt::v4
{
	static constexpr std::uint8_t version_number = asio2::detail::to_underlying(mqtt::version::v4);

	enum class connect_reason_code : std::uint8_t
	{
		/* 0	0x00	Connection Accepted : */ success						 = 0,
		/* 1	0x01	Connection Refused  : */ unacceptable_protocol_version	 = 1,
		/* 2	0x02	Connection Refused  : */ identifier_rejected			 = 2,
		/* 3	0x03	Connection Refused  : */ server_unavailable				 = 3,
		/* 4	0x04	Connection Refused  : */ bad_user_name_or_password		 = 4,
		/* 5	0x05	Connection Refused  : */ not_authorized					 = 5,
		/* 6 - 255		Reserved for future use */
	};

	/**
	 * CONNECT - Client requests a connection to a Server
	 * 
	 * After a Network Connection is established by a Client to a Server, the first packet sent from the
	 * Client to the Server MUST be a CONNECT packet [MQTT-3.1.0-1].
	 * 
	 * A Client can only send the CONNECT Packet once over a Network Connection. The Server MUST process
	 * a second CONNECT Packet sent from a Client as a protocol violation and disconnect the Client
	 * 
	 * http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718028
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
			fixed_header::serialize(buffer);

			protocol_name_                   .serialize(buffer);
			protocol_version_                .serialize(buffer);
			connect_flags_.byte              .serialize(buffer);
			keep_alive_                      .serialize(buffer);
			client_id_                       .serialize(buffer);
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
			client_id_              .deserialize(data);

			if (has_will())
			{
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
		inline bool         clean_start     () const { return                      (connect_flags_.bits.clean_session); }
		inline bool         clean_session   () const { return                      (connect_flags_.bits.clean_session); }
		inline bool         has_will        () const { return                      (connect_flags_.bits.will_flag    ); }
		inline qos_type     will_qos        () const { return static_cast<qos_type>(connect_flags_.bits.will_qos     ); }
		inline bool         will_retain     () const { return                      (connect_flags_.bits.will_retain  ); }
		inline bool         has_password    () const { return                      (connect_flags_.bits.password_flag); }
		inline bool         has_username    () const { return                      (connect_flags_.bits.username_flag); }

		inline connect&     clean_start     (bool v) { connect_flags_.bits.clean_session = v; return (*this); }
		inline connect&     clean_session   (bool v) { connect_flags_.bits.clean_session = v; return (*this); }

		inline two_byte_integer::value_type keep_alive        () const { return keep_alive_               ; }
		inline utf8_string::view_type       client_id         () const { return client_id_    .data_view(); }
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
		inline connect& client_id(String&& v)
		{
			client_id_ = std::forward<String>(v);
			update_remain_length();
			return (*this);
		}
		template<class String>
		inline connect& username(String&& v)
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
		template<class String1, class String2, class QosOrInt>
		inline connect& will_attributes(String1&& topic, String2&& payload,
			QosOrInt qos = qos_type::at_most_once, bool retain = false)
		{
			will_topic_   = std::forward<String1>(topic);
			will_payload_ = std::forward<String2>(payload);
			connect_flags_.bits.will_flag   = true;
			connect_flags_.bits.will_qos    = static_cast<std::uint8_t>(qos);
			connect_flags_.bits.will_retain = retain;
			update_remain_length();
			return (*this);
		}

		inline bool will_topic_has_value() const noexcept { return will_topic_.has_value(); }
		inline bool username_has_value  () const noexcept { return username_  .has_value(); }
		inline bool password_has_value  () const noexcept { return password_  .has_value(); }

		inline connect& update_remain_length()
		{
			remain_length_ = static_cast<std::int32_t>(0
				+ protocol_name_                 .required_size()
				+ protocol_version_              .required_size()
				+ connect_flags_.byte            .required_size()
				+ keep_alive_                    .required_size()
				+ client_id_                     .required_size()
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

		// The 8 bit unsigned value that represents the revision level of the protocol used by the Client. 
		// The value of the Protocol Level field for the version 3.1.1 of the protocol is 4 (0x04).
		one_byte_integer protocol_version_{ 0x04 };

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
				bool         clean_session : 1; // Clean Session flag
				std::uint8_t reserved      : 1; // unused
			} bits;
		#else
			struct
			{
				std::uint8_t reserved      : 1; // unused
				bool         clean_session : 1; // Clean Session flag
				bool         will_flag     : 1; // will flag
				std::uint8_t will_qos      : 2; // will QoS value
				bool         will_retain   : 1; // will retain setting
				bool         password_flag : 1; // Password Flag
				bool         username_flag : 1; // User Name Flag
			} bits;
		#endif
		} connect_flags_{};                     // connect flags byte

		// The Keep Alive is a time interval measured in seconds. Expressed as a 16-bit word
		// Default to 60 seconds
		two_byte_integer               keep_alive_  { 60 };

		// The Client Identifier (ClientID) identifies the Client to the Server. 
		// Each Client connecting to the Server has a unique ClientID. 
		utf8_string                    client_id_   {};

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
	 * CONNACK - Acknowledge connection request
	 * 
	 * http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718033
	 */
	class connack : public fixed_header<version_number>
	{
	public:
		connack() : fixed_header(control_packet_type::connack)
		{
			update_remain_length();
		}

		explicit connack(bool session_present, std::uint8_t reason_code)
			: fixed_header(control_packet_type::connack)
			, reason_code_(reason_code)
		{
			connack_flags_.bits.session_present = session_present;
			update_remain_length();
		}

		explicit connack(bool session_present, connect_reason_code reason_code)
			: fixed_header(control_packet_type::connack)
			, reason_code_(asio2::detail::to_underlying(reason_code))
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
			fixed_header::serialize(buffer);

			connack_flags_.byte      .serialize(buffer);
			reason_code_             .serialize(buffer);

			return (*this);
		}

		inline connack& deserialize(std::string_view& data)
		{
			fixed_header::deserialize(data);

			connack_flags_.byte     .deserialize(data);
			reason_code_            .deserialize(data);

			update_remain_length();

			return (*this);
		}

		inline bool                session_present() const { return connack_flags_.bits.session_present; }
		inline connect_reason_code reason_code    () const { return static_cast<connect_reason_code>(reason_code_.value()); }

		inline connack       & session_present(bool         v) { connack_flags_.bits.session_present = v; return (*this); }
		inline connack       & reason_code    (std::uint8_t v) { reason_code_                        = v; return (*this); }
		inline connack       & reason_code    (connect_reason_code v)
		{ reason_code_ = asio2::detail::to_underlying(v); return (*this); }

		inline connack& update_remain_length()
		{
			remain_length_ = static_cast<std::int32_t>(0
				+ connack_flags_.byte    .required_size()
				+ reason_code_           .required_size()
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
	};

	/**
	 * PUBLISH - Publish message
	 * 
	 * A PUBLISH packet is sent from a Client to a Server or from a Server to a Client to transport an Application Message.
	 * 
	 * http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718037
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
			payload_.serialize(buffer);

			return (*this);
		}

		inline publish& deserialize(std::string_view& data)
		{
			fixed_header::deserialize(data);

			topic_name_.deserialize(data);
			// A PUBLISH Packet MUST NOT contain a Packet Identifier if its QoS value is set to 0 [MQTT-2.3.1-5].
			if (type_and_flags_.bits.qos == 1 || type_and_flags_.bits.qos == 2)
			{
				two_byte_integer packet_id{};
				packet_id.deserialize(data);
				packet_id_ = packet_id;
			}
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
		inline application_message::view_type  payload   () const { return payload_.data_view()   ; }

		inline publish       &  packet_id (std::uint16_t    v) { packet_id_  = v                      ; update_remain_length(); return (*this); }
		template<class String>
		inline publish       &  topic_name(String&&         v) { topic_name_ = std::forward<String>(v); update_remain_length(); return (*this); }
		template<class String>
		inline publish       &  payload   (String&&         v) { payload_    = std::forward<String>(v); update_remain_length(); return (*this); }

		inline bool has_packet_id() const noexcept { return packet_id_.has_value(); }

		inline publish& update_remain_length()
		{
			remain_length_ = static_cast<std::int32_t>(0
				+               topic_name_.required_size()
				+ (packet_id_ ? packet_id_->required_size() : 0)
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
	 * A PUBACK Packet is the response to a PUBLISH Packet with QoS level 1.
	 * 
	 * http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718043
	 */
	class puback : public fixed_header<version_number>
	{
	public:
		puback() : fixed_header(control_packet_type::puback)
		{
			update_remain_length();
		}

		explicit puback(std::uint16_t packet_id)
			: fixed_header(control_packet_type::puback)
			, packet_id_  (packet_id)
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
			fixed_header::serialize(buffer);

			packet_id_  .serialize(buffer);

			return (*this);
		}

		inline puback& deserialize(std::string_view& data)
		{
			fixed_header::deserialize(data);

			packet_id_  .deserialize(data);

			update_remain_length();

			return (*this);
		}

		inline two_byte_integer::value_type    packet_id  () const { return packet_id_  .value()   ; }

		inline puback       &  packet_id  (std::uint16_t v) { packet_id_   = v; return (*this); }

		inline puback& update_remain_length()
		{
			remain_length_ = static_cast<std::int32_t>(0
				+ packet_id_  .required_size()
				);

			return (*this);
		}
	protected:
		// This contains the Packet Identifier from the PUBLISH Packet that is being acknowledged.
		two_byte_integer                packet_id_ {};
	};

	/**
	 * PUBREC - Publish received (QoS 2 publish received, part 1)
	 * 
	 * A PUBREC Packet is the response to a PUBLISH Packet with QoS 2.
	 * It is the second packet of the QoS 2 protocol exchange.
	 * 
	 * http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718048
	 */
	class pubrec : public fixed_header<version_number>
	{
	public:
		pubrec() : fixed_header(control_packet_type::pubrec)
		{
			update_remain_length();
		}

		explicit pubrec(std::uint16_t packet_id)
			: fixed_header(control_packet_type::pubrec)
			, packet_id_  (packet_id)
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
			fixed_header::serialize(buffer);

			packet_id_  .serialize(buffer);

			return (*this);
		}

		inline pubrec& deserialize(std::string_view& data)
		{
			fixed_header::deserialize(data);

			packet_id_  .deserialize(data);

			update_remain_length();

			return (*this);
		}

		inline two_byte_integer::value_type    packet_id  () const { return packet_id_  .value()   ; }

		inline pubrec       &  packet_id  (std::uint16_t v) { packet_id_   = v; return (*this); }

		inline pubrec& update_remain_length()
		{
			remain_length_ = static_cast<std::int32_t>(0
				+ packet_id_  .required_size()
				);

			return (*this);
		}
	protected:
		// The variable header contains the Packet Identifier from the PUBLISH Packet that is being acknowledged.
		two_byte_integer                packet_id_ {};
	};

	/**
	 * PUBREL - Publish release (QoS 2 publish received, part 2)
	 * 
	 * A PUBREL Packet is the response to a PUBREC Packet.
	 * It is the third packet of the QoS 2 protocol exchange.
	 * 
	 * http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718053
	 */
	class pubrel : public fixed_header<version_number>
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

		explicit pubrel(std::uint16_t packet_id)
			: fixed_header(control_packet_type::pubrel)
			, packet_id_  (packet_id)
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
			fixed_header::serialize(buffer);

			packet_id_  .serialize(buffer);

			return (*this);
		}

		inline pubrel& deserialize(std::string_view& data)
		{
			fixed_header::deserialize(data);

			packet_id_  .deserialize(data);

			update_remain_length();

			return (*this);
		}

		inline two_byte_integer::value_type    packet_id  () const { return packet_id_  .value()   ; }

		inline pubrel       &  packet_id  (std::uint16_t v) { packet_id_   = v; return (*this); }

		inline pubrel& update_remain_length()
		{
			remain_length_ = static_cast<std::int32_t>(0
				+ packet_id_  .required_size()
				);

			return (*this);
		}
	protected:
		// The variable header contains the same Packet Identifier as the PUBREC Packet that is being acknowledged.
		two_byte_integer                packet_id_ {};
	};

	/**
	 * PUBCOMP - Publish complete (QoS 2 publish received, part 3)
	 * 
	 * The PUBCOMP Packet is the response to a PUBREL Packet.
	 * It is the fourth and final packet of the QoS 2 protocol exchange.
	 * 
	 * http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718058
	 */
	class pubcomp : public fixed_header<version_number>
	{
	public:
		pubcomp() : fixed_header(control_packet_type::pubcomp)
		{
			update_remain_length();
		}

		explicit pubcomp(std::uint16_t packet_id)
			: fixed_header(control_packet_type::pubcomp)
			, packet_id_  (packet_id)
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
			fixed_header::serialize(buffer);

			packet_id_  .serialize(buffer);

			return (*this);
		}

		inline pubcomp& deserialize(std::string_view& data)
		{
			fixed_header::deserialize(data);

			packet_id_  .deserialize(data);

			update_remain_length();

			return (*this);
		}

		inline two_byte_integer::value_type    packet_id  () const { return packet_id_  .value()   ; }

		inline pubcomp       &  packet_id  (std::uint16_t v) { packet_id_   = v; return (*this); }

		inline pubcomp& update_remain_length()
		{
			remain_length_ = static_cast<std::int32_t>(0
				+ packet_id_  .required_size()
				);

			return (*this);
		}
	protected:
		// The variable header contains the same Packet Identifier as the PUBREL Packet that is being acknowledged.
		two_byte_integer                packet_id_ {};
	};

	/**
	 * SUBSCRIBE - Subscribe to topics
	 * 
	 * The SUBSCRIBE Packet is sent from the Client to the Server to create one or more Subscriptions.
	 * Each Subscription registers a Client's interest in one or more Topics.
	 * The Server sends PUBLISH Packets to the Client in order to forward Application Messages that were
	 * published to Topics that match these Subscriptions.
	 * The SUBSCRIBE Packet also specifies (for each Subscription) the maximum QoS with which the Server
	 * can send Application Messages to the Client.
	 * 
	 * http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718063
	 */
	class subscribe : public fixed_header<version_number>
	{
	public:
		subscribe() : fixed_header(control_packet_type::subscribe)
		{
			// Bits 3,2,1 and 0 of the fixed header of the SUBSCRIBE Control Packet are reserved and MUST be
			// set to 0,0,1 and 0 respectively. The Server MUST treat any other value as malformed and close
			// the Network Connection [MQTT-3.8.1-1].
			type_and_flags_.reserved.bit1 = 1;

			update_remain_length();
		}

		explicit subscribe(std::uint16_t packet_id)
			: fixed_header(control_packet_type::subscribe)
			, packet_id_  (packet_id)
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
			subscriptions_.serialize(buffer);

			return (*this);
		}

		inline subscribe& deserialize(std::string_view& data)
		{
			fixed_header::deserialize(data);

			packet_id_    .deserialize(data);
			subscriptions_.deserialize(data);

			update_remain_length();

			return (*this);
		}

		inline two_byte_integer::value_type    packet_id    () const { return packet_id_  .value()   ; }
		inline subscriptions_set&              subscriptions()       { return subscriptions_         ; }
		inline subscriptions_set const&        subscriptions() const { return subscriptions_         ; }

		inline subscribe       &  packet_id  (std::uint16_t v) { packet_id_   = v; return (*this); }

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
				+ subscriptions_.required_size()
				);

			return (*this);
		}
	protected:
		// The variable header contains a Packet Identifier.
		two_byte_integer                packet_id_    {};

		// The payload of a SUBSCRIBE Packet contains a list of Topic Filters indicating the Topics
		// to which the Client wants to subscribe. The Topic Filters in a SUBSCRIBE packet payload
		// MUST be UTF-8 encoded strings as defined in Section 1.5.3 [MQTT-3.8.3-1]. A Server SHOULD
		// support Topic filters that contain the wildcard characters defined in Section 4.7.1. If
		// it chooses not to support topic filters that contain wildcard characters it MUST reject 
		// any Subscription request whose filter contains them [MQTT-3.8.3-2]. Each filter is
		// followed by a byte called the Requested QoS. This gives the maximum QoS level at which 
		// the Server can send Application Messages to the Client.
		subscriptions_set               subscriptions_{};
	};

	/**
	 * SUBACK - Subscribe acknowledgement
	 * 
	 * A SUBACK Packet is sent by the Server to the Client to confirm receipt and processing of a SUBSCRIBE Packet.
	 * 
	 * http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718068
	 */
	class suback : public fixed_header<version_number>
	{
	public:
		suback() : fixed_header(control_packet_type::suback)
		{
			update_remain_length();
		}

		explicit suback(std::uint16_t packet_id)
			: fixed_header(control_packet_type::suback)
			, packet_id_  (packet_id)
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
			reason_codes_ .serialize(buffer);

			return (*this);
		}

		inline suback& deserialize(std::string_view& data)
		{
			fixed_header::deserialize(data);

			packet_id_    .deserialize(data);
			reason_codes_ .deserialize(data);

			update_remain_length();

			return (*this);
		}

		inline two_byte_integer::value_type    packet_id    () const { return packet_id_  .value()   ; }
		inline one_byte_integer_set  &         reason_codes ()       { return reason_codes_          ; }
		inline one_byte_integer_set const&     reason_codes () const { return reason_codes_          ; }

		inline suback       &  packet_id  (std::uint16_t v) { packet_id_   = v; return (*this); }

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
				+ reason_codes_ .required_size()
				);

			return (*this);
		}
	protected:
		// The variable header contains the Packet Identifier from the SUBSCRIBE Packet that is being acknowledged. 
		two_byte_integer                packet_id_   {};

		// The payload contains a list of return codes. Each return code corresponds to a Topic Filter
		// in the SUBSCRIBE Packet being acknowledged. The order of return codes in the SUBACK Packet
		// MUST match the order of Topic Filters in the SUBSCRIBE Packet [MQTT-3.9.3-1].
		one_byte_integer_set            reason_codes_{};
	};

	/**
	 * UNSUBSCRIBE - Unsubscribe from topics
	 * 
	 * An UNSUBSCRIBE Packet is sent by the Client to the Server, to unsubscribe from topics.
	 * 
	 * http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718072
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
			topic_filters_.serialize(buffer);

			return (*this);
		}

		inline unsubscribe& deserialize(std::string_view& data)
		{
			fixed_header::deserialize(data);

			packet_id_    .deserialize(data);
			topic_filters_.deserialize(data);

			update_remain_length();

			return (*this);
		}

		inline two_byte_integer::value_type    packet_id    () const { return packet_id_  .value()   ; }
		inline utf8_string_set  &              topic_filters()       { return topic_filters_         ; }
		inline utf8_string_set  const&         topic_filters() const { return topic_filters_         ; }

		inline unsubscribe       &  packet_id  (std::uint16_t v) { packet_id_   = v; return (*this); }

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
				+ topic_filters_.required_size()
				);

			return (*this);
		}
	protected:
		// The variable header contains a Packet Identifier. 
		two_byte_integer                packet_id_    {};

		// The payload for the UNSUBSCRIBE Packet contains the list of Topic Filters that the Client
		// wishes to unsubscribe from. The Topic Filters in an UNSUBSCRIBE packet MUST be UTF-8
		// encoded strings as defined in Section 1.5.3, packed contiguously [MQTT-3.10.3-1].
		// The Payload of an UNSUBSCRIBE packet MUST contain at least one Topic Filter.An UNSUBSCRIBE
		// packet with no payload is a protocol violation[MQTT - 3.10.3 - 2].See section 4.8 for
		// information about handling errors.
		utf8_string_set                 topic_filters_{};
	};

	/**
	 * UNSUBACK - Unsubscribe acknowledgement
	 * 
	 * The UNSUBACK packet is sent by the Server to the Client to confirm receipt of an UNSUBSCRIBE packet.
	 * 
	 * http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718077
	 */
	class unsuback : public fixed_header<version_number>
	{
	public:
		unsuback() : fixed_header(control_packet_type::unsuback)
		{
			update_remain_length();
		}

		explicit unsuback(std::uint16_t packet_id)
			: fixed_header(control_packet_type::unsuback)
			, packet_id_  (packet_id)
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
			fixed_header::serialize(buffer);

			packet_id_    .serialize(buffer);

			return (*this);
		}

		inline unsuback& deserialize(std::string_view& data)
		{
			fixed_header::deserialize(data);

			packet_id_    .deserialize(data);

			update_remain_length();

			return (*this);
		}

		inline two_byte_integer::value_type    packet_id    () const { return packet_id_  .value()   ; }

		inline unsuback       &  packet_id  (std::uint16_t v) { packet_id_   = v; return (*this); }

		inline unsuback& update_remain_length()
		{
			remain_length_ = static_cast<std::int32_t>(0
				+ packet_id_    .required_size()
				);

			return (*this);
		}
	protected:
		// The variable header contains the Packet Identifier of the UNSUBSCRIBE Packet that is being acknowledged.
		two_byte_integer                packet_id_   {};

		// The UNSUBACK Packet has no payload.
	};

	/**
	 * PINGREQ - PING request
	 * 
	 * The PINGREQ packet is sent from a Client to the Server.
	 * 
	 * http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718081
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
	 * http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718086
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
	 * The DISCONNECT Packet is the final Control Packet sent from the Client to the Server.
	 * It indicates that the Client is disconnecting cleanly.
	 * 
	 * http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718090
	 */
	class disconnect : public fixed_header<version_number>
	{
	public:
		disconnect() : fixed_header(control_packet_type::disconnect)
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
			fixed_header::serialize(buffer);

			return (*this);
		}

		inline disconnect& deserialize(std::string_view& data)
		{
			fixed_header::deserialize(data);

			update_remain_length();

			return (*this);
		}

		inline disconnect& update_remain_length()
		{
			remain_length_ = static_cast<std::int32_t>(0
				);

			return (*this);
		}
	protected:
		// The DISCONNECT Packet has no variable header.

		// The DISCONNECT Packet has no payload.
	};

}

namespace asio2::mqtt
{
	template<typename = void>
	inline constexpr std::string_view to_string(v4::connect_reason_code v)
	{
		using namespace std::string_view_literals;
		switch(v)
		{
		case v4::connect_reason_code::success						 : return "Connection accepted"sv;
		case v4::connect_reason_code::unacceptable_protocol_version	 : return "The Server does not support the level of the MQTT protocol requested by the Client"sv;
		case v4::connect_reason_code::identifier_rejected			 : return "The Client identifier is correct UTF-8 but not allowed by the Server"sv;
		case v4::connect_reason_code::server_unavailable			 : return "The Network Connection has been made but the MQTT service is unavailable"sv;
		case v4::connect_reason_code::bad_user_name_or_password		 : return "The data in the user name or password is malformed"sv;
		case v4::connect_reason_code::not_authorized				 : return "The Client is not authorized to connect"sv;
		default:
			ASIO2_ASSERT(false);
			break;
		}
		return "unknown"sv;
	}

	template<typename message_type>
	inline constexpr bool is_v4_message()
	{
		using type = asio2::detail::remove_cvref_t<message_type>;
		if constexpr (
			std::is_same_v<type, mqtt::v4::connect     > ||
			std::is_same_v<type, mqtt::v4::connack     > ||
			std::is_same_v<type, mqtt::v4::publish     > ||
			std::is_same_v<type, mqtt::v4::puback      > ||
			std::is_same_v<type, mqtt::v4::pubrec      > ||
			std::is_same_v<type, mqtt::v4::pubrel      > ||
			std::is_same_v<type, mqtt::v4::pubcomp     > ||
			std::is_same_v<type, mqtt::v4::subscribe   > ||
			std::is_same_v<type, mqtt::v4::suback      > ||
			std::is_same_v<type, mqtt::v4::unsubscribe > ||
			std::is_same_v<type, mqtt::v4::unsuback    > ||
			std::is_same_v<type, mqtt::v4::pingreq     > ||
			std::is_same_v<type, mqtt::v4::pingresp    > ||
			std::is_same_v<type, mqtt::v4::disconnect  > )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

#endif // !__ASIO2_MQTT_PROTOCOL_V4_HPP__
