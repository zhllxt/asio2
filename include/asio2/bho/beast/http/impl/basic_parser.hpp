//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BHO_BEAST_HTTP_IMPL_BASIC_PARSER_HPP
#define BHO_BEAST_HTTP_IMPL_BASIC_PARSER_HPP

#include <asio2/bho/beast/core/buffer_traits.hpp>
#include <asio/buffer.hpp>

namespace bho {
namespace beast {
namespace http {

template<bool isRequest>
template<class ConstBufferSequence>
std::size_t
basic_parser<isRequest>::
put(ConstBufferSequence const& buffers,
    error_code& ec)
{
    static_assert(net::is_const_buffer_sequence<
        ConstBufferSequence>::value,
            "ConstBufferSequence type requirements not met");
    auto const p = net::buffer_sequence_begin(buffers);
    auto const last = net::buffer_sequence_end(buffers);
    if(p == last)
    {
        ec = {};
        return 0;
    }
    if(std::next(p) == last)
    {
        // single buffer
        return put(net::const_buffer(*p), ec);
    }
    auto const size = buffer_bytes(buffers);
    if(size <= max_stack_buffer)
        return put_from_stack(size, buffers, ec);
    if(size > buf_len_)
    {
        // reallocate
        buf_ = std::make_unique<char[]>(size);
        buf_len_ = size;
    }
    // flatten
    net::buffer_copy(net::buffer(
        buf_.get(), size), buffers);
    return put(net::const_buffer{
        buf_.get(), size}, ec);
}

template<bool isRequest>
std::optional<std::uint64_t>
basic_parser<isRequest>::
content_length_unchecked() const
{
    if(f_ & flagContentLength)
        return len0_;
    return std::nullopt;
}

template<bool isRequest>
template<class ConstBufferSequence>
std::size_t
basic_parser<isRequest>::
put_from_stack(std::size_t size,
    ConstBufferSequence const& buffers,
        error_code& ec)
{
    char buf[max_stack_buffer];
    net::buffer_copy(net::mutable_buffer(
        buf, sizeof(buf)), buffers);
    return put(net::const_buffer{
        buf, size}, ec);
}

} // http
} // beast
} // bho

#endif
