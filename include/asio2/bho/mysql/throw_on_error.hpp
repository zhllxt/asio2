//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_MYSQL_THROW_ON_ERROR_HPP
#define BHO_MYSQL_THROW_ON_ERROR_HPP

#include <asio2/bho/mysql/detail/throw_on_error_loc.hpp>

namespace bho {
namespace mysql {

/**
 * \brief Throws an exception in case of error, including diagnostic information.
 * \details If err indicates a failure (`err.failed() == true`), throws an exception that
 * derives from \ref error_with_diagnostics. The exception will make
 * `diag` available in \ref error_with_diagnostics::get_diagnostics.
 */
inline void throw_on_error(error_code err, const diagnostics& diag = {})
{
    detail::throw_on_error_loc(err, diag, bho::source_location{});
}

}  // namespace mysql
}  // namespace bho

#endif
