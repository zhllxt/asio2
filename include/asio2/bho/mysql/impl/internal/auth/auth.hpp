//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_MYSQL_IMPL_INTERNAL_AUTH_AUTH_HPP
#define BHO_MYSQL_IMPL_INTERNAL_AUTH_AUTH_HPP

#include <asio2/bho/mysql/error_code.hpp>
#include <asio2/bho/mysql/string_view.hpp>

#include <asio2/bho/mysql/detail/config.hpp>

#include <asio2/bho/core/span.hpp>

#include <vector>

namespace bho {
namespace mysql {
namespace detail {

struct auth_response
{
    std::vector<std::uint8_t> data;
    string_view plugin_name;
};

BHO_ATTRIBUTE_NODISCARD
BHO_MYSQL_DECL
error_code compute_auth_response(
    string_view plugin_name,
    string_view password,
    span<const std::uint8_t> challenge,
    bool use_ssl,
    auth_response& output
);

}  // namespace detail
}  // namespace mysql
}  // namespace bho

#ifdef BHO_MYSQL_HEADER_ONLY
#include <asio2/bho/mysql/impl/internal/auth/auth.ipp>
#endif

#endif
