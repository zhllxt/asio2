//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BHO_BEAST_WEBSOCKET_DETAIL_MASK_HPP
#define BHO_BEAST_WEBSOCKET_DETAIL_MASK_HPP

#include <asio2/bho/beast/core/detail/config.hpp>
#include <asio2/bho/beast/core/buffers_range.hpp>
#include <asio/buffer.hpp>
#include <array>
#include <climits>
#include <cstdint>
#include <random>
#include <type_traits>

namespace bho {
namespace beast {
namespace websocket {
namespace detail {

using prepared_key = std::array<unsigned char, 4>;

BHO_BEAST_DECL
void
prepare_key(prepared_key& prepared, std::uint32_t key);

// Apply mask in place
//
BHO_BEAST_DECL
void
mask_inplace(net::mutable_buffer const& b, prepared_key& key);

// Apply mask in place
//
template<class MutableBufferSequence>
void
mask_inplace(
    MutableBufferSequence const& buffers,
    prepared_key& key)
{
    for(net::mutable_buffer b :
            beast::buffers_range_ref(buffers))
        detail::mask_inplace(b, key);
}

} // detail
} // websocket
} // beast
} // bho


#if BEAST_HEADER_ONLY
#include <asio2/bho/beast/websocket/detail/mask.ipp>
#endif


#endif
