//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BHO_BEAST_WEBSOCKET_DETAIL_PRNG_HPP
#define BHO_BEAST_WEBSOCKET_DETAIL_PRNG_HPP

#include <asio2/bho/beast/core/detail/config.hpp>
#include <asio2/bho/config.hpp>
#include <cstdint>
#include <random>

namespace bho {
namespace beast {
namespace websocket {
namespace detail {

using generator = std::uint32_t(*)();

//------------------------------------------------------------------------------

// Manually seed the prngs, must be called
// before acquiring a prng for the first time.
//
BHO_BEAST_DECL
std::uint32_t const*
prng_seed(std::seed_seq* ss = nullptr);

// Acquire a PRNG using the TLS implementation if it
// is available, otherwise using the no-TLS implementation.
//
BHO_BEAST_DECL
generator
make_prng(bool secure);

} // detail
} // websocket
} // beast
} // bho

#ifdef BEAST_HEADER_ONLY
#include <asio2/bho/beast/websocket/detail/prng.ipp>
#endif

#endif
