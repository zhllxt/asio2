//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_MYSQL_SRC_HPP
#define BHO_MYSQL_SRC_HPP

// This file is meant to be included once, in a translation unit of
// the program, with the macro BHO_MYSQL_SEPARATE_COMPILATION defined.

#include <asio2/bho/mysql/detail/config.hpp>

#ifndef BHO_MYSQL_SEPARATE_COMPILATION
#error You need to define BHO_MYSQL_SEPARATE_COMPILATION in all translation units that use the compiled version of BHO.MySQL, \
    as well as the one where this file is included.
#endif

#include <asio2/bho/mysql/impl/any_stream_impl.ipp>
#include <asio2/bho/mysql/impl/channel_ptr.ipp>
#include <asio2/bho/mysql/impl/column_type.ipp>
#include <asio2/bho/mysql/impl/date.ipp>
#include <asio2/bho/mysql/impl/datetime.ipp>
#include <asio2/bho/mysql/impl/error_categories.ipp>
#include <asio2/bho/mysql/impl/execution_state_impl.ipp>
#include <asio2/bho/mysql/impl/field.ipp>
#include <asio2/bho/mysql/impl/field_kind.ipp>
#include <asio2/bho/mysql/impl/field_view.ipp>
#include <asio2/bho/mysql/impl/internal/auth/auth.ipp>
#include <asio2/bho/mysql/impl/internal/channel/message_parser.ipp>
#include <asio2/bho/mysql/impl/internal/error/server_error_to_string.ipp>
#include <asio2/bho/mysql/impl/internal/protocol/binary_serialization.ipp>
#include <asio2/bho/mysql/impl/internal/protocol/deserialize_binary_field.ipp>
#include <asio2/bho/mysql/impl/internal/protocol/deserialize_text_field.ipp>
#include <asio2/bho/mysql/impl/internal/protocol/protocol.ipp>
#include <asio2/bho/mysql/impl/internal/protocol/protocol_field_type.ipp>
#include <asio2/bho/mysql/impl/meta_check_context.ipp>
#include <asio2/bho/mysql/impl/network_algorithms.ipp>
#include <asio2/bho/mysql/impl/results_impl.ipp>
#include <asio2/bho/mysql/impl/resultset.ipp>
#include <asio2/bho/mysql/impl/row_impl.ipp>
#include <asio2/bho/mysql/impl/static_execution_state_impl.ipp>
#include <asio2/bho/mysql/impl/static_results_impl.ipp>

#endif
