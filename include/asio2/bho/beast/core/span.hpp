//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BHO_BEAST_CORE_SPAN_HPP
#define BHO_BEAST_CORE_SPAN_HPP

#include <asio2/bho/beast/core/detail/config.hpp>
#include <asio2/bho/core/span.hpp>

namespace bho {
namespace beast {

template<class T, std::size_t E = bho::dynamic_extent>
using span = bho::span<T, E>;

} // beast
} // bho

#endif
