//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_MYSQL_IMPL_INTERNAL_NETWORK_ALGORITHMS_CLOSE_STATEMENT_HPP
#define BHO_MYSQL_IMPL_INTERNAL_NETWORK_ALGORITHMS_CLOSE_STATEMENT_HPP

#include <asio2/bho/mysql/diagnostics.hpp>
#include <asio2/bho/mysql/error_code.hpp>
#include <asio2/bho/mysql/statement.hpp>

#include <asio2/bho/mysql/detail/config.hpp>

#include <asio2/bho/mysql/impl/internal/channel/channel.hpp>
#include <asio2/bho/mysql/impl/internal/protocol/protocol.hpp>

#include <asio/async_result.hpp>

namespace bho {
namespace mysql {
namespace detail {

inline void compose_close_statement(channel& chan, const statement& stmt)
{
    chan.serialize(close_stmt_command{stmt.id()}, chan.reset_sequence_number());
}

inline void close_statement_impl(channel& chan, const statement& stmt, error_code& err, diagnostics& diag)
{
    err.clear();
    diag.clear();

    // Serialize the close message
    compose_close_statement(chan, stmt);

    // Send it. No response is sent back
    chan.write(err);
}

template <class CompletionToken>
ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(error_code))
async_close_statement_impl(channel& chan, const statement& stmt, diagnostics& diag, CompletionToken&& token)
{
    // We can do this here because we know no deferred tokens reach this function (thanks to erasing)
    diag.clear();

    // Serialize the close message
    compose_close_statement(chan, stmt);

    // Send it. No response is sent back
    return chan.async_write(std::forward<CompletionToken>(token));
}

}  // namespace detail
}  // namespace mysql
}  // namespace bho

#endif /* INCLUDE_BHO_MYSQL_DETAIL_NETWORK_ALGORITHMS_CLOSE_STATEMENT_HPP_ */
