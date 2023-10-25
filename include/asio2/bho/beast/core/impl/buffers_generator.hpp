//
// Copyright (c) 2022 Seth Heeren (sgheeren at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BHO_BEAST_CORE_IMPL_BUFFERS_GENERATOR_HPP
#define BHO_BEAST_CORE_IMPL_BUFFERS_GENERATOR_HPP

#include <asio2/bho/beast/core/buffers_generator.hpp>

#include <asio/write.hpp>
#include <asio/async_result.hpp>
#include <asio/compose.hpp>
#include <asio/coroutine.hpp>

#include <asio2/bho/beast/core/buffer_traits.hpp>
#include <asio2/bho/beast/core/stream_traits.hpp>

#include <asio2/bho/throw_exception.hpp>
#include <type_traits>

namespace bho {
namespace beast {

namespace detail {

template <
    class AsyncWriteStream,
    class BuffersGenerator>
struct write_buffers_generator_op
    : asio::coroutine
{
    write_buffers_generator_op(
        AsyncWriteStream& s, BuffersGenerator g)
        : s_(s)
        , g_(std::move(g))
    {
    }

    template<class Self>
    void operator()(
        Self& self, error_code ec = {}, std::size_t n = 0)
    {
        ASIO_CORO_REENTER(*this)
        {
            while(! g_.is_done())
            {
                ASIO_CORO_YIELD
                {
                    auto cb = g_.prepare(ec);
                    if(ec)
                        goto complete;
                    s_.async_write_some(
                        cb, std::move(self));
                }
                if(ec)
                    goto complete;

                g_.consume(n);

                total_ += n;
            }

        complete:
            self.complete(ec, total_);
        }
    }

private:
    AsyncWriteStream& s_;
    BuffersGenerator g_;
    std::size_t total_ = 0;
};

} // detail

template<
    class SyncWriteStream,
    class BuffersGenerator,
    typename std::enable_if< //
        is_buffers_generator<typename std::decay<
            BuffersGenerator>::type>::value>::type* /*= nullptr*/
    >
size_t
write(
    SyncWriteStream& stream,
    BuffersGenerator&& generator,
    beast::error_code& ec)
{
    static_assert(
        is_sync_write_stream<SyncWriteStream>::value,
        "SyncWriteStream type requirements not met");

    ec.clear();
    size_t total = 0;
    while(! generator.is_done())
    {
        auto cb = generator.prepare(ec);
        if(ec)
            break;

        size_t n = net::write(stream, cb, ec);

        if(ec)
            break;

        generator.consume(n);
        total += n;
    }

    return total;
}

//----------------------------------------------------------

template<
    class SyncWriteStream,
    class BuffersGenerator,
    typename std::enable_if<is_buffers_generator<
        typename std::decay<BuffersGenerator>::type>::value>::
        type* /*= nullptr*/
    >
std::size_t
write(SyncWriteStream& stream, BuffersGenerator&& generator)
{
    static_assert(
        is_sync_write_stream<SyncWriteStream>::value,
        "SyncWriteStream type requirements not met");
    beast::error_code ec;
    std::size_t n = write(
        stream, std::forward<BuffersGenerator>(generator), ec);
    if(ec)
        BHO_THROW_EXCEPTION(system_error{ ec });
    return n;
}

//----------------------------------------------------------

template<
    class AsyncWriteStream,
    class BuffersGenerator,
    BHO_BEAST_ASYNC_TPARAM2 CompletionToken,
    typename std::enable_if<is_buffers_generator<
        BuffersGenerator>::value>::type* /*= nullptr*/
    >
BHO_BEAST_ASYNC_RESULT2(CompletionToken)
async_write(
    AsyncWriteStream& stream,
    BuffersGenerator generator,
    CompletionToken&& token)
{
    static_assert(
        beast::is_async_write_stream<AsyncWriteStream>::value,
        "AsyncWriteStream type requirements not met");

    return net::async_compose< //
        CompletionToken,
        void(error_code, std::size_t)>(
        detail::write_buffers_generator_op<
            AsyncWriteStream,
            BuffersGenerator>{ stream, std::move(generator) },
        token,
        stream);
}

} // namespace beast
} // namespace bho

#endif
