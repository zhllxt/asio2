//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BHO_BEAST_CORE_DETAIL_BUFFER_HPP
#define BHO_BEAST_CORE_DETAIL_BUFFER_HPP

#include <asio2/bho/beast/core/error.hpp>
#include <asio/buffer.hpp>
#include <optional>
#include <stdexcept>

namespace bho {
namespace beast {
namespace detail {

template<
    class DynamicBuffer,
    class ErrorValue>
auto
dynamic_buffer_prepare_noexcept(
    DynamicBuffer& buffer,
    std::size_t size,
    error_code& ec,
    ErrorValue ev) ->
        std::optional<typename
        DynamicBuffer::mutable_buffers_type>
{
    if(buffer.max_size() - buffer.size() < size)
    {
        // length error
        BHO_BEAST_ASSIGN_EC(ec, ev);
        return std::nullopt;
    }
    std::optional<typename
        DynamicBuffer::mutable_buffers_type> result;
    result.emplace(buffer.prepare(size));
    ec = {};
    return result;
}

template<
    class DynamicBuffer,
    class ErrorValue>
auto
dynamic_buffer_prepare(
    DynamicBuffer& buffer,
    std::size_t size,
    error_code& ec,
    ErrorValue ev) ->
        std::optional<typename
        DynamicBuffer::mutable_buffers_type>
{
#ifndef BHO_NO_EXCEPTIONS
    try
    {
        std::optional<typename
            DynamicBuffer::mutable_buffers_type> result;
        result.emplace(buffer.prepare(size));
        ec = {};
        return result;
    }
    catch(std::length_error const&)
    {
        BHO_BEAST_ASSIGN_EC(ec, ev);
    }
    return std::nullopt;

#else
    return dynamic_buffer_prepare_noexcept(
        buffer, size, ec, ev);
#endif
}

} // detail
} // beast
} // bho

#endif
