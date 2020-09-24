//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BEAST_DETAIL_ALLOCATOR_HPP
#define BEAST_DETAIL_ALLOCATOR_HPP

#include <memory>

namespace beast {
namespace detail {

template<class Alloc>
using allocator_traits = std::allocator_traits<Alloc>;

} // detail
} // beast

#endif
