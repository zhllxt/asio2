//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BHO_BEAST_DETAIL_ALLOCATOR_HPP
#define BHO_BEAST_DETAIL_ALLOCATOR_HPP

#include <asio2/bho/config.hpp>
#ifdef BHO_NO_CXX11_ALLOCATOR
#include <memory>
#else
#include <memory>
#endif

namespace bho {
namespace beast {
namespace detail {

// This is a workaround for allocator_traits
// implementations which falsely claim C++11
// compatibility.

#ifdef BHO_NO_CXX11_ALLOCATOR
template<class Alloc>
using allocator_traits = std::allocator_traits<Alloc>;

#else
template<class Alloc>
using allocator_traits = std::allocator_traits<Alloc>;

#endif

} // detail
} // beast
} // bho

#endif
