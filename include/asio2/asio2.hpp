/*
 * Copyright (c) 2017-2023 zhllxt
 * 
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_HPP__
#define __ASIO2_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/version.hpp>
#include <asio2/config.hpp>

#include <asio2/base/timer.hpp>
#include <asio2/tcp/tcp_client.hpp>
#include <asio2/tcp/tcp_server.hpp>
#include <asio2/udp/udp_client.hpp>
#include <asio2/udp/udp_server.hpp>
#include <asio2/udp/udp_cast.hpp>
#include <asio2/http/http_client.hpp>
#include <asio2/http/http_server.hpp>
#include <asio2/websocket/ws_client.hpp>
#include <asio2/websocket/ws_server.hpp>
#include <asio2/rpc/rpc_client.hpp>
#include <asio2/rpc/rpc_server.hpp>
#include <asio2/icmp/ping.hpp>
#include <asio2/serial_port/serial_port.hpp>

#if defined(ASIO2_ENABLE_SSL) || defined(ASIO2_USE_SSL)
#	include <asio2/tcp/tcps_client.hpp>
#	include <asio2/tcp/tcps_server.hpp>
#	include <asio2/http/https_client.hpp>
#	include <asio2/http/https_server.hpp>
#	include <asio2/websocket/wss_client.hpp>
#	include <asio2/websocket/wss_server.hpp>
#	include <asio2/rpc/rpcs_client.hpp>
#	include <asio2/rpc/rpcs_server.hpp>
#endif

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_HPP__
