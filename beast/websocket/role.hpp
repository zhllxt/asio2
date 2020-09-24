//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BEAST_WEBSOCKET_ROLE_HPP
#define BEAST_WEBSOCKET_ROLE_HPP

#include <beast/core/detail/config.hpp>

#ifndef BEAST_ALLOW_DEPRECATED

#error This file is deprecated interface, #define BEAST_ALLOW_DEPRECATED to allow it

#else

#include <beast/core/role.hpp>

namespace beast {
namespace websocket {

using role_type = beast::role_type;

} // websocket
} // beast

#endif

#endif
