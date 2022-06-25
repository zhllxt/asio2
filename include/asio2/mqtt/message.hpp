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

#ifndef __ASIO2_MQTT_MESSAGE_HPP__
#define __ASIO2_MQTT_MESSAGE_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/mqtt/protocol_util.hpp>

namespace asio2::mqtt
{
	using message_variant = std::variant<
		v3::connect     , // 0
		v3::connack     , // 1
		v3::publish     , // 2
		v3::puback      , // 3
		v3::pubrec      , // 4
		v3::pubrel      , // 5
		v3::pubcomp     , // 6
		v3::subscribe   , // 7
		v3::suback      , // 8
		v3::unsubscribe , // 9
		v3::unsuback    , // 10
		v3::pingreq     , // 11
		v3::pingresp    , // 12
		v3::disconnect  , // 13

		v4::connect     , // 14
		v4::connack     , // 15
		v4::publish     , // 16
		v4::puback      , // 17
		v4::pubrec      , // 18
		v4::pubrel      , // 19
		v4::pubcomp     , // 20
		v4::subscribe   , // 21
		v4::suback      , // 22
		v4::unsubscribe , // 23
		v4::unsuback    , // 24
		v4::pingreq     , // 25
		v4::pingresp    , // 26
		v4::disconnect  , // 27

		v5::connect     , // 28
		v5::connack     , // 29
		v5::publish     , // 30
		v5::puback      , // 31
		v5::pubrec      , // 32
		v5::pubrel      , // 33
		v5::pubcomp     , // 34
		v5::subscribe   , // 35
		v5::suback      , // 36
		v5::unsubscribe , // 37
		v5::unsuback    , // 38
		v5::pingreq     , // 39
		v5::pingresp    , // 40
		v5::disconnect  , // 41
		v5::auth          // 42
	>;

	class message : public message_variant
	{
	public:
		using super = message_variant;

		// a default-constructed variant holds a value of its first alternative
		message() noexcept : message_variant()
		{
		}

		template<class T, std::enable_if_t<is_control_message<T>(), int> = 0>
		message(T&& v) : message_variant(std::forward<T>(v))
		{
		}

		template<class T, std::enable_if_t<is_control_message<T>(), int> = 0>
		message& operator=(T&& v)
		{
			this->base() = std::forward<T>(v);
			return (*this);
		}

		message(message&&) noexcept = default;
		message(message const&) = default;
		message& operator=(message&&) noexcept = default;
		message& operator=(message const&) = default;

		template<class T, std::enable_if_t<is_control_message<T>(), int> = 0>
		operator T&()
		{
			return std::get<T>(this->base());
		}

		template<class T, std::enable_if_t<is_control_message<T>(), int> = 0>
		operator const T&()
		{
			return std::get<T>(this->base());
		}

		template<class T, std::enable_if_t<is_control_message<T>(), int> = 0>
		operator T*() noexcept
		{
			return std::get_if<T>(std::addressof(this->base()));
		}

		template<class T, std::enable_if_t<is_control_message<T>(), int> = 0>
		operator const T*() noexcept
		{
			return std::get_if<T>(std::addressof(this->base()));
		}

		template<class T, std::enable_if_t<is_control_message<T>(), int> = 0>
		operator T()
		{
			return std::get<T>(this->base());
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
		 * @function Checks if the variant holds the alternative T. 
		 */
		template<class T>
		inline bool has() noexcept
		{
			return std::holds_alternative<T>(this->base());
		}

		/**
		 * @function If this holds the alternative T, returns a pointer to the value stored in the variant.
		 * Otherwise, returns a null pointer value.
		 */
		template<class T>
		inline std::add_pointer_t<T> get_if() noexcept
		{
			return std::get_if<T>(std::addressof(this->base()));
		}

		/**
		 * @function If this holds the alternative T, returns a pointer to the value stored in the variant.
		 * Otherwise, returns a null pointer value.
		 */
		template<class T>
		inline std::add_pointer_t<const T> get_if() noexcept
		{
			return std::get_if<T>(std::addressof(this->base()));
		}

		/**
		 * @function If this holds the alternative T, returns a reference to the value stored in the variant.
		 * Otherwise, throws std::bad_variant_access.
		 */
		template<class T>
		inline T& get()
		{
			return std::get<T>(this->base());
		}

		/**
		 * @function If this holds the alternative T, returns a reference to the value stored in the variant.
		 * Otherwise, throws std::bad_variant_access.
		 */
		template<class T>
		inline const T& get()
		{
			return std::get<T>(this->base());
		}

	protected:
	};
}

#endif // !__ASIO2_MQTT_MESSAGE_HPP__
