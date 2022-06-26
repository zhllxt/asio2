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

#ifndef __ASIO2_MQTT_PROTOCOL_UTIL_HPP__
#define __ASIO2_MQTT_PROTOCOL_UTIL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/mqtt/protocol_v3.hpp>
#include <asio2/mqtt/protocol_v4.hpp>
#include <asio2/mqtt/protocol_v5.hpp>

namespace asio2::mqtt
{
	template<typename message_type>
	inline constexpr bool is_control_message()
	{
		return (is_v3_message<message_type>() || is_v4_message<message_type>() || is_v5_message<message_type>());
	}
}

#endif // !__ASIO2_MQTT_PROTOCOL_UTIL_HPP__
