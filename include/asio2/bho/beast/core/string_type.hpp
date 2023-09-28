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

#ifndef BHO_BEAST_USE_STD_STRING_VIEW
#define BHO_BEAST_USE_STD_STRING_VIEW
#endif

#if defined(BHO_BEAST_USE_STD_STRING_VIEW)
#include <string_view>
#else
#include <string_view>
#endif

namespace bho {
namespace beast {

#if BHO_BEAST_DOXYGEN || ! defined(BHO_BEAST_USE_STD_STRING_VIEW)
/// The type of string view used by the library
using string_view = std::string_view;

/// The type of `basic_string_view` used by the library
template<class CharT, class Traits>
using basic_string_view =
    std::basic_string_view<CharT, Traits>;

#else
using string_view = std::string_view;

template<class CharT, class Traits>
using basic_string_view =
    std::basic_string_view<CharT, Traits>;

#endif

} // beast
} // bho

#endif
