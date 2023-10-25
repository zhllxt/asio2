//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_MYSQL_UNIX_HPP
#define BHO_MYSQL_UNIX_HPP

#include <asio2/bho/mysql/connection.hpp>

#include <asio/local/stream_protocol.hpp>

namespace bho {
namespace mysql {

#if defined(BHO_ASIO_HAS_LOCAL_SOCKETS) || defined(BHO_MYSQL_DOXYGEN)

/// A connection to MySQL over a UNIX domain socket.
using unix_connection = connection<asio::local::stream_protocol::socket>;

#endif

}  // namespace mysql
}  // namespace bho

#endif
