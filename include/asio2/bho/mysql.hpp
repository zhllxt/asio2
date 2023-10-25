//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_MYSQL_HPP
#define BHO_MYSQL_HPP

#include <asio2/bho/mysql/bad_field_access.hpp>
#include <asio2/bho/mysql/blob.hpp>
#include <asio2/bho/mysql/blob_view.hpp>
#include <asio2/bho/mysql/buffer_params.hpp>
#include <asio2/bho/mysql/client_errc.hpp>
#include <asio2/bho/mysql/column_type.hpp>
#include <asio2/bho/mysql/common_server_errc.hpp>
#include <asio2/bho/mysql/connection.hpp>
#include <asio2/bho/mysql/date.hpp>
#include <asio2/bho/mysql/datetime.hpp>
#include <asio2/bho/mysql/days.hpp>
#include <asio2/bho/mysql/diagnostics.hpp>
#include <asio2/bho/mysql/error_categories.hpp>
#include <asio2/bho/mysql/error_code.hpp>
#include <asio2/bho/mysql/error_with_diagnostics.hpp>
#include <asio2/bho/mysql/execution_state.hpp>
#include <asio2/bho/mysql/field.hpp>
#include <asio2/bho/mysql/field_kind.hpp>
#include <asio2/bho/mysql/field_view.hpp>
#include <asio2/bho/mysql/handshake_params.hpp>
#include <asio2/bho/mysql/mariadb_collations.hpp>
#include <asio2/bho/mysql/mariadb_server_errc.hpp>
#include <asio2/bho/mysql/metadata.hpp>
#include <asio2/bho/mysql/metadata_collection_view.hpp>
#include <asio2/bho/mysql/metadata_mode.hpp>
#include <asio2/bho/mysql/mysql_collations.hpp>
#include <asio2/bho/mysql/mysql_server_errc.hpp>
#include <asio2/bho/mysql/results.hpp>
#include <asio2/bho/mysql/resultset.hpp>
#include <asio2/bho/mysql/resultset_view.hpp>
#include <asio2/bho/mysql/row.hpp>
#include <asio2/bho/mysql/row_view.hpp>
#include <asio2/bho/mysql/rows.hpp>
#include <asio2/bho/mysql/rows_view.hpp>
#include <asio2/bho/mysql/ssl_mode.hpp>
#include <asio2/bho/mysql/statement.hpp>
#include <asio2/bho/mysql/static_execution_state.hpp>
#include <asio2/bho/mysql/static_results.hpp>
#include <asio2/bho/mysql/string_view.hpp>
#include <asio2/bho/mysql/tcp.hpp>
#include <asio2/bho/mysql/tcp_ssl.hpp>
#include <asio2/bho/mysql/throw_on_error.hpp>
#include <asio2/bho/mysql/time.hpp>
#include <asio2/bho/mysql/unix.hpp>
#include <asio2/bho/mysql/unix_ssl.hpp>

#endif
