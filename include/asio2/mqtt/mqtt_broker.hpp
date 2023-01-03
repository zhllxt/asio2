/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
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
