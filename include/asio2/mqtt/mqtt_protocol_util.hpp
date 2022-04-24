/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * chinese : http://mqtt.p2hp.com/mqtt-5-0
 * english : https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_MQTT_PROTOCOL_PARSER_HPP__
#define __ASIO2_MQTT_PROTOCOL_PARSER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/mqtt/mqtt_protocol_v3.hpp>
#include <asio2/mqtt/mqtt_protocol_v4.hpp>
#include <asio2/mqtt/mqtt_protocol_v5.hpp>

namespace asio2::mqtt
{
	template<class Byte>
	inline mqtt::control_packet_type packet_type_from_byte(Byte byte)
	{
		mqtt::fixed_header<0>::type_and_flags tf{};
		tf.byte = static_cast<std::uint8_t>(byte);
		return static_cast<mqtt::control_packet_type>(tf.bits.type);
	}

	template<class Byte>
	inline mqtt::control_packet_type message_type_from_byte(Byte byte)
	{
		return packet_type_from_byte(byte);
	}

	template<mqtt::version V, class Fun>
	inline void data_to_packet(std::string_view& data, Fun&& f)
	{
		mqtt::control_packet_type type = mqtt::message_type_from_byte(data.front());

		if constexpr /**/ (V == mqtt::version::v3)
		{
			switch (type)
			{
			case mqtt::control_packet_type::connect     : { mqtt::v3::connect     v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::connack     : { mqtt::v3::connack     v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::publish     : { mqtt::v3::publish     v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::puback      : { mqtt::v3::puback      v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::pubrec      : { mqtt::v3::pubrec      v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::pubrel      : { mqtt::v3::pubrel      v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::pubcomp     : { mqtt::v3::pubcomp     v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::subscribe   : { mqtt::v3::subscribe   v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::suback      : { mqtt::v3::suback      v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::unsubscribe : { mqtt::v3::unsubscribe v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::unsuback    : { mqtt::v3::unsuback    v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::pingreq     : { mqtt::v3::pingreq     v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::pingresp    : { mqtt::v3::pingresp    v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::disconnect  : { mqtt::v3::disconnect  v{}; v.deserialize(data); f(std::move(v)); } break;
			default:
				ASIO2_ASSERT(false);
				asio::detail::throw_error(mqtt::make_error_code(mqtt::error::malformed_packet));
				break;
			}
		}
		else if constexpr (V == mqtt::version::v4)
		{
			switch (type)
			{
			case mqtt::control_packet_type::connect     : { mqtt::v4::connect     v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::connack     : { mqtt::v4::connack     v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::publish     : { mqtt::v4::publish     v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::puback      : { mqtt::v4::puback      v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::pubrec      : { mqtt::v4::pubrec      v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::pubrel      : { mqtt::v4::pubrel      v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::pubcomp     : { mqtt::v4::pubcomp     v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::subscribe   : { mqtt::v4::subscribe   v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::suback      : { mqtt::v4::suback      v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::unsubscribe : { mqtt::v4::unsubscribe v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::unsuback    : { mqtt::v4::unsuback    v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::pingreq     : { mqtt::v4::pingreq     v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::pingresp    : { mqtt::v4::pingresp    v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::disconnect  : { mqtt::v4::disconnect  v{}; v.deserialize(data); f(std::move(v)); } break;
			default:
				ASIO2_ASSERT(false);
				asio::detail::throw_error(mqtt::make_error_code(mqtt::error::malformed_packet));
				break;
			}
		}
		else if constexpr (V == mqtt::version::v5)
		{
			switch (type)
			{
			case mqtt::control_packet_type::connect     : { mqtt::v5::connect     v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::connack     : { mqtt::v5::connack     v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::publish     : { mqtt::v5::publish     v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::puback      : { mqtt::v5::puback      v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::pubrec      : { mqtt::v5::pubrec      v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::pubrel      : { mqtt::v5::pubrel      v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::pubcomp     : { mqtt::v5::pubcomp     v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::subscribe   : { mqtt::v5::subscribe   v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::suback      : { mqtt::v5::suback      v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::unsubscribe : { mqtt::v5::unsubscribe v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::unsuback    : { mqtt::v5::unsuback    v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::pingreq     : { mqtt::v5::pingreq     v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::pingresp    : { mqtt::v5::pingresp    v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::disconnect  : { mqtt::v5::disconnect  v{}; v.deserialize(data); f(std::move(v)); } break;
			case mqtt::control_packet_type::auth        : { mqtt::v5::auth        v{}; v.deserialize(data); f(std::move(v)); } break;
			default:
				ASIO2_ASSERT(false);
				asio::detail::throw_error(mqtt::make_error_code(mqtt::error::malformed_packet));
				break;
			}
		}
		else
		{
			ASIO2_ASSERT(false);
			asio::detail::throw_error(mqtt::make_error_code(mqtt::error::malformed_packet));
		}

		ASIO2_ASSERT(data.empty());
	}

	template<mqtt::version V, class Fun>
	inline void data_to_message(std::string_view& data, Fun&& f)
	{
		return data_to_packet<V>(data, std::forward<Fun>(f));
	}

	template<typename = void>
	inline mqtt::version version_from_connect_data(std::string_view data)
	{
		try
		{
			mqtt::fixed_header<0> header{};
			header.deserialize(data);

			// The Protocol Name is a UTF-8 Encoded String that represents the protocol name ¡°MQTT¡±. 
			// The string, its offset and length will not be changed by future versions of the MQTT specification.
			mqtt::utf8_string protocol_name{};
			protocol_name.deserialize(data);

			// The 8 bit unsigned value that represents the revision level of the protocol used by the Client. 
			// The value of the Protocol Level field for the version 3.1.1 of the protocol is 4 (0x04).
			mqtt::one_byte_integer protocol_version{};
			protocol_version.deserialize(data);

			return static_cast<mqtt::version>(protocol_version.value());
		}
		catch (system_error const&) {}

		return static_cast<mqtt::version>(0);
	}

	template<typename message_type>
	inline constexpr bool is_v3_message()
	{
		using type = asio2::detail::remove_cvref_t<message_type>;
		if constexpr (
			std::is_same_v<type, mqtt::v3::connect     > ||
			std::is_same_v<type, mqtt::v3::connack     > ||
			std::is_same_v<type, mqtt::v3::publish     > ||
			std::is_same_v<type, mqtt::v3::puback      > ||
			std::is_same_v<type, mqtt::v3::pubrec      > ||
			std::is_same_v<type, mqtt::v3::pubrel      > ||
			std::is_same_v<type, mqtt::v3::pubcomp     > ||
			std::is_same_v<type, mqtt::v3::subscribe   > ||
			std::is_same_v<type, mqtt::v3::suback      > ||
			std::is_same_v<type, mqtt::v3::unsubscribe > ||
			std::is_same_v<type, mqtt::v3::unsuback    > ||
			std::is_same_v<type, mqtt::v3::pingreq     > ||
			std::is_same_v<type, mqtt::v3::pingresp    > ||
			std::is_same_v<type, mqtt::v3::disconnect  > )
		{
			return true;
		}
		else
		{
			return false;
		}
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

#endif // !__ASIO2_MQTT_PROTOCOL_PARSER_HPP__
