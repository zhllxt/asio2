//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_MYSQL_STRING_VIEW_HPP
#define BHO_MYSQL_STRING_VIEW_HPP

#include <asio2/bho/core/detail/string_view.hpp>

namespace bho {
namespace mysql {

/// Type used to represent read-only string references, similar to `std::string_view`.
using string_view = bho::core::string_view;

}  // namespace mysql
}  // namespace bho

#endif
