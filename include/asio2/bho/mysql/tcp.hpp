//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_MYSQL_TCP_HPP
#define BHO_MYSQL_TCP_HPP

#include <asio2/bho/mysql/connection.hpp>

#include <asio/ip/tcp.hpp>

namespace bho {
namespace mysql {

/// A connection to MySQL over a TCP socket.
using tcp_connection = connection<asio::ip::tcp::socket>;

}  // namespace mysql
}  // namespace bho

#endif
