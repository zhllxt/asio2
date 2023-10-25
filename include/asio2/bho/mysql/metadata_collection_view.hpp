//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_MYSQL_METADATA_COLLECTION_VIEW_HPP
#define BHO_MYSQL_METADATA_COLLECTION_VIEW_HPP

#include <asio2/bho/mysql/metadata.hpp>

#include <asio2/bho/core/span.hpp>

namespace bho {
namespace mysql {

/**
 * \brief A read-only view of a collection of \ref metadata objects.
 * \par Object lifetimes
 * The object doesn't own the storage for the \ref metadata objects. These are typically
 * owned by a \ref results or \ref execution_state object. This view is valid as long
 * as the memory allocated for the \ref metadata objects remain valid.
 */
using metadata_collection_view = bho::span<const metadata>;

}  // namespace mysql
}  // namespace bho

#endif
