//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_MYSQL_CLIENT_ERRC_HPP
#define BHO_MYSQL_CLIENT_ERRC_HPP

#include <asio2/bho/mysql/error_code.hpp>

#include <asio2/bho/mysql/detail/config.hpp>

#include <asio/error.hpp>

namespace bho {
namespace mysql {

/**
 * \brief MySQL client-defined error codes.
 * \details These errors are produced by the client itself, rather than the server.
 */
enum class client_errc : int
{
    /// An incomplete message was received from the server (indicates a deserialization error or
    /// packet mismatch).
    incomplete_message = 1,

    /// An unexpected value was found in a server-received message (indicates a deserialization
    /// error or packet mismatch).
    protocol_value_error,

    /// The server does not support the minimum required capabilities to establish the connection.
    server_unsupported,

    /// Unexpected extra bytes at the end of a message were received (indicates a deserialization
    /// error or packet mismatch).
    extra_bytes,

    /// Mismatched sequence numbers (usually caused by a packet mismatch).
    sequence_number_mismatch,

    /// The user employs an authentication plugin not known to this library.
    unknown_auth_plugin,

    /// The authentication plugin requires the connection to use SSL.
    auth_plugin_requires_ssl,

    /// The number of parameters passed to the prepared statement does not match the number of
    /// actual parameters.
    wrong_num_params,

    /// The connection mandatory SSL, but the server doesn't accept SSL connections.
    server_doesnt_support_ssl,

    /// The static interface detected a mismatch between your C++ type definitions and what the server
    /// returned in the query.
    metadata_check_failed,

    /// The static interface detected a mismatch between the number of row types passed to `static_results`
    /// or `static_execution_state` and the number of resultsets returned by your query.
    num_resultsets_mismatch,

    /// The StaticRow type passed to read_some_rows does not correspond to the resultset type being read.
    row_type_mismatch,

    /// The static interface encountered an error when parsing a field into a C++ data structure.
    static_row_parsing_error,
};

BHO_MYSQL_DECL
const asio::error_category& get_client_category() noexcept;

/// Creates an \ref error_code from a \ref client_errc.
inline error_code make_error_code(client_errc error)
{
    return error_code(static_cast<int>(error), get_client_category());
}

}  // namespace mysql

}  // namespace bho

#ifndef BHO_MYSQL_DOXYGEN
namespace std {

template <>
struct is_error_code_enum<::bho::mysql::client_errc>
{
    static constexpr bool value = true;
};
}  // namespace std
#endif


#ifdef BHO_MYSQL_HEADER_ONLY
#include <asio2/bho/mysql/impl/error_categories.ipp>
#endif

#endif
