//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_MYSQL_IMPL_INTERNAL_NETWORK_ALGORITHMS_READ_SOME_ROWS_DYNAMIC_HPP
#define BHO_MYSQL_IMPL_INTERNAL_NETWORK_ALGORITHMS_READ_SOME_ROWS_DYNAMIC_HPP

#include <asio2/bho/mysql/diagnostics.hpp>
#include <asio2/bho/mysql/error_code.hpp>
#include <asio2/bho/mysql/rows_view.hpp>

#include <asio2/bho/mysql/detail/config.hpp>
#include <asio2/bho/mysql/detail/execution_processor/execution_state_impl.hpp>

#include <asio2/bho/mysql/impl/internal/channel/channel.hpp>
#include <asio2/bho/mysql/impl/internal/network_algorithms/read_some_rows.hpp>

#include <asio/async_result.hpp>
#include <asio/coroutine.hpp>

namespace bho {
namespace mysql {
namespace detail {

inline rows_view get_some_rows(const channel& ch, const execution_state_impl& st)
{
    return access::construct<rows_view>(
        ch.shared_fields().data(),
        ch.shared_fields().size(),
        st.meta().size()
    );
}

struct read_some_rows_dynamic_op : asio::coroutine
{
    channel& chan_;
    diagnostics& diag_;
    execution_state_impl& st_;

    read_some_rows_dynamic_op(channel& chan, diagnostics& diag, execution_state_impl& st) noexcept
        : chan_(chan), diag_(diag), st_(st)
    {
    }

    template <class Self>
    void operator()(Self& self, error_code err = {}, std::size_t = 0)
    {
        // Error checking
        if (err)
        {
            self.complete(err, rows_view());
            return;
        }

        // Normal path
        ASIO_CORO_REENTER(*this)
        {
            chan_.shared_fields().clear();
            ASIO_CORO_YIELD async_read_some_rows_impl(chan_, st_, output_ref(), diag_, std::move(self));
            self.complete(error_code(), get_some_rows(chan_, st_));
        }
    }
};

// External interface
inline rows_view read_some_rows_dynamic_impl(
    channel& channel,
    execution_state_impl& st,
    error_code& err,
    diagnostics& diag
)
{
    err.clear();
    diag.clear();
    channel.shared_fields().clear();
    read_some_rows_impl(channel, st, output_ref(), err, diag);
    if (err)
        return rows_view();
    return get_some_rows(channel, st);
}

template <class CompletionToken>
ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(error_code, rows_view))
async_read_some_rows_dynamic_impl(
    channel& channel,
    execution_state_impl& st,
    diagnostics& diag,
    CompletionToken&& token
)
{
    return asio::async_compose<CompletionToken, void(error_code, rows_view)>(
        read_some_rows_dynamic_op(channel, diag, st),
        token,
        channel
    );
}

}  // namespace detail
}  // namespace mysql
}  // namespace bho

#endif
