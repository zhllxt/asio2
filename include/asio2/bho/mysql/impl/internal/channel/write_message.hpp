//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_MYSQL_IMPL_INTERNAL_CHANNEL_WRITE_MESSAGE_HPP
#define BHO_MYSQL_IMPL_INTERNAL_CHANNEL_WRITE_MESSAGE_HPP

#include <asio2/bho/mysql/error_code.hpp>

#include <asio2/bho/mysql/detail/any_stream.hpp>

#include <asio2/bho/mysql/impl/internal/channel/message_writer.hpp>
#include <asio2/bho/mysql/impl/internal/protocol/constants.hpp>

#include <asio/async_result.hpp>
#include <asio/buffer.hpp>
#include <asio/compose.hpp>
#include <asio/coroutine.hpp>

#include <cstddef>
#include <cstdint>

namespace bho {
namespace mysql {
namespace detail {

// Writes an entire message to stream; partitions the message into
// chunks and adds the required headers
inline void write_message(any_stream& stream, message_writer& processor, error_code& ec)
{
    while (!processor.done())
    {
        std::size_t bytes_written = stream.write_some(asio::buffer(processor.next_chunk()), ec);
        if (ec)
            break;
        processor.on_bytes_written(bytes_written);
    }
}

struct write_message_op : asio::coroutine
{
    any_stream& stream_;
    message_writer& processor_;

    write_message_op(any_stream& stream, message_writer& processor) noexcept
        : stream_(stream), processor_(processor)
    {
    }

    template <class Self>
    void operator()(Self& self, error_code ec = {}, std::size_t bytes_written = 0)
    {
        // Error handling
        if (ec)
        {
            self.complete(ec);
            return;
        }

        // Non-error path
        ASIO_CORO_REENTER(*this)
        {
            // done() never returns false after a call to prepare_buffer(), so no post() needed
            BHO_ASSERT(!processor_.done());
            while (!processor_.done())
            {
                ASIO_CORO_YIELD stream_.async_write_some(
                    asio::buffer(processor_.next_chunk()),
                    std::move(self)
                );
                processor_.on_bytes_written(bytes_written);
            };

            self.complete(error_code());
        }
    }
};

template <ASIO_COMPLETION_TOKEN_FOR(void(::bho::mysql::error_code)) CompletionToken>
ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(bho::mysql::error_code))
async_write_message(any_stream& stream, message_writer& processor, CompletionToken&& token)
{
    return asio::async_compose<CompletionToken, void(error_code)>(
        write_message_op(stream, processor),
        token,
        stream
    );
}

}  // namespace detail
}  // namespace mysql
}  // namespace bho

#endif
