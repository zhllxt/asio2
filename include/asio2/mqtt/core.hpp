/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * https://github.com/mcxiaoke/mqtt
 * https://github.com/eclipse/paho.mqtt.c
 * https://github.com/eclipse/paho.mqtt.cpp
 * 
 * https://github.com/mqtt/mqtt.org/wiki/libraries
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_MQTT_CORE_HPP__
#define __ASIO2_MQTT_CORE_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <iosfwd>
#include <cstdint>
#include <cstddef>

#include <memory>
#include <chrono>
#include <functional>
#include <atomic>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <array>
#include <variant>
#include <tuple>
#include <type_traits>

#include <asio2/external/predef.h>

#ifdef ASIO2_HEADER_ONLY
#include <asio2/bho/beast/websocket/detail/utf8_checker.hpp>
#else
#include <boost/beast/websocket/detail/utf8_checker.hpp>
#endif

#include <asio2/base/detail/util.hpp>

#include <asio2/mqtt/detail/mqtt_topic_util.hpp>

#include <asio2/mqtt/error.hpp>

/*
 * Server                     Broker      Port             Websocket
 * ------------------------------------------------------------------
 * iot.eclipse.org            Mosquitto   1883/8883        n/a
 * broker.hivemq.com          HiveMQ      1883             8000        **
 * test.mosquitto.org         Mosquitto   1883/8883/8884   8080/8081
 * test.mosca.io              mosca       1883             80
 * broker.mqttdashboard.com   HiveMQ      1883
 *
 */

//namespace asio2::detail
//{
//	template <class> class mqtt_handler_t;
//	template <class> class mqtt_invoker_t;
//	template <class> class mqtt_aop_connect;
//	template <class> class mqtt_aop_publish;
//}

namespace asio2::mqtt
{
	static constexpr unsigned int max_payload = 268435455u;

	enum class version : std::uint8_t
	{
		// mqtt version 3.1  , The protocol version of the mqtt 3.1   CONNECT message is 0x03
		v3 = 3,

		// mqtt version 3.1.1, The protocol version of the mqtt 3.1.1 CONNECT message is 0x04
		v4 = 4,

		// mqtt version 5.0  , The protocol version of the mqtt 5.0   CONNECT message is 0x05
		v5 = 5
	};

	/**
	 * MQTT Control Packet type
	 * 
	 * http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718021
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901022
	 */
	enum class control_packet_type : std::uint8_t
	{
		// [Forbidden]
		// Reserved
		reserved    = 0,

		// [Client to Server]
		// Client request to connect to Server
		connect     = 1,

		// [Server to Client]
		// Connect acknowledgment
		connack     = 2,

		// [Client to Server] or [Server to Client]
		// Publish message
		publish     = 3,

		// [Client to Server] or [Server to Client]
		// Publish acknowledgment
		puback      = 4,

		// [Client to Server] or [Server to Client]
		// Publish received (assured delivery part 1)
		pubrec      = 5,

		// [Client to Server] or [Server to Client]
		// Publish release (assured delivery part 2)
		pubrel      = 6,

		// [Client to Server] or [Server to Client]
		// Publish complete (assured delivery part 3)
		pubcomp     = 7,

		// [Client to Server]
		// Client subscribe request
		subscribe   = 8,

		// [Server to Client]
		// Subscribe acknowledgment
		suback      = 9,

		// [Client to Server]
		// Unsubscribe request
		unsubscribe = 10,

		// [Server to Client]
		// Unsubscribe acknowledgment
		unsuback    = 11,

		// [Client to Server]
		// PING request
		pingreq     = 12,

		// [Server to Client]
		// PING response
		pingresp    = 13,

		// [Client to Server]
		// Client is disconnecting
		disconnect  = 14,

		// [Client to Server] or [Server to Client]
		// Authentication exchange
		// Only valid in mqtt 5.0
		// https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901256
		auth        = 15,
	};

	/**
	 * Data representation
	 * 
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901006
	 */
	enum class representation_type : std::uint8_t
	{
		one_byte_integer,
		two_byte_integer,
		four_byte_integer,
		variable_byte_integer,
		binary_data,
		utf8_string,
		utf8_string_pair
	};

	/**
	 * This field indicates the level of assurance for delivery of an Application Message.
	 * The QoS levels are shown below.
	 * 
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901103
	 */
	enum class qos_type : std::uint8_t
	{
		at_most_once  = 0, // At most once delivery
		at_least_once = 1, // At least once delivery
		exactly_once  = 2, // Exactly once delivery
	};

	static constexpr std::uint8_t qos_min_value = static_cast<std::uint8_t>(qos_type::at_most_once);
	static constexpr std::uint8_t qos_max_value = static_cast<std::uint8_t>(qos_type::exactly_once);

	template<class QosOrInt>
	typename std::enable_if_t<
		std::is_same_v<asio2::detail::remove_cvref_t<QosOrInt>, mqtt::qos_type> ||
		std::is_integral_v<asio2::detail::remove_cvref_t<QosOrInt>>, bool>
	inline is_valid_qos(QosOrInt q)
	{
		return (static_cast<std::uint8_t>(q) >= qos_min_value && static_cast<std::uint8_t>(q) <= qos_max_value);
	}

	/**
	 * Bits 4 and 5 of the Subscription Options represent the Retain Handling option.
	 * This option specifies whether retained messages are sent when the subscription is established.
	 * This does not affect the sending of retained messages at any point after the subscribe.
	 * If there are no retained messages matching the Topic Filter, all of these values act the same.
	 * The values are:
	 * 
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901169
	 */
	enum class retain_handling_type : std::uint8_t
	{
		// Send retained messages at the time of the subscribe
		send                       = 0,

		// Send retained messages at subscribe only if the subscription does not currently exist
		send_only_new_subscription = 1,

		// Do not send retained messages at the time of the subscribe
		not_send                   = 2,
	};

	/*
	 * The algorithm for encoding a non-negative integer (X) into the Variable Byte Integer encoding scheme is as follows:
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901011
	 */
	template<class Integer>
	std::array<std::uint8_t, 4> encode_variable_byte_integer(Integer X)
	{
		std::int32_t i = 0;
		std::array<std::uint8_t, 4> value{};

		if (static_cast<std::size_t>(X) > static_cast<std::size_t>(mqtt::max_payload))
		{
			asio2::set_last_error(asio::error::invalid_argument);
			return value;
		}

		asio2::clear_last_error();

		do
		{
			std::uint8_t encodedByte = X % 128;

			X = X / 128;

			// if there are more data to encode, set the top bit of this byte
			if (X > 0)
			{
				encodedByte = encodedByte | 128;
			}

			// 'output' encodedByte
			value[i] = encodedByte;

			++i;

		} while (X > 0);

		return value;
	}

	template<class Integer>
	inline bool check_size(Integer size)
	{
		return (std::size_t(size) <= std::size_t(65535));
	}

	template<class String>
	inline bool check_utf8(String& str)
	{
	#ifdef ASIO2_HEADER_ONLY
		namespace beast = ::bho::beast;
	#else
		namespace beast = ::boost::beast;
	#endif
	#if defined(ASIO2_CHECK_UTF8)
		return beast::websocket::detail::check_utf8(str.data(), str.size());
	#else
		asio2::detail::ignore_unused(str);
		return true;
	#endif
	}

	/*
	 * @return pair.first - the integer value, pair.second - number of bytes
	 * 
	 * The algorithm for decoding a Variable Byte Integer type is as follows:
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901011
	 */
	template<class BufferContainer>
	std::pair<std::int32_t, std::int32_t> decode_variable_byte_integer(BufferContainer X)
	{
		std::int32_t i = 0;

		std::int32_t multiplier = 1;

		std::int32_t value = 0;

		std::uint8_t encodedByte;

		do
		{
			if (X.size() < std::size_t(i + 1))
			{
				i = 0;
				value = 0;
				asio2::set_last_error(asio::error::no_buffer_space);
				return { value, i };
			}

			encodedByte = static_cast<std::uint8_t>(X[i]); // 'next byte from stream'

			++i;

			value += (encodedByte & 127) * multiplier;

			if (multiplier > 128 * 128 * 128)
			{
				i = 0;
				value = 0;
				asio2::set_last_error(asio::error::invalid_argument);
				return { value, i };
			}

			multiplier *= 128;

		} while ((encodedByte & 128) != 0);

		asio2::clear_last_error();

		// When this algorithm terminates, value contains the Variable Byte Integer value.
		return { value, i };
	}

	/**
	 * Bits in a byte are labelled 7 to 0. Bit number 7 is the most significant bit,
	 * the least significant bit is assigned bit number 0.
	 * 
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901007
	 */
	class one_byte_integer
	{
	public:
		using value_type = std::uint8_t;

		one_byte_integer() = default;

		explicit one_byte_integer(std::uint8_t v) : value_(v)
		{
		}

		inline std::uint8_t value() const noexcept
		{
			return value_;
		}

		inline operator std::uint8_t() const noexcept { return value(); }

		inline one_byte_integer& operator=(std::uint8_t v)
		{
			value_ = v;
			return (*this);
		}

		inline constexpr std::size_t required_size() const noexcept
		{
			return (sizeof(std::uint8_t));
		}

		inline constexpr std::size_t size() const noexcept
		{
			return (sizeof(std::uint8_t));
		}

		inline one_byte_integer& serialize(std::vector<asio::const_buffer>& buffers)
		{
			buffers.emplace_back(std::addressof(value_), required_size());
			return (*this);
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline one_byte_integer& serialize(Container& buffer)
		{
			static_assert(sizeof(typename Container::value_type) == std::size_t(1));

			auto* p = reinterpret_cast<typename Container::const_pointer>(std::addressof(value_));

			buffer.insert(buffer.end(), p, p + required_size());

			return (*this);
		}

		inline one_byte_integer& deserialize(std::string_view& data)
		{
			if (data.size() < required_size())
			{
				set_last_error(mqtt::make_error_code(mqtt::error::malformed_packet));
				return (*this);
			}

			asio2::clear_last_error();

			value_ = data[0];

			data.remove_prefix(required_size());

			return (*this);
		}

	protected:
		std::uint8_t value_{ 0 };
	};

	/**
	 * Two Byte Integer data values are 16-bit unsigned integers in big-endian order: the high order
	 * byte precedes the lower order byte. This means that a 16-bit word is presented on the network
	 * as Most Significant Byte (MSB), followed by Least Significant Byte (LSB).
	 * 
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901008
	 */
	class two_byte_integer
	{
	public:
		using value_type = std::uint16_t;

		two_byte_integer() = default;

		explicit two_byte_integer(std::uint16_t v)
		{
			*this = v;
		}

		inline std::uint16_t value() const noexcept
		{
		#if ASIO2_ENDIAN_BIG_BYTE
			return value_;
		#else
			return (((value_ >> 8) & 0x00ff) | ((value_ << 8) & 0xff00));
		#endif
		}

		inline operator std::uint16_t() const noexcept { return value(); }

		inline two_byte_integer& operator=(std::uint16_t v)
		{
		#if ASIO2_ENDIAN_BIG_BYTE
			value_ = v;
		#else
			value_ = ((v >> 8) & 0x00ff) | ((v << 8) & 0xff00);
		#endif
			return (*this);
		}

		inline constexpr std::size_t required_size() const noexcept
		{
			return (sizeof(std::uint16_t));
		}

		inline constexpr std::size_t size() const noexcept
		{
			return (sizeof(std::uint16_t));
		}

		inline two_byte_integer& serialize(std::vector<asio::const_buffer>& buffers)
		{
			buffers.emplace_back(std::addressof(value_), required_size());
			return (*this);
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline two_byte_integer& serialize(Container& buffer)
		{
			static_assert(sizeof(typename Container::value_type) == std::size_t(1));

			auto* p = reinterpret_cast<typename Container::const_pointer>(std::addressof(value_));

			buffer.insert(buffer.end(), p, p + required_size());

			return (*this);
		}

		inline two_byte_integer& deserialize(std::string_view& data)
		{
			if (data.size() < required_size())
			{
				set_last_error(mqtt::make_error_code(mqtt::error::malformed_packet));
				return (*this);
			}

			asio2::clear_last_error();

			value_ = ((data[1] << 8) & 0xff00) | ((data[0] << 0) & 0x00ff);

			data.remove_prefix(required_size());

			return (*this);
		}

	protected:
		std::uint16_t value_{ 0 };
	};

	/**
	 * Four Byte Integer data values are 32-bit unsigned integers in big-endian order: the high order
	 * byte precedes the successively lower order bytes. This means that a 32-bit word is presented on
	 * the network as Most Significant Byte (MSB), followed by the next most Significant Byte (MSB), 
	 * followed by the next most Significant Byte (MSB), followed by Least Significant Byte (LSB).
	 * 
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901009
	 */
	class four_byte_integer
	{
	public:
		using value_type = std::uint32_t;

		four_byte_integer() = default;

		explicit four_byte_integer(std::uint32_t v)
		{
			*this = v;
		}

		inline std::uint32_t value() const noexcept
		{
		#if ASIO2_ENDIAN_BIG_BYTE
			return value_;
		#else
			return (
				((value_ >> 24) & 0x000000ff) | 
				((value_ << 24) & 0xff000000) | 
				((value_ >>  8) & 0x0000ff00) | 
				((value_ <<  8) & 0x00ff0000));
		#endif
		}

		inline operator std::uint32_t() const noexcept { return value(); }

		inline four_byte_integer& operator=(std::uint32_t v)
		{
		#if ASIO2_ENDIAN_BIG_BYTE
			value_ = v;
		#else
			value_ = ((v >> 24) & 0x000000ff) | ((v << 24) & 0xff000000) | ((v >> 8) & 0x0000ff00) | ((v << 8) & 0x00ff0000);
		#endif
			return (*this);
		}

		inline constexpr std::size_t required_size() const noexcept
		{
			return (sizeof(std::uint32_t));
		}

		inline constexpr std::size_t size() const noexcept
		{
			return (sizeof(std::uint32_t));
		}

		inline four_byte_integer& serialize(std::vector<asio::const_buffer>& buffers)
		{
			buffers.emplace_back(std::addressof(value_), required_size());
			return (*this);
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline four_byte_integer& serialize(Container& buffer)
		{
			static_assert(sizeof(typename Container::value_type) == std::size_t(1));

			auto* p = reinterpret_cast<typename Container::const_pointer>(std::addressof(value_));

			buffer.insert(buffer.end(), p, p + required_size());

			return (*this);
		}

		inline four_byte_integer& deserialize(std::string_view& data)
		{
			if (data.size() < required_size())
			{
				set_last_error(mqtt::make_error_code(mqtt::error::malformed_packet));
				return (*this);
			}

			asio2::clear_last_error();

			value_ =
				((data[3] << 24) & 0xff000000) | 
				((data[2] << 16) & 0x00ff0000) | 
				((data[1] <<  8) & 0x0000ff00) | 
				((data[0] <<  0) & 0x000000ff);

			data.remove_prefix(required_size());

			return (*this);
		}

	protected:
		std::uint32_t value_{ 0 };
	};

	/**
	 * The Variable Byte Integer is encoded using an encoding scheme which uses a single byte for values up to 127.
	 * Larger values are handled as follows. The least significant seven bits of each byte encode the data, and
	 * the most significant bit is used to indicate whether there are bytes following in the representation.
	 * Thus, each byte encodes 128 values and a "continuation bit".
	 * The maximum number of bytes in the Variable Byte Integer field is four.
	 * The encoded value MUST use the minimum number of bytes necessary to represent the value [MQTT-1.5.5-1].
	 * This is shown in Table 1?1 Size of Variable Byte Integer.
	 * 
	 * Table 1-1 Size of Variable Byte Integer
	 * +---------+-------------------------------------+---------------------------------------+
	 * | Digits  | From	                               | To                                    |
	 * +---------+-------------------------------------+---------------------------------------+
	 * | 1       | 0 (0x00)                            | 127 (0x7F)                            |
	 * +---------+-------------------------------------+---------------------------------------+
	 * | 2       | 128 (0x80, 0x01)                    | 16,383 (0xFF, 0x7F)                   |
	 * +---------+-------------------------------------+---------------------------------------+
	 * | 3       | 16,384 (0x80, 0x80, 0x01)           | 2,097,151 (0xFF, 0xFF, 0x7F)          |
	 * +---------+-------------------------------------+---------------------------------------+
	 * | 4       | 2,097,152 (0x80, 0x80, 0x80, 0x01)  | 268,435,455 (0xFF, 0xFF, 0xFF, 0x7F)  |
	 * +---------+-------------------------------------+---------------------------------------+
	 * 
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901011
	 */
	class variable_byte_integer
	{
	public:
		using value_type = std::int32_t;

		variable_byte_integer() = default;

		explicit variable_byte_integer(std::int32_t v)
		{
			*this = v;
		}

		inline std::int32_t value() const
		{
			return std::get<0>(decode_variable_byte_integer(value_));
		}

		inline operator std::int32_t() const { return value(); }

		inline variable_byte_integer& operator=(std::int32_t v)
		{
			value_ = encode_variable_byte_integer(v);

			return (*this);
		}

		inline std::size_t required_size() const
		{
			return std::get<1>(decode_variable_byte_integer(value_));
		}

		inline std::size_t size() const
		{
			return std::get<1>(decode_variable_byte_integer(value_));
		}
		
		inline variable_byte_integer& serialize(std::vector<asio::const_buffer>& buffers)
		{
			buffers.emplace_back(value_.data(), required_size());
			return (*this);
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline variable_byte_integer& serialize(Container& buffer)
		{
			static_assert(sizeof(typename Container::value_type) == std::size_t(1));

			auto* p = reinterpret_cast<typename Container::const_pointer>(value_.data());

			buffer.insert(buffer.end(), p, p + required_size());

			return (*this);
		}

		inline variable_byte_integer& deserialize(std::string_view& data)
		{
			auto[value, bytes] = decode_variable_byte_integer(data);

			if (asio2::get_last_error())
				return (*this);

			asio2::detail::ignore_unused(value);

			std::memcpy((void*)value_.data(), (const void*)data.data(), bytes);

			data.remove_prefix(bytes);

			return (*this);
		}

	protected:
		// The Variable Byte Integer is encoded using an encoding scheme which uses a single byte for values
		// up to 127. Larger values are handled as follows. The least significant seven bits of each byte
		// encode the data, and the most significant bit is used to indicate whether there are bytes following
		// in the representation. Thus, each byte encodes 128 values and a "continuation bit". The maximum
		// number of bytes in the Variable Byte Integer field is four. The encoded value MUST use the minimum
		// number of bytes necessary to represent the value [MQTT-1.5.5-1]. 
		std::array<std::uint8_t, 4> value_{};
	};

	/**
	 * UTF-8 Encoded String
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901010
	 */
	class utf8_string
	{
	public:
		using value_type = std::string;
		using view_type  = std::string_view;

		utf8_string() = default;

		utf8_string(utf8_string const& o) : length_(o.length_), data_(o.data_)
		{
			//asio2::detail::disable_sso(data_);
		}
		utf8_string& operator=(utf8_string const& o)
		{
			length_ = o.length_;
			data_   = o.data_;

			//asio2::detail::disable_sso(data_);

			return (*this);
		}

		explicit utf8_string(const char* const s)
		{
			*this = std::string{ s };
		}

		explicit utf8_string(std::string s)
		{
			*this = std::move(s);
		}

		explicit utf8_string(std::string_view s)
		{
			*this = std::move(s);
		}

		inline utf8_string& operator=(std::string s)
		{
			if (!check_size(s.size()))
			{
				asio2::set_last_error(asio::error::invalid_argument);
				return (*this);
			}
			if (!check_utf8(s))
			{
				asio2::set_last_error(asio::error::invalid_argument);
				return (*this);
			}

			asio2::clear_last_error();

			data_   = std::move(s);
			length_ = static_cast<std::uint16_t>(data_.size());

			//asio2::detail::disable_sso(data_);

			return (*this);
		}

		inline utf8_string& operator=(std::string_view s)
		{
			*this = std::string{ s };

			return (*this);
		}

		inline utf8_string& operator=(const char* const s)
		{
			*this = std::string{ s };

			return (*this);
		}

		inline bool         operator==(const std::string      & s) noexcept { return (data_ == s); }
		inline bool         operator==(const std::string_view & s) noexcept { return (data_ == s); }
		inline bool         operator!=(const std::string      & s) noexcept { return (data_ != s); }
		inline bool         operator!=(const std::string_view & s) noexcept { return (data_ != s); }

		inline utf8_string& operator+=(const std::string      & s)
		{
			if (!check_size(data_.size() + s.size()))
			{
				asio2::set_last_error(asio::error::invalid_argument);
				return (*this);
			}
			if (!check_utf8(s))
			{
				asio2::set_last_error(asio::error::invalid_argument);
				return (*this);
			}

			asio2::clear_last_error();

			data_  += s;
			length_ = static_cast<std::uint16_t>(data_.size());

			//asio2::detail::disable_sso(data_);

			return (*this);
		}
		inline utf8_string& operator+=(const std::string_view & s)
		{
			if (!check_size(data_.size() + s.size()))
			{
				asio2::set_last_error(asio::error::invalid_argument);
				return (*this);
			}
			if (!check_utf8(s))
			{
				asio2::set_last_error(asio::error::invalid_argument);
				return (*this);
			}

			asio2::clear_last_error();

			data_  += s;
			length_ = static_cast<std::uint16_t>(data_.size());

			//asio2::detail::disable_sso(data_);

			return (*this);
		}
		inline utf8_string& operator+=(const char* const s)
		{
			this->operator+=(std::string_view{ s });

			return (*this);
		}

		// https://stackoverflow.com/questions/56833000/c20-with-u8-char8-t-and-stdstring
	#if defined(__cpp_lib_char8_t)
		explicit utf8_string(const char8_t* const s)
		{
			*this = std::u8string_view{ s };
		}

		explicit utf8_string(const std::u8string& s)
		{
			*this = s;
		}

		explicit utf8_string(const std::u8string_view& s)
		{
			*this = s;
		}

		inline utf8_string& operator=(const std::u8string& s)
		{
			*this = std::string{ s.begin(), s.end() };

			return (*this);
		}

		inline utf8_string& operator=(const std::u8string_view& s)
		{
			*this = std::string{ s.begin(), s.end() };

			return (*this);
		}

		inline utf8_string& operator=(const char8_t* const s)
		{
			*this = std::u8string_view{ s };

			return (*this);
		}

		inline bool         operator==(const std::u8string      & s) noexcept { return (data_ == std::string{s.begin(), s.end()}); }
		inline bool         operator==(const std::u8string_view & s) noexcept { return (data_ == std::string{s.begin(), s.end()}); }
		inline bool         operator!=(const std::u8string      & s) noexcept { return (data_ != std::string{s.begin(), s.end()}); }
		inline bool         operator!=(const std::u8string_view & s) noexcept { return (data_ != std::string{s.begin(), s.end()}); }

		inline utf8_string& operator+=(const std::u8string      & s)
		{
			*this += std::string{ s.begin(), s.end() };

			return (*this);
		}
		inline utf8_string& operator+=(const std::u8string_view & s)
		{
			*this += std::string{ s.begin(), s.end() };

			return (*this);
		}
		inline utf8_string& operator+=(const char8_t* const s)
		{
			*this += std::u8string_view{ s };

			return (*this);
		}
	#endif

		inline std::size_t  required_size() const noexcept
		{
			return (length_.required_size() + data_.size());
		}

		inline std::size_t  size() const noexcept
		{
			return (data_.size());
		}

		inline bool empty() const noexcept  { return data_.empty(); }

		inline std::string      data     () const { return data_; }
		inline std::string_view data_view() const { return data_; }

		inline operator std::string      () const { return data_; }
		inline operator std::string_view () const { return data_; }

		inline utf8_string& serialize(std::vector<asio::const_buffer>& buffers)
		{
			length_.serialize(buffers);

			buffers.emplace_back(asio::buffer(data_));

			return (*this);
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline utf8_string& serialize(Container& buffer)
		{
			length_.serialize(buffer);

			buffer.insert(buffer.end(), data_.begin(), data_.end());

			return (*this);
		}

		inline utf8_string& deserialize(std::string_view& data)
		{
			length_.deserialize(data);

			if (asio2::get_last_error())
				return (*this);

			if (data.size() < length_.value())
			{
				set_last_error(mqtt::make_error_code(mqtt::error::malformed_packet));
				return (*this);
			}

			asio2::clear_last_error();

			data_ = data.substr(0, length_.value());

			data.remove_prefix(data_.size());

			//asio2::detail::disable_sso(data_);

			return (*this);
		}

	protected:
		// Each of these strings is prefixed with a Two Byte Integer length field that gives the number of
		// bytes in a UTF-8 encoded string itself, as illustrated in Figure 1.1 Structure of UTF-8 Encoded
		// Strings below. Consequently, the maximum size of a UTF-8 Encoded String is 65,535 bytes.
		two_byte_integer length_{};

		// UTF-8 encoded character data, if length > 0.
		std::string      data_  {};
	};

	/**
	 * Binary Data
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901012
	 */
	class binary_data
	{
	public:
		using value_type = std::string;
		using view_type  = std::string_view;

		binary_data() = default;

		binary_data(binary_data const& o) : length_(o.length_), data_(o.data_)
		{
			//asio2::detail::disable_sso(data_);
		}
		binary_data& operator=(binary_data const& o)
		{
			length_ = o.length_;
			data_   = o.data_;

			//asio2::detail::disable_sso(data_);

			return (*this);
		}

		explicit binary_data(const char* const s)
		{
			*this = std::string{ s };
		}

		explicit binary_data(std::string s)
		{
			*this = std::move(s);
		}

		explicit binary_data(std::string_view s)
		{
			*this = std::move(s);
		}

		inline binary_data& operator=(std::string s)
		{
			if (!check_size(s.size()))
			{
				asio2::set_last_error(asio::error::invalid_argument);
				return (*this);
			}

			asio2::clear_last_error();

			data_   = std::move(s);
			length_ = static_cast<std::uint16_t>(data_.size());

			//asio2::detail::disable_sso(data_);

			return (*this);
		}

		inline binary_data& operator=(std::string_view s)
		{
			*this = std::string{ s };

			return (*this);
		}

		inline binary_data& operator=(const char* const s)
		{
			*this = std::string{ s };

			return (*this);
		}

		inline bool         operator==(const std::string      & s) noexcept { return (data_ == s); }
		inline bool         operator==(const std::string_view & s) noexcept { return (data_ == s); }
		inline bool         operator!=(const std::string      & s) noexcept { return (data_ != s); }
		inline bool         operator!=(const std::string_view & s) noexcept { return (data_ != s); }

		inline binary_data& operator+=(const std::string      & s)
		{
			if (!check_size(data_.size() + s.size()))
			{
				asio2::set_last_error(asio::error::invalid_argument);
				return (*this);
			}

			asio2::clear_last_error();

			data_  += s;
			length_ = static_cast<std::uint16_t>(data_.size());
			
			//asio2::detail::disable_sso(data_);

			return (*this);
		}
		inline binary_data& operator+=(const std::string_view & s)
		{
			if (!check_size(data_.size() + s.size()))
			{
				asio2::set_last_error(asio::error::invalid_argument);
				return (*this);
			}

			asio2::clear_last_error();

			data_  += s;
			length_ = static_cast<std::uint16_t>(data_.size());
			
			//asio2::detail::disable_sso(data_);

			return (*this);
		}
		inline binary_data& operator+=(const char* const s)
		{
			this->operator+=(std::string_view{ s });

			return (*this);
		}

	#if defined(__cpp_lib_char8_t)
		explicit binary_data(const char8_t* const s)
		{
			*this = std::u8string{ s };
		}

		explicit binary_data(const std::u8string& s)
		{
			*this = s;
		}

		explicit binary_data(const std::u8string_view& s)
		{
			*this = s;
		}

		inline binary_data& operator=(const std::u8string& s)
		{
			*this = std::string{ s.begin(), s.end() };

			return (*this);
		}

		inline binary_data& operator=(const std::u8string_view& s)
		{
			*this = std::string{ s.begin(), s.end() };

			return (*this);
		}

		inline binary_data& operator=(const char8_t* const s)
		{
			*this = std::u8string_view{ s };

			return (*this);
		}

		inline bool         operator==(const std::u8string      & s) noexcept { return (data_ == std::string(s.begin(), s.end())); }
		inline bool         operator==(const std::u8string_view & s) noexcept { return (data_ == std::string(s.begin(), s.end())); }
		inline bool         operator!=(const std::u8string      & s) noexcept { return (data_ != std::string(s.begin(), s.end())); }
		inline bool         operator!=(const std::u8string_view & s) noexcept { return (data_ != std::string(s.begin(), s.end())); }

		inline binary_data& operator+=(const std::u8string      & s)
		{
			*this += std::string{ s.begin(), s.end() };

			return (*this);
		}
		inline binary_data& operator+=(const std::u8string_view & s)
		{
			*this += std::string{ s.begin(), s.end() };

			return (*this);
		}
		inline binary_data& operator+=(const char8_t* const s)
		{
			*this += std::u8string_view{ s };

			return (*this);
		}
	#endif

		inline std::size_t  required_size() const noexcept
		{
			return (length_.required_size() + data_.size());
		}

		inline std::size_t  size() const noexcept
		{
			return (data_.size());
		}

		inline bool empty() const noexcept  { return data_.empty(); }

		inline std::string      data     () const { return data_; }
		inline std::string_view data_view() const { return data_; }

		inline operator std::string      () const { return data_; }
		inline operator std::string_view () const { return data_; }

		inline binary_data& serialize(std::vector<asio::const_buffer>& buffers)
		{
			length_.serialize(buffers);

			buffers.emplace_back(asio::buffer(data_));

			return (*this);
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline binary_data& serialize(Container& buffer)
		{
			length_.serialize(buffer);

			buffer.insert(buffer.end(), data_.begin(), data_.end());

			return (*this);
		}

		inline binary_data& deserialize(std::string_view& data)
		{
			length_.deserialize(data);

			if (asio2::get_last_error())
				return (*this);

			if (data.size() < length_.value())
			{
				set_last_error(mqtt::make_error_code(mqtt::error::malformed_packet));
				return (*this);
			}

			asio2::clear_last_error();

			data_ = data.substr(0, length_.value());

			data.remove_prefix(data_.size());

			//asio2::detail::disable_sso(data_);

			return (*this);
		}

	protected:
		// Binary Data is represented by a Two Byte Integer length which indicates the number of data bytes,
		// followed by that number of bytes. Thus, the length of Binary Data is limited to the range of 0 to
		// 65,535 Bytes.
		two_byte_integer length_{};

		// number of bytes
		std::string      data_  {};
	};

	/**
	 * A UTF-8 String Pair consists of two UTF-8 Encoded Strings.
	 * This data type is used to hold name-value pairs.
	 * The first string serves as the name, and the second string contains the value.
	 * 
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901013
	 */
	class utf8_string_pair
	{
	public:
		using value_type = std::pair<std::string, std::string>;
		using view_type  = std::pair<std::string_view, std::string_view>;

		utf8_string_pair() = default;

		explicit utf8_string_pair(const char* const key, const char* const val)
			: key_(std::move(key)), val_(std::move(val))
		{
		}

		explicit utf8_string_pair(std::string key, std::string val)
			: key_(std::move(key)), val_(std::move(val))
		{
		}

		explicit utf8_string_pair(std::string_view key, std::string_view val)
			: key_(std::move(key)), val_(std::move(val))
		{
		}

		inline utf8_string_pair& operator=(std::tuple<std::string, std::string> kv)
		{
			key_ = std::move(std::get<0>(kv));
			val_ = std::move(std::get<1>(kv));

			return (*this);
		}

		inline utf8_string_pair& operator=(std::pair<std::string, std::string> kv)
		{
			key_ = std::move(std::get<0>(kv));
			val_ = std::move(std::get<1>(kv));

			return (*this);
		}

		inline utf8_string_pair& operator=(std::tuple<std::string_view, std::string_view> kv)
		{
			key_ = std::move(std::get<0>(kv));
			val_ = std::move(std::get<1>(kv));

			return (*this);
		}

		inline utf8_string_pair& operator=(std::pair<std::string_view, std::string_view> kv)
		{
			key_ = std::move(std::get<0>(kv));
			val_ = std::move(std::get<1>(kv));

			return (*this);
		}

	#if defined(__cpp_lib_char8_t)
		explicit utf8_string_pair(const char8_t* const key, const char8_t* const val)
			: key_(std::move(key)), val_(std::move(val))
		{
		}

		explicit utf8_string_pair(const std::u8string& key, const std::u8string& val)
			: key_(key), val_(val)
		{
		}

		explicit utf8_string_pair(const std::u8string_view& key, const std::u8string_view& val)
			: key_(key), val_(val)
		{
		}

		inline utf8_string_pair& operator=(std::tuple<std::u8string, std::u8string> kv)
		{
			key_ = std::move(std::get<0>(kv));
			val_ = std::move(std::get<1>(kv));

			return (*this);
		}

		inline utf8_string_pair& operator=(std::pair<std::u8string, std::u8string> kv)
		{
			key_ = std::move(std::get<0>(kv));
			val_ = std::move(std::get<1>(kv));

			return (*this);
		}

		inline utf8_string_pair& operator=(std::tuple<std::u8string_view, std::u8string_view> kv)
		{
			key_ = std::move(std::get<0>(kv));
			val_ = std::move(std::get<1>(kv));

			return (*this);
		}

		inline utf8_string_pair& operator=(std::pair<std::u8string_view, std::u8string_view> kv)
		{
			key_ = std::move(std::get<0>(kv));
			val_ = std::move(std::get<1>(kv));

			return (*this);
		}
	#endif

		inline std::size_t  required_size() const noexcept
		{
			return (key_.required_size() + val_.required_size());
		}

		inline std::size_t  size() const noexcept
		{
			return (key_.size() + val_.size());
		}

		inline std::string key() const { return key_.data(); }
		inline std::string val() const { return val_.data(); }

		inline std::string_view key_view() const { return key_.data_view(); }
		inline std::string_view val_view() const { return val_.data_view(); }

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline utf8_string_pair& serialize(Container& buffer)
		{
			key_.serialize(buffer);
			val_.serialize(buffer);

			return (*this);
		}

		inline utf8_string_pair& deserialize(std::string_view& data)
		{
			key_.deserialize(data);

			if (asio2::get_last_error())
				return (*this);

			val_.deserialize(data);

			return (*this);
		}

	protected:
		utf8_string key_{};
		utf8_string val_{};
	};

	/**
	 * The Payload contains the Application Message that is being published.
	 * The content and format of the data is application specific.
	 * The length of the Payload can be calculated by subtracting the length of the Variable Header
	 * from the Remaining Length field that is in the Fixed Header.
	 * It is valid for a PUBLISH packet to contain a zero length Payload.
	 * 
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901119
	 */
	class application_message
	{
	public:
		using value_type = std::string;
		using view_type  = std::string_view;

		application_message() = default;

		application_message(application_message const& o) : data_(o.data_)
		{
			//asio2::detail::disable_sso(data_);
		}
		application_message& operator=(application_message const& o)
		{
			data_   = o.data_;

			//asio2::detail::disable_sso(data_);

			return (*this);
		}

		explicit application_message(const char* const s)
		{
			*this = std::string{ s };
		}

		explicit application_message(std::string s)
		{
			*this = std::move(s);
		}

		explicit application_message(std::string_view s)
		{
			*this = std::move(s);
		}

		inline application_message& operator=(std::string s)
		{
			if (s.size() > std::size_t(max_payload))
			{
				set_last_error(asio::error::invalid_argument);
				return (*this);
			}

			asio2::clear_last_error();

			data_   = std::move(s);

			//asio2::detail::disable_sso(data_);

			return (*this);
		}

		inline application_message& operator=(std::string_view s)
		{
			*this = std::string{ s };

			return (*this);
		}

		inline application_message& operator=(const char* const s)
		{
			*this = std::string{ s };

			return (*this);
		}

		inline bool         operator==(const std::string      & s) noexcept { return (data_ == s); }
		inline bool         operator==(const std::string_view & s) noexcept { return (data_ == s); }
		inline bool         operator!=(const std::string      & s) noexcept { return (data_ != s); }
		inline bool         operator!=(const std::string_view & s) noexcept { return (data_ != s); }

		inline application_message& operator+=(const std::string      & s)
		{
			if (data_.size() + s.size() > std::size_t(max_payload))
			{
				set_last_error(asio::error::invalid_argument);
				return (*this);
			}

			asio2::clear_last_error();

			data_  += s;

			//asio2::detail::disable_sso(data_);
			
			return (*this);
		}
		inline application_message& operator+=(const std::string_view & s)
		{
			if (data_.size() + s.size() > std::size_t(max_payload))
			{
				set_last_error(asio::error::invalid_argument);
				return (*this);
			}

			asio2::clear_last_error();

			data_  += s;

			//asio2::detail::disable_sso(data_);
			
			return (*this);
		}
		inline application_message& operator+=(const char* const s)
		{
			this->operator+=(std::string_view{ s });
			
			return (*this);
		}

	#if defined(__cpp_lib_char8_t)
		explicit application_message(const char8_t* const s)
		{
			*this = std::u8string{ s };
		}

		explicit application_message(const std::u8string& s)
		{
			*this = s;
		}

		explicit application_message(const std::u8string_view& s)
		{
			*this = s;
		}

		inline application_message& operator=(const std::u8string& s)
		{
			*this = std::string(s.begin(), s.end());

			return (*this);
		}

		inline application_message& operator=(const std::u8string_view& s)
		{
			*this = std::string(s.begin(), s.end());

			return (*this);
		}

		inline application_message& operator=(const char8_t* const s)
		{
			*this = std::u8string_view{ s };

			return (*this);
		}

		inline bool         operator==(const std::u8string      & s) noexcept { return (data_ == std::string(s.begin(), s.end())); }
		inline bool         operator==(const std::u8string_view & s) noexcept { return (data_ == std::string(s.begin(), s.end())); }
		inline bool         operator!=(const std::u8string      & s) noexcept { return (data_ != std::string(s.begin(), s.end())); }
		inline bool         operator!=(const std::u8string_view & s) noexcept { return (data_ != std::string(s.begin(), s.end())); }

		inline application_message& operator+=(const std::u8string      & s)
		{
			*this += std::string{ s.begin(), s.end() };
			
			return (*this);
		}
		inline application_message& operator+=(const std::u8string_view & s)
		{
			*this += std::string{ s.begin(), s.end() };
			
			return (*this);
		}
		inline application_message& operator+=(const char8_t* const s)
		{
			*this += std::u8string_view{ s };
			
			return (*this);
		}
	#endif

		inline std::size_t  required_size() const noexcept
		{
			return (data_.size());
		}

		inline std::size_t  size() const noexcept
		{
			return (data_.size());
		}

		inline bool empty() const noexcept  { return data_.empty(); }

		inline std::string      data     () const { return data_; }
		inline std::string_view data_view() const { return data_; }

		inline operator std::string      () const { return data_; }
		inline operator std::string_view () const { return data_; }

		inline application_message& serialize(std::vector<asio::const_buffer>& buffers)
		{
			buffers.emplace_back(asio::buffer(data_));

			return (*this);
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline application_message& serialize(Container& buffer)
		{
			buffer.insert(buffer.end(), data_.begin(), data_.end());

			return (*this);
		}

		inline application_message& deserialize(std::string_view& data)
		{
			data_ = data;

			data.remove_prefix(data_.size());

			//asio2::detail::disable_sso(data_);

			return (*this);
		}

	protected:
		std::string      data_{};
	};

	/**
	 * this class is just used for user application, it is not the part of mqtt protocol.
	 */
	template<class derived_t>
	class user_message_attr
	{
		//template <class> friend class asio2::detail::mqtt_handler_t;
		//template <class> friend class asio2::detail::mqtt_invoker_t;
		//template <class> friend class asio2::detail::mqtt_aop_connect;
		//template <class> friend class asio2::detail::mqtt_aop_publish;

	public:
		 user_message_attr() = default;
		~user_message_attr() = default;

	//protected:
		inline derived_t& set_send_flag(bool v) { send_flag_ = v; return (static_cast<derived_t&>(*this)); }
		inline bool       get_send_flag(      ) const { return send_flag_; }

	protected:
		bool send_flag_ = true;
	};

	/**
	 * Fixed header format
	 * Each MQTT Control Packet contains a Fixed Header as shown below.
	 * 
	 * http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718020
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901021
	 */
	template<std::uint8_t Version>
	class fixed_header : public user_message_attr<fixed_header<Version>>
	{
	public:
		union type_and_flags
		{
			one_byte_integer byte{ 0 };  // the whole byte
		#if ASIO2_ENDIAN_BIG_BYTE
			struct
			{
				std::uint8_t type   : 4; // MQTT Control Packet type
				bool         dup    : 1; // Duplicate delivery of a PUBLISH Control Packet
				std::uint8_t qos    : 2; // PUBLISH Quality of Service, 0, 1 or 2
				bool         retain : 1; // PUBLISH Retain flag
			} bits;
			struct
			{
				std::uint8_t type   : 4; // MQTT Control Packet type
				std::uint8_t bit3   : 1;
				std::uint8_t bit2   : 1;
				std::uint8_t bit1   : 1;
				std::uint8_t bit0   : 1;
			} reserved;
		#else
			struct
			{
				bool         retain : 1; // PUBLISH Retain flag
				std::uint8_t qos    : 2; // PUBLISH Quality of Service, 0, 1 or 2
				bool         dup    : 1; // Duplicate delivery of a PUBLISH Control Packet
				std::uint8_t type   : 4; // MQTT Control Packet type
			} bits;
			struct
			{
				std::uint8_t bit0   : 1;
				std::uint8_t bit1   : 1;
				std::uint8_t bit2   : 1;
				std::uint8_t bit3   : 1;
				std::uint8_t type   : 4; // MQTT Control Packet type
			} reserved;
		#endif
		};

	public:
		fixed_header()
		{
		}

		explicit fixed_header(control_packet_type type)
		{
			type_and_flags_.bits.type = asio2::detail::to_underlying(type);
		}

		constexpr std::uint8_t version() const noexcept { return Version; }

		inline std::size_t required_size() const
		{
			return (type_and_flags_.byte.required_size() + remain_length_.required_size());
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline fixed_header& serialize(Container& buffer)
		{
			type_and_flags_.byte.serialize(buffer);
			remain_length_      .serialize(buffer);

			return (*this);
		}

		inline fixed_header& deserialize(std::string_view& data)
		{
			type_and_flags_.byte.deserialize(data);

			if (asio2::get_last_error())
				return (*this);

			remain_length_      .deserialize(data);

			return (*this);
		}

		inline control_packet_type packet_type  () const { return static_cast<control_packet_type>(type_and_flags_.bits.type  ); }
		inline control_packet_type message_type () const { return packet_type(); }
		inline std::int32_t        remain_length() const { return remain_length_.value(); }

	protected:
		type_and_flags        type_and_flags_{   };

		// The Remaining Length is a Variable Byte Integer that represents the number of bytes remaining
		// within the current Control Packet, including data in the Variable Header and the Payload.
		// The Remaining Length does not include the bytes used to encode the Remaining Length.
		// The packet size is the total number of bytes in an MQTT Control Packet, this is equal to the
		// length of the Fixed Header plus the Remaining Length.
		variable_byte_integer remain_length_ { 0 };
	};

	/**
	 * Used to init a variant mqtt::message to empty.
	 */
	class nullmsg : public fixed_header<asio2::detail::to_underlying(mqtt::version::v4)>
	{
	public:
		nullmsg() : fixed_header(control_packet_type::reserved) {}

		inline nullmsg& update_remain_length() { return (*this); }


	};

	/**
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901168
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901248
	 */
	class subscription
	{
	public:
		subscription() = default;

		/*
		 * the "no_local,rap,retain_handling" are only valid in mqtt 5.0
		 */
		template<class String, class QosOrInt>
		explicit subscription(String&& topic_filter,
			QosOrInt qos, bool no_local = false, bool rap = false,
			retain_handling_type retain_handling = retain_handling_type::send
		)
			: topic_filter_(std::forward<String>(topic_filter))
		{
			option_.bits.qos             = static_cast<std::uint8_t>(qos);
			option_.bits.nl              = no_local;
			option_.bits.rap             = rap;
			option_.bits.retain_handling = asio2::detail::to_underlying(retain_handling);
		}

		inline std::size_t required_size() const
		{
			return (topic_filter_.required_size() + option_.byte.required_size());
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline subscription& serialize(Container& buffer)
		{
			topic_filter_.serialize(buffer);
			option_.byte .serialize(buffer);

			return (*this);
		}

		inline subscription& deserialize(std::string_view& data)
		{
			topic_filter_.deserialize(data);

			if (asio2::get_last_error())
				return (*this);

			option_.byte .deserialize(data);

			return (*this);
		}

		inline qos_type               qos            () const { return static_cast<qos_type>(option_.bits.qos); }
		inline bool                   no_local       () const { return option_.bits.nl             ; }
		inline bool                   rap            () const { return option_.bits.rap            ; }
		inline retain_handling_type   retain_handling() const { return static_cast<retain_handling_type>(option_.bits.retain_handling); }

		inline std::string_view       share_name     () const
		{
			auto[name, filter] = parse_topic_filter(topic_filter_.data_view());
			std::ignore = filter;
			return name;
		}
		inline std::string_view       topic_filter   () const
		{
			auto[name, filter] = parse_topic_filter(topic_filter_.data_view());
			std::ignore = name;
			return filter;
		}

		template<class QosOrInt>
		inline subscription& qos            (QosOrInt     v) { option_.bits.qos = static_cast<std::uint8_t>(v); return (*this); }
		inline subscription& no_local       (bool         v) { option_.bits.nl              = v; return (*this); }
		inline subscription& rap            (bool         v) { option_.bits.rap             = v; return (*this); }
		inline subscription& retain_handling(std::uint8_t v) { option_.bits.retain_handling = v; return (*this); }
		template<class String>
		inline subscription& topic_filter   (String&&     v) { topic_filter_ = std::forward<String>(v) ; return (*this); }

		inline subscription& retain_handling(retain_handling_type v) { option_.bits.retain_handling = asio2::detail::to_underlying(v); return (*this); }

	protected:
		// The Payload of a SUBSCRIBE packet contains a list of Topic Filters indicating the Topics to
		// which the Client wants to subscribe. The Topic Filters MUST be a UTF-8 Encoded String
		utf8_string      topic_filter_{};

		// Each Topic Filter is followed by a Subscription Options byte.
		union
		{
			one_byte_integer byte{ 0 };  // the whole byte
		#if ASIO2_ENDIAN_BIG_BYTE
			struct
			{
				std::uint8_t reserved        : 2; // reserved for future use
				std::uint8_t retain_handling : 2; // Retain Handling option,     [Only valid for mqtt 5.0]
				bool         rap             : 1; // Retain As Published option, [Only valid for mqtt 5.0]
				bool         nl              : 1; // No Local option,            [Only valid for mqtt 5.0]
				std::uint8_t qos             : 2; // PUBLISH Quality of Service, 0, 1 or 2
			} bits;
		#else
			struct
			{
				std::uint8_t qos             : 2; // PUBLISH Quality of Service, 0, 1 or 2
				bool         nl              : 1; // No Local option,            [Only valid for mqtt 5.0]
				bool         rap             : 1; // Retain As Published option, [Only valid for mqtt 5.0]
				std::uint8_t retain_handling : 2; // Retain Handling option,     [Only valid for mqtt 5.0]
				std::uint8_t reserved        : 2; // reserved for future use
			} bits;
		#endif
		}                option_{ };
	};

	/**
	 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901168
	 */
	class subscriptions_set
	{
	protected:
		template<class... Args>
		static constexpr bool _is_subscription() noexcept
		{
			if constexpr (sizeof...(Args) == std::size_t(0))
				return true;
			else
				return ((std::is_same_v<asio2::detail::remove_cvref_t<Args>, mqtt::subscription>) && ...);
		}

	public:
		subscriptions_set()
		{
		}

		template<class... Subscriptions, std::enable_if_t<_is_subscription<Subscriptions...>(), int> = 0>
		explicit subscriptions_set(Subscriptions&&... Subscripts)
		{
			set(std::forward<Subscriptions>(Subscripts)...);
		}

		subscriptions_set(subscriptions_set&&) noexcept = default;
		subscriptions_set(subscriptions_set const&) = default;
		subscriptions_set& operator=(subscriptions_set&&) noexcept = default;
		subscriptions_set& operator=(subscriptions_set const&) = default;

		template<class... Subscriptions, std::enable_if_t<_is_subscription<Subscriptions...>(), int> = 0>
		inline subscriptions_set& set(Subscriptions&&... Subscripts)
		{
			data_.clear();

			(data_.emplace_back(std::forward<Subscriptions>(Subscripts)), ...);

			return (*this);
		}

		template<class... Subscriptions, std::enable_if_t<_is_subscription<Subscriptions...>(), int> = 0>
		inline subscriptions_set& add(Subscriptions&&... Subscripts)
		{
			(data_.emplace_back(std::forward<Subscriptions>(Subscripts)), ...);

			return (*this);
		}

		inline subscriptions_set& erase(std::string_view topic_filter)
		{
			for (auto it = data_.begin(); it != data_.end();)
			{
				if (it->topic_filter() == topic_filter)
					it = data_.erase(it);
				else
					++it;
			}

			return (*this);
		}

		inline subscriptions_set& clear()
		{
			data_.clear();

			return (*this);
		}

		inline std::size_t required_size() const
		{
			std::size_t r = 0;
			for (auto& v : data_)
			{
				r += v.required_size();
			}
			return r;
		}

		inline std::size_t count() const noexcept
		{
			return data_.size();
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline subscriptions_set& serialize(Container& buffer)
		{
			for (auto& v : data_)
			{
				v.serialize(buffer);
			}

			return (*this);
		}

		inline subscriptions_set& deserialize(std::string_view& data)
		{
			while (!data.empty())
			{
				subscription s{};
				s.deserialize(data);

				if (asio2::get_last_error())
					return (*this);

				data_.emplace_back(std::move(s));
			}

			return (*this);
		}

		inline std::vector<subscription>& data() { return data_; }
		inline std::vector<subscription> const& data() const { return data_; }

	protected:
		std::vector<subscription> data_{};
	};

	class one_byte_integer_set
	{
	protected:
		template<class... Args>
		static constexpr bool _is_integral() noexcept
		{
			if constexpr (sizeof...(Args) == std::size_t(0))
				return true;
			else
				return ((std::is_integral_v<asio2::detail::remove_cvref_t<Args>>) && ...);
		}

	public:
		one_byte_integer_set()
		{
		}

		template<class... Integers, std::enable_if_t<_is_integral<Integers...>(), int> = 0>
		explicit one_byte_integer_set(Integers... Ints)
		{
			set(Ints...);
		}

		one_byte_integer_set(one_byte_integer_set&&) noexcept = default;
		one_byte_integer_set(one_byte_integer_set const&) = default;
		one_byte_integer_set& operator=(one_byte_integer_set&&) noexcept = default;
		one_byte_integer_set& operator=(one_byte_integer_set const&) = default;

		template<class... Integers, std::enable_if_t<_is_integral<Integers...>(), int> = 0>
		inline one_byte_integer_set& set(Integers... Ints)
		{
			data_.clear();

			(data_.emplace_back(static_cast<one_byte_integer::value_type>(Ints)), ...);

			return (*this);
		}

		template<class... Integers, std::enable_if_t<_is_integral<Integers...>(), int> = 0>
		inline one_byte_integer_set& add(Integers... Ints)
		{
			(data_.emplace_back(static_cast<one_byte_integer::value_type>(Ints)), ...);

			return (*this);
		}

		inline one_byte_integer_set& erase(std::size_t index)
		{
			if (index < data_.size())
				data_.erase(std::next(data_.begin(), index));

			return (*this);
		}

		inline one_byte_integer_set& clear()
		{
			data_.clear();

			return (*this);
		}

		inline std::size_t required_size() const
		{
			return (data_.empty() ? 0 : data_.size() * data_.front().required_size());
		}

		inline std::size_t count() const noexcept
		{
			return data_.size();
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline one_byte_integer_set& serialize(Container& buffer)
		{
			for (auto& v : data_)
			{
				v.serialize(buffer);
			}

			return (*this);
		}

		inline one_byte_integer_set& deserialize(std::string_view& data)
		{
			while (!data.empty())
			{
				one_byte_integer v{};
				v.deserialize(data);

				if (asio2::get_last_error())
					return (*this);

				data_.emplace_back(std::move(v));
			}

			return (*this);
		}

		inline std::vector<one_byte_integer>& data() { return data_; }
		inline std::vector<one_byte_integer> const& data() const { return data_; }

		inline one_byte_integer::value_type at(std::size_t i) const
		{
			return static_cast<one_byte_integer::value_type>(data_.size() > i ? data_[i].value() : -1);
		}

	protected:
		std::vector<one_byte_integer> data_{};
	};

	class utf8_string_set
	{
	public:
		utf8_string_set()
		{
		}

		template<class... Strings>
		explicit utf8_string_set(Strings&&... Strs)
		{
			set(std::forward<Strings>(Strs)...);
		}

		utf8_string_set(utf8_string_set&&) noexcept = default;
		utf8_string_set(utf8_string_set const&) = default;
		utf8_string_set& operator=(utf8_string_set&&) noexcept = default;
		utf8_string_set& operator=(utf8_string_set const&) = default;

		template<class... Strings>
		inline utf8_string_set& set(Strings&&... Strs)
		{
			data_.clear();

			(data_.emplace_back(std::forward<Strings>(Strs)), ...);

			return (*this);
		}

		template<class... Strings>
		inline utf8_string_set& add(Strings... Strs)
		{
			(data_.emplace_back(std::forward<Strings>(Strs)), ...);

			return (*this);
		}

		inline utf8_string_set& erase(std::string_view s)
		{
			for (auto it = data_.begin(); it != data_.end();)
			{
				if (it->data_view() == s)
					it = data_.erase(it);
				else
					++it;
			}

			return (*this);
		}

		inline utf8_string_set& clear()
		{
			data_.clear();

			return (*this);
		}

		inline std::size_t required_size() const
		{
			std::size_t r = 0;
			for (auto& v : data_)
			{
				r += v.required_size();
			}
			return r;
		}

		inline std::size_t count() const noexcept
		{
			return data_.size();
		}

		/*
		 * The Container is usually a std::string, std::vector<char>, ...
		 */
		template<class Container>
		inline utf8_string_set& serialize(Container& buffer)
		{
			for (auto& v : data_)
			{
				v.serialize(buffer);
			}

			return (*this);
		}

		inline utf8_string_set& deserialize(std::string_view& data)
		{
			while (!data.empty())
			{
				utf8_string s{};
				s.deserialize(data);

				if (asio2::get_last_error())
					return (*this);

				data_.emplace_back(std::move(s));
			}

			return (*this);
		}

		inline std::vector<utf8_string>& data() { return data_; }
		inline std::vector<utf8_string> const& data() const { return data_; }

		/**
		 * function signature : void(mqtt::utf8_string& str)
		 */
		template<class Function>
		inline utf8_string_set& for_each(Function&& f)
		{
			for (auto& v : data_)
			{
				f(v);
			}

			return (*this);
		}

	protected:
		std::vector<utf8_string> data_{};
	};

	namespace
	{
		using iterator = asio::buffers_iterator<asio::streambuf::const_buffers_type>;
		using diff_type = typename iterator::difference_type;
		std::pair<iterator, bool> mqtt_match_role(iterator begin, iterator end)
		{
			for (iterator p = begin; p < end;)
			{
				// Fixed header : at least 2 bytes
				if (end - p < static_cast<diff_type>(2))
					break;

				p += 1;

				// Remaining Length
				std::int32_t remain_length = 0, remain_bytes = 0;

				auto[value, bytes] = decode_variable_byte_integer(std::string_view{ reinterpret_cast<
					std::string_view::const_pointer>(p.operator->()), static_cast<std::size_t>(end - p) });

				if (error_code ec = asio2::get_last_error(); ec)
				{
					// need more data
					if (ec == asio::error::no_buffer_space)
					{
						return std::pair(begin, false);
					}
					// illegal packet
					else
					{
						return std::pair(begin, true);
					}
				}

				remain_length = value;
				remain_bytes  = bytes;

				p += remain_bytes;

				// Variable Header, Payload
				if (end - p < static_cast<diff_type>(remain_length))
					break;

				return std::pair(p + static_cast<diff_type>(remain_length), true);
			}
			return std::pair(begin, false);
		}
	}

	// Write the text for a one_byte_integer variable to an output stream.
	inline std::ostream& operator<<(std::ostream& os, one_byte_integer v)
	{
		std::string s;
		v.serialize(s);
		return os << std::move(s);
	}

	// Write the text for a two_byte_integer variable to an output stream.
	inline std::ostream& operator<<(std::ostream& os, two_byte_integer v)
	{
		std::string s;
		v.serialize(s);
		return os << std::move(s);
	}

	// Write the text for a four_byte_integer variable to an output stream.
	inline std::ostream& operator<<(std::ostream& os, four_byte_integer v)
	{
		std::string s;
		v.serialize(s);
		return os << std::move(s);
	}

	// Write the text for a variable_byte_integer variable to an output stream.
	inline std::ostream& operator<<(std::ostream& os, variable_byte_integer v)
	{
		std::string s;
		v.serialize(s);
		return os << std::move(s);
	}

	// Write the text for a utf8_string variable to an output stream.
	inline std::ostream& operator<<(std::ostream& os, utf8_string& v)
	{
		std::string s;
		v.serialize(s);
		return os << std::move(s);
	}

	// Write the text for a binary_data variable to an output stream.
	inline std::ostream& operator<<(std::ostream& os, binary_data& v)
	{
		std::string s;
		v.serialize(s);
		return os << std::move(s);
	}

	// Write the text for a utf8_string_pair variable to an output stream.
	inline std::ostream& operator<<(std::ostream& os, utf8_string_pair& v)
	{
		std::string s;
		v.serialize(s);
		return os << std::move(s);
	}

	// Write the text for a application_message variable to an output stream.
	inline std::ostream& operator<<(std::ostream& os, application_message& v)
	{
		return os << v.data_view();
	}
}

#endif // !__ASIO2_MQTT_CORE_HPP__
