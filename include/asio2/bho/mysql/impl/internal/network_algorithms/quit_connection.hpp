//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_MYSQL_IMPL_INTERNAL_NETWORK_ALGORITHMS_QUIT_CONNECTION_HPP
#define BHO_MYSQL_IMPL_INTERNAL_NETWORK_ALGORITHMS_QUIT_CONNECTION_HPP

#include <asio2/bho/mysql/diagnostics.hpp>
#include <asio2/bho/mysql/error_code.hpp>

#include <asio2/bho/mysql/detail/config.hpp>

#include <asio2/bho/mysql/impl/internal/channel/channel.hpp>
#include <asio2/bho/mysql/impl/internal/protocol/protocol.hpp>

#include <asio/coroutine.hpp>

namespace bho {
namespace mysql {
namespace detail {

inline void compose_quit(channel& chan) { chan.serialize(quit_command(), chan.reset_sequence_number()); }

struct quit_connection_op : asio::coroutine
{
    channel& chan_;
    diagnostics& diag_;

    quit_connection_op(channel& chan, diagnostics& diag) noexcept : chan_(chan), diag_(diag) {}

    template <class Self>
    void operator()(Self& self, error_code err = {})
    {
        ASIO_CORO_REENTER(*this)
        {
            diag_.clear();

            // Quit message
            compose_quit(chan_);
            ASIO_CORO_YIELD chan_.async_write(std::move(self));
            if (err)
            {
                self.complete(err);
            }

            // SSL shutdown error ignored, as MySQL doesn't always gracefully
            // close SSL connections.
            if (chan_.stream().ssl_active())
            {
                ASIO_CORO_YIELD chan_.stream().async_shutdown(std::move(self));
            }

            self.complete(error_code());
        }
    }
};

// Interface
inline void quit_connection_impl(channel& chan, error_code& err, diagnostics& diag)
{
    err.clear();
    diag.clear();

    compose_quit(chan);
    chan.write(err);
    if (err)
        return;
    if (chan.stream().ssl_active())
    {
        // SSL shutdown. Result ignored as MySQL does not always perform
        // graceful SSL shutdowns
        error_code ignored;
        chan.stream().shutdown(ignored);
    }
}

template <class CompletionToken>
ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(error_code))
async_quit_connection_impl(channel& chan, diagnostics& diag, CompletionToken&& token)
{
    return asio::async_compose<CompletionToken, void(error_code)>(
        quit_connection_op(chan, diag),
        token,
        chan
    );
}

}  // namespace detail
}  // namespace mysql
}  // namespace bho

#endif /* INCLUDE_BHO_MYSQL_DETAIL_NETWORK_ALGORITHMS_QUIT_CONNECTION_HPP_ */
