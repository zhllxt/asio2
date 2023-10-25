//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BHO_BEAST_CORE_TCP_STREAM_HPP
#define BHO_BEAST_CORE_TCP_STREAM_HPP

#include <asio2/bho/beast/core/detail/config.hpp>
#include <asio2/bho/beast/core/basic_stream.hpp>
#include <asio2/bho/beast/core/rate_policy.hpp>
#include <asio/executor.hpp>
#include <asio/ip/tcp.hpp>

namespace bho {
namespace beast {

/** A TCP/IP stream socket with timeouts and a polymorphic executor.

    @see basic_stream
*/
using tcp_stream = basic_stream<
    net::ip::tcp,
    net::any_io_executor,
    unlimited_rate_policy>;

} // beast
} // bho

#endif
