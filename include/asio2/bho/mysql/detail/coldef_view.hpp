//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_MYSQL_DETAIL_COLDEF_VIEW_HPP
#define BHO_MYSQL_DETAIL_COLDEF_VIEW_HPP

#include <asio2/bho/mysql/column_type.hpp>
#include <asio2/bho/mysql/string_view.hpp>

namespace bho {
namespace mysql {
namespace detail {

struct coldef_view
{
    string_view database;
    string_view table;
    string_view org_table;
    string_view name;
    string_view org_name;
    std::uint16_t collation_id;
    std::uint32_t column_length;  // maximum length of the field
    column_type type;
    std::uint16_t flags;
    std::uint8_t decimals;  // max shown decimal digits. 0x00 for int/static strings; 0x1f for
                            // dynamic strings, double, float
};

}  // namespace detail
}  // namespace mysql
}  // namespace bho

#endif
