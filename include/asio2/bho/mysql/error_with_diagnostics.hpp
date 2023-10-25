//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_MYSQL_ERROR_WITH_DIAGNOSTICS_HPP
#define BHO_MYSQL_ERROR_WITH_DIAGNOSTICS_HPP

#include <asio2/bho/mysql/diagnostics.hpp>
#include <asio2/bho/mysql/error_code.hpp>

#include <system_error>

namespace bho {
namespace mysql {

/**
 * \brief A system_error with an embedded diagnostics object.
 * \details
 * Like `asio::system_error`, but adds a \ref diagnostics member
 * containing additional information.
 */
class error_with_diagnostics : public asio::system_error
{
    diagnostics diag_;

    static asio::system_error create_base(const error_code& err, const diagnostics& diag)
    {
        return diag.client_message().empty() ? asio::system_error(err)
                                             : asio::system_error(err, diag.client_message());
    }

public:
    /// Initializing constructor.
    error_with_diagnostics(const error_code& err, const diagnostics& diag)
        : asio::system_error(create_base(err, diag)), diag_(diag)
    {
    }

    /**
     * \brief Retrieves the server diagnostics embedded in this object.
     * \par Exception safety
     * No-throw guarantee.
     *
     * \par Object lifetimes
     * The returned reference is valid as long as `*this` is alive.
     */
    const diagnostics& get_diagnostics() const noexcept { return diag_; }
};

}  // namespace mysql
}  // namespace bho

#endif
