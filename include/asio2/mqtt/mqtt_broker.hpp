/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_MQTT_BROKER_HPP__
#define __ASIO2_MQTT_BROKER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/mqtt/mqtt_server.hpp>

namespace asio2
{
	template<class session_t>
	using mqtt_broker_t = mqtt_server_t<session_t>;

	using mqtt_broker = mqtt_server;
}

#endif // !__ASIO2_MQTT_BROKER_HPP__
