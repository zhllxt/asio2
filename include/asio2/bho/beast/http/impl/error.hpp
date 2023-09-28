//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BHO_BEAST_HTTP_IMPL_ERROR_HPP
#define BHO_BEAST_HTTP_IMPL_ERROR_HPP

#include <type_traits>

namespace std {
template<>
struct is_error_code_enum<::bho::beast::http::error>
{
    static bool const value = true;
};
} // bho

namespace bho {
namespace beast {
namespace http {

BHO_BEAST_DECL
error_code
make_error_code(error ev);

} // http
} // beast
} // bho

#endif
