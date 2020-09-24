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

// the tests trigger deprecation warnings when compiled with msvc in C++17 mode
// warning STL4009: std::allocator<void> is deprecated in C++17
#if defined(_MSVC_LANG) && _MSVC_LANG > 201402
#	define _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
#	define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
#	define _SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING
#	define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#endif

#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable:4191) // asio inner : from FARPROC to cancel_io_ex_t is unsafe
#  pragma warning(disable:4996) // warning STL4009: std::allocator<void> is deprecated in C++17
#endif

/*
 * see : https://github.com/retf/Boost.Application/pull/40
 */
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
#   ifndef _WIN32_WINNT
#       include <winsdkver.h>
#       define _WIN32_WINNT _WIN32_WINNT_WIN7
#       include <SDKDDKVer.h>
#   endif
#endif

#include <asio2/version.hpp>
#include <asio2/config.hpp>

#include <asio2/base/selector.hpp>
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
#include <asio2/scp/scp.hpp>

#if defined(ASIO2_USE_SSL)
#	include <asio2/tcp/tcps_client.hpp>
#	include <asio2/tcp/tcps_server.hpp>
#	include <asio2/http/https_client.hpp>
#	include <asio2/http/https_server.hpp>
#	include <asio2/http/wss_client.hpp>
#	include <asio2/http/wss_server.hpp>
#endif

#if defined(_MSC_VER)
#  pragma warning(pop)
#endif

#endif // !__ASIO2_HPP__
