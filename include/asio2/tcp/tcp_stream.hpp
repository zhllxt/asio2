/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_TCP_STREAM_HPP__
#define __ASIO2_TCP_STREAM_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/basic_stream.hpp>

namespace asio2
{
	/** A TCP/IP stream socket with timeouts and a polymorphic executor.

		@see basic_stream
	*/
	template<class RatePolicy = unlimited_rate_policy>
	using tcp_stream = basic_stream<asio::ip::tcp, asio::any_io_executor, RatePolicy>;
}

#endif // !__ASIO2_TCP_STREAM_HPP__
