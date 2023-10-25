//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_MYSQL_ERROR_CODE_HPP
#define BHO_MYSQL_ERROR_CODE_HPP

#include <asio/error.hpp>

namespace bho {
namespace mysql {

/// An alias for bho::system error codes.
using error_code = asio::error_code;

}  // namespace mysql
}  // namespace bho

#endif
