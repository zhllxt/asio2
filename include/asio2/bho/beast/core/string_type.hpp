//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BHO_BEAST_STRING_TYPE_HPP
#define BHO_BEAST_STRING_TYPE_HPP

#include <asio2/bho/beast/core/detail/config.hpp>

#include <string_view>

namespace bho {
namespace beast {

using string_view = std::string_view;

template<class CharT, class Traits>
using basic_string_view =
    std::basic_string_view<CharT, Traits>;

} // beast
} // bho

#endif
