/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_TCP_KEEPALIVE_COMPONENT_HPP__
#define __ASIO2_TCP_KEEPALIVE_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/keepalive_options.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t>
	class tcp_keepalive_cp
	{
	public:
		 tcp_keepalive_cp() noexcept {}
		~tcp_keepalive_cp() noexcept {}

		/**
		 * @brief set tcp socket keep alive options
		 * @param onoff    - Turn keepalive on or off.
		 * @param idle     - How many seconds after the connection is idle, start sending keepalives.
		 * @param interval - How many seconds later to send again when no reply is received.
		 * @param count    - How many times to resend when no reply is received.
		 * @li on macOS Catalina 10.15.5 (19F101), the default value is:
		 * onoff - false, idle - 7200, interval - 75, count - 8
		 */
		bool set_keep_alive_options(
			bool         onoff = true,
			unsigned int idle = 60,
			unsigned int interval = 3,
			unsigned int count = 3
		) noexcept
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return detail::set_keepalive_options(derive.socket(), onoff, idle, interval, count);
		}
	};
}

#endif // !__ASIO2_TCP_KEEPALIVE_COMPONENT_HPP__
