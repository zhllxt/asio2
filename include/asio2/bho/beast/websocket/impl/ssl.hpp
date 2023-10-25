//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BHO_BEAST_WEBSOCKET_IMPL_SSL_HPP
#define BHO_BEAST_WEBSOCKET_IMPL_SSL_HPP

#include <utility>
#include <asio2/bho/beast/websocket/teardown.hpp>
#include <asio/compose.hpp>
#include <asio/coroutine.hpp>

namespace bho {
namespace beast {

/*

    See
    http://stackoverflow.com/questions/32046034/what-is-the-proper-way-to-securely-disconnect-an-asio-ssl-socket/32054476#32054476

    Behavior of ssl::stream regarding close_notify

    If the remote host calls async_shutdown then the
    local host's async_read will complete with eof.

    If both hosts call async_shutdown then the calls
    to async_shutdown will complete with eof.

*/

template<class AsyncStream>
void
teardown(
    role_type role,
    asio::ssl::stream<AsyncStream>& stream,
    error_code& ec)
{
    stream.shutdown(ec);
    using bho::beast::websocket::teardown;
    error_code ec2;
    teardown(role, stream.next_layer(), ec ? ec2 : ec);
}

namespace detail {

template<class AsyncStream>
struct ssl_shutdown_op
    : asio::coroutine
{
    ssl_shutdown_op(
        asio::ssl::stream<AsyncStream>& s,
        role_type role)
        : s_(s)
        , role_(role)
    {
    }

    template<class Self>
    void
    operator()(Self& self, error_code ec = {}, std::size_t = 0)
    {
        ASIO_CORO_REENTER(*this)
        {
            self.reset_cancellation_state(net::enable_total_cancellation());

            ASIO_CORO_YIELD
                s_.async_shutdown(std::move(self));
            ec_ = ec;

            using bho::beast::websocket::async_teardown;
            ASIO_CORO_YIELD
                async_teardown(role_, s_.next_layer(), std::move(self));
            if (!ec_)
                ec_ = ec;

            self.complete(ec_);
        }
    }

private:
    asio::ssl::stream<AsyncStream>& s_;
    role_type role_;
    error_code ec_;
};

} // detail

template<
    class AsyncStream,
    class TeardownHandler>
void
async_teardown(
    role_type role,
    asio::ssl::stream<AsyncStream>& stream,
    TeardownHandler&& handler)
{
    return asio::async_compose<TeardownHandler, void(error_code)>(
        detail::ssl_shutdown_op<AsyncStream>(stream, role),
        handler,
        stream);
}

} // beast
} // bho

#endif
