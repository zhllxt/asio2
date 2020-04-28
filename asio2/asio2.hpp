/*
 * COPYRIGHT (C) 2017-2019, zhllxt
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

#if defined(_MSC_VER) && !defined(NOMINMAX)
#  define ASIO2_NOMINMAX
#  define NOMINMAX
#endif

#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable:4191) // asio inner : from FARPROC to cancel_io_ex_t is unsafe
#endif

#include <asio2/version.hpp>
#include <asio2/config.hpp>

#include <asio2/base/timer.hpp>
#include <asio2/tcp/tcp_client.hpp>
#include <asio2/tcp/tcp_server.hpp>
#include <asio2/udp/udp_client.hpp>
#include <asio2/udp/udp_server.hpp>
#include <asio2/udp/udp_cast.hpp>
#include <asio2/rpc/rpc_client.hpp>
#include <asio2/rpc/rpc_server.hpp>
#include <asio2/icmp/ping.hpp>
#include <asio2/scp/scp.hpp>

#if defined(ASIO2_USE_SSL)
	#include <asio2/tcp/tcps_client.hpp>
	#include <asio2/tcp/tcps_server.hpp>
#endif

#ifndef ASIO_STANDALONE
	#include <asio2/http/http_client.hpp>
	#include <asio2/http/http_server.hpp>
	#include <asio2/http/ws_client.hpp>
	#include <asio2/http/ws_server.hpp>
	#include <asio2/http/httpws_server.hpp>
	#if defined(ASIO2_USE_SSL)
		#include <asio2/http/https_client.hpp>
		#include <asio2/http/https_server.hpp>
		#include <asio2/http/wss_client.hpp>
		#include <asio2/http/wss_server.hpp>
		#include <asio2/http/httpwss_server.hpp>
	#endif
#endif // ASIO_STANDALONE

#if defined(_MSC_VER)
#  pragma warning(pop)
#endif

#if defined(_MSC_VER) && defined(NOMINMAX) && defined(ASIO2_NOMINMAX)
#  undef NOMINMAX
#  undef ASIO2_NOMINMAX
#endif

#endif // !__ASIO2_HPP__
