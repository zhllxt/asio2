/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_TCP_KEEPALIVE_COMPONENT_HPP__
#define __ASIO2_TCP_KEEPALIVE_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <tuple>

#include <asio2/base/selector.hpp>
#include <asio2/base/error.hpp>

#if defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_) || defined(WIN32)
#include <Mstcpip.h> // tcp_keepalive struct
#endif

namespace asio2::detail
{
	template<class socket_t>
	class tcp_keepalive_cp
	{
	public:
		tcp_keepalive_cp(socket_t & socket) : socket_ref_(socket) {}
		~tcp_keepalive_cp() = default;

		/**
		 * @function : set tcp socket keep alive options
		 * @param    : onoff    - Turn keepalive on or off
		 * @param    : idle     - How many seconds after the connection is idle, start sending keepalives
		 * @param    : interval - How many seconds later to send again when no reply is received
		 * @param    : count    - How many times to resend when no reply is received
		 */
		bool keep_alive_options(
			bool         onoff = true,
			unsigned int idle = 60,
			unsigned int interval = 3,
			unsigned int count = 3
		)
		{
			try
			{
				std::ignore = count;

				auto & socket = this->socket_ref_.lowest_layer();
				if (!socket.is_open())
				{
					set_last_error(asio::error::not_connected);
					return false;
				}

				asio::socket_base::keep_alive option(onoff);
				socket.set_option(option);

				auto native_fd = socket.native_handle();

#if defined(__unix__) || defined(__linux__)
				// For *n*x systems
				int ret_keepidle = setsockopt(native_fd, SOL_TCP, TCP_KEEPIDLE, (void*)&idle, sizeof(unsigned int));
				int ret_keepintvl = setsockopt(native_fd, SOL_TCP, TCP_KEEPINTVL, (void*)&interval, sizeof(unsigned int));
				int ret_keepinit = setsockopt(native_fd, SOL_TCP, TCP_KEEPCNT, (void*)&count, sizeof(unsigned int));

				if (ret_keepidle || ret_keepintvl || ret_keepinit)
				{
					set_last_error(errno);
					return false;
				}
#elif defined(__OSX__)
				//// Set the timeout before the first keep alive message
				//int ret_tcpkeepalive = setsockopt(native_fd, IPPROTO_TCP, TCP_KEEPALIVE, (void*)&idle, sizeof(unsigned int));
				//int ret_tcpkeepintvl = setsockopt(native_fd, IPPROTO_TCP, TCP_CONNECTIONTIMEOUT, (void*)&interval, sizeof(unsigned int));

				//if (ret_tcpkeepalive || ret_tcpkeepintvl)
				//{
				//	set_last_error(errno);
				//	return false;
				//}
#elif defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_) || defined(WIN32)
				// Partially supported on windows
				tcp_keepalive keepalive_options;
				keepalive_options.onoff = onoff;
				keepalive_options.keepalivetime = idle * 1000; // Keep Alive in milliseconds.
				keepalive_options.keepaliveinterval = interval * 1000; // Resend if No-Reply 

				DWORD bytes_returned = 0;

				if (SOCKET_ERROR == ::WSAIoctl(native_fd, SIO_KEEPALIVE_VALS, (LPVOID)&keepalive_options,
					(DWORD)sizeof(keepalive_options), nullptr, 0, (LPDWORD)&bytes_returned, nullptr, nullptr))
				{
					if (::WSAGetLastError() != WSAEWOULDBLOCK)
					{
						set_last_error(::WSAGetLastError());
						return false;
					}
				}
#endif
				return true;
			}
			catch (system_error & e)
			{
				set_last_error(e);
			}
			return false;
		}

	private:
		socket_t & socket_ref_;
	};
}

#endif // !__ASIO2_TCP_KEEPALIVE_COMPONENT_HPP__
