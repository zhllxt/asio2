//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BHO_BEAST_ERROR_HPP
#define BHO_BEAST_ERROR_HPP

#include <asio2/bho/beast/core/detail/config.hpp>
#include <asio/error.hpp>
#include <system_error>

namespace bho {
namespace beast {

/// The type of error code used by the library
using error_code = asio::error_code;

/// The type of system error thrown by the library
using system_error = asio::system_error;

/// The type of error category used by the library
using error_category = asio::error_category;

/// A function to return the generic error category used by the library
#if BHO_BEAST_DOXYGEN
error_category const&
generic_category();
#else
using std::generic_category;
#endif

/// A function to return the system error category used by the library
#if BHO_BEAST_DOXYGEN
error_category const&
system_category();
#else
using asio::system_category;
#endif

/// The type of error condition used by the library
using error_condition = asio::error_condition;

/// The set of constants used for cross-platform error codes
#if BHO_BEAST_DOXYGEN
enum errc{};
#else
using errc = std::errc;
#endif

//------------------------------------------------------------------------------

/// Error codes returned from library operations
enum class error
{
    /** The socket was closed due to a timeout

        This error indicates that a socket was closed due to a
        a timeout detected during an operation.

        Error codes with this value will compare equal to @ref condition::timeout.
    */
    timeout = 1
};

/// Error conditions corresponding to sets of library error codes.
enum class condition
{
    /** The operation timed out

        This error indicates that an operation took took too long.
    */
    timeout = 1
};

} // beast
} // bho

#include <asio2/bho/beast/core/impl/error.hpp>
#ifdef BEAST_HEADER_ONLY
#include <asio2/bho/beast/core/impl/error.ipp>
#endif

#endif
