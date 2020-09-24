//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BEAST_WEBSOCKET_DETAIL_HYBI13_HPP
#define BEAST_WEBSOCKET_DETAIL_HYBI13_HPP

#include <beast/core/static_string.hpp>
#include <beast/core/string.hpp>
#include <beast/core/detail/base64.hpp>
#include <cstdint>

namespace beast {
namespace websocket {
namespace detail {

using sec_ws_key_type = static_string<
    beast::detail::base64::encoded_size(16)>;

using sec_ws_accept_type = static_string<
    beast::detail::base64::encoded_size(20)>;

BEAST_DECL
void
make_sec_ws_key(sec_ws_key_type& key);

BEAST_DECL
void
make_sec_ws_accept(
    sec_ws_accept_type& accept,
    string_view key);

} // detail
} // websocket
} // beast

#if BEAST_HEADER_ONLY
#include <beast/websocket/detail/hybi13.ipp>
#endif

#endif
