//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_MYSQL_IMPL_INTERNAL_NETWORK_ALGORITHMS_PING_HPP
#define BHO_MYSQL_IMPL_INTERNAL_NETWORK_ALGORITHMS_PING_HPP

#include <asio2/bho/mysql/client_errc.hpp>
#include <asio2/bho/mysql/diagnostics.hpp>
#include <asio2/bho/mysql/error_code.hpp>

#include <asio2/bho/mysql/detail/config.hpp>

#include <asio2/bho/mysql/impl/internal/channel/channel.hpp>
#include <asio2/bho/mysql/impl/internal/protocol/protocol.hpp>

#include <asio/async_result.hpp>
#include <asio/coroutine.hpp>

namespace bho {
namespace mysql {
namespace detail {

inline void serialize_ping_message(channel& chan)
{
    chan.serialize(ping_command(), chan.reset_sequence_number());
}

struct ping_op : asio::coroutine
{
    channel& chan_;
    diagnostics& diag_;

    ping_op(channel& chan, diagnostics& diag) noexcept : chan_(chan), diag_(diag) {}

    template <class Self>
    void operator()(Self& self, error_code err = {}, span<const std::uint8_t> buff = {})
    {
        // Error checking
        if (err)
        {
            self.complete(err);
            return;
        }

        // Regular coroutine body; if there has been an error, we don't get here
        ASIO_CORO_REENTER(*this)
        {
            diag_.clear();

            // Serialize the message
            serialize_ping_message(chan_);

            // Write message
            ASIO_CORO_YIELD chan_.async_write(std::move(self));

            // Read response
            ASIO_CORO_YIELD chan_.async_read_one(chan_.shared_sequence_number(), std::move(self));

            // Verify it's what we expected
            self.complete(deserialize_ok_response(buff, chan_.flavor(), diag_));
        }
    }
};

// Interface
inline void ping_impl(channel& chan, error_code& err, diagnostics& diag)
{
    err.clear();
    diag.clear();

    // Serialize the message
    serialize_ping_message(chan);

    // Send it
    chan.write(err);
    if (err)
        return;

    // Read response
    auto response = chan.read_one(chan.shared_sequence_number(), err);
    if (err)
        return;

    // Verify it's what we expected
    err = deserialize_ok_response(response, chan.flavor(), diag);
}

template <class CompletionToken>
ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(error_code))
async_ping_impl(channel& chan, diagnostics& diag, CompletionToken&& token)
{
    return asio::async_compose<CompletionToken, void(error_code)>(ping_op(chan, diag), token, chan);
}

}  // namespace detail
}  // namespace mysql
}  // namespace bho

#endif
