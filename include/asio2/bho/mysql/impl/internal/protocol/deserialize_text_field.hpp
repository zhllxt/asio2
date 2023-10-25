//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_MYSQL_IMPL_INTERNAL_PROTOCOL_DESERIALIZE_TEXT_FIELD_HPP
#define BHO_MYSQL_IMPL_INTERNAL_PROTOCOL_DESERIALIZE_TEXT_FIELD_HPP

#include <asio2/bho/mysql/field_view.hpp>
#include <asio2/bho/mysql/metadata.hpp>

#include <asio2/bho/mysql/detail/config.hpp>

#include <asio2/bho/mysql/impl/internal/protocol/serialization.hpp>

namespace bho {
namespace mysql {
namespace detail {

BHO_MYSQL_DECL
deserialize_errc deserialize_text_field(string_view from, const metadata& meta, field_view& output);

}  // namespace detail
}  // namespace mysql
}  // namespace bho

#ifdef BHO_MYSQL_HEADER_ONLY
#include <asio2/bho/mysql/impl/internal/protocol/deserialize_text_field.ipp>
#endif

#endif
