//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_MYSQL_DETAIL_THROW_ON_ERROR_LOC_HPP
#define BHO_MYSQL_DETAIL_THROW_ON_ERROR_LOC_HPP

#include <asio2/bho/mysql/diagnostics.hpp>
#include <asio2/bho/mysql/error_code.hpp>
#include <asio2/bho/mysql/error_with_diagnostics.hpp>

#include <asio2/bho/assert/source_location.hpp>

namespace bho {
namespace mysql {
namespace detail {

inline void throw_on_error_loc(error_code err, const diagnostics& diag, const bho::source_location& loc)
{
    if (err)
    {
        ::bho::throw_exception(error_with_diagnostics(err, diag), loc);
    }
}

}  // namespace detail
}  // namespace mysql
}  // namespace bho

#endif
