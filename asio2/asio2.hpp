/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 * 
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
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
#include <asio2/http/ws_client.hpp>
#include <asio2/http/ws_server.hpp>
#include <asio2/rpc/rpc_client.hpp>
#include <asio2/rpc/rpc_server.hpp>
#include <asio2/icmp/ping.hpp>
#include <asio2/serial_port/serial_port.hpp>

#if defined(ASIO2_USE_SSL)
#	include <asio2/tcp/tcps_client.hpp>
#	include <asio2/tcp/tcps_server.hpp>
#	include <asio2/http/https_client.hpp>
#	include <asio2/http/https_server.hpp>
#	include <asio2/http/wss_client.hpp>
#	include <asio2/http/wss_server.hpp>
#endif

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_HPP__
