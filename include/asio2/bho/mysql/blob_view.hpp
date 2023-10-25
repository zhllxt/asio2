//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_MYSQL_BLOB_VIEW_HPP
#define BHO_MYSQL_BLOB_VIEW_HPP

#include <asio2/bho/core/span.hpp>

namespace bho {
namespace mysql {

/// Non-owning type used to represent binary blobs.
using blob_view = bho::span<const unsigned char>;

}  // namespace mysql
}  // namespace bho

#endif
