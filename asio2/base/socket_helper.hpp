/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_GLOBAL_HPP__
#define __ASIO2_GLOBAL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if !defined(NDEBUG) && !defined(DEBUG) && !defined(_DEBUG)
#	define NDEBUG
#endif

#include <cassert>

#include <boost/asio.hpp>
#include <boost/system/system_error.hpp>

#if   defined(LINUX)
#elif defined(__OSX__)
#elif defined(WINDOWS)
#	include <mstcpip.h>
#endif

#include <asio2/base/error.hpp>
#include <asio2/base/def.hpp>

#include <asio2/util/def.hpp>

namespace asio2
{

	/**
	 * @function : set tcp socket keep alive options
	 * @param    : onoff    - turn on or turn off
	 * @param    : timeout  - check time out
	 * @param    : interval - check interval
	 * @param    : count    - check times
	 */
	template<typename socket_t>
	bool set_keepalive_vals(
		socket_t   & socket,
		bool         onoff    = true,
		unsigned int timeout  = 30 * 1000,
		unsigned int interval = 10 * 1000,
		unsigned int count    = 3
	)
	{
		if (!socket.is_open())
		{
			assert(false);
			return false;
		}

		boost::asio::socket_base::keep_alive option(onoff);
		socket.set_option(option);

		auto native_fd = socket.native_handle();

#if defined(LINUX)
		// For *n*x systems
		int ret_keepidle = setsockopt(native_fd, SOL_TCP, TCP_KEEPIDLE, (void*)&timeout, sizeof(unsigned int));
		int ret_keepintvl = setsockopt(native_fd, SOL_TCP, TCP_KEEPINTVL, (void*)&interval, sizeof(unsigned int));
		int ret_keepinit = setsockopt(native_fd, SOL_TCP, TCP_KEEPCNT, (void*)&count, sizeof(unsigned int));

		if (ret_keepidle || ret_keepintvl || ret_keepinit)
		{
			return false;
		}

#elif defined(__OSX__)
		// Set the timeout before the first keep alive message
		int ret_tcpkeepalive = setsockopt(native_fd, IPPROTO_TCP, TCP_KEEPALIVE, (void*)&timeout, sizeof(unsigned int));
		int ret_tcpkeepintvl = setsockopt(native_fd, IPPROTO_TCP, TCP_CONNECTIONTIMEOUT, (void*)&interval, sizeof(unsigned int));

		if (ret_tcpkeepalive || ret_tcpkeepintvl)
		{
			return false;
		}

#elif defined(WINDOWS)
		// Partially supported on windows
		tcp_keepalive keepalive_options;
		keepalive_options.onoff = onoff;
		keepalive_options.keepalivetime = timeout;
		keepalive_options.keepaliveinterval = interval;

		DWORD bytes_returned = 0;

		if (SOCKET_ERROR == ::WSAIoctl(native_fd, SIO_KEEPALIVE_VALS, (LPVOID)&keepalive_options, (DWORD)sizeof(keepalive_options),
			nullptr, 0, (LPDWORD)&bytes_returned, nullptr, nullptr))
		{
			if (::WSAGetLastError() != WSAEWOULDBLOCK)
				return false;
		}
#endif
		return true;
	}

}

#endif // !__ASIO2_GLOBAL_HPP__
