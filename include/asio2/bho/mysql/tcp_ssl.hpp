//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_MYSQL_TCP_SSL_HPP
#define BHO_MYSQL_TCP_SSL_HPP

#include <asio2/bho/mysql/connection.hpp>

#include <asio/ip/tcp.hpp>
#include <asio/ssl/stream.hpp>

namespace bho {
namespace mysql {

/// A connection to MySQL over a TCP socket using TLS.
using tcp_ssl_connection = connection<asio::ssl::stream<asio::ip::tcp::socket>>;

}  // namespace mysql
}  // namespace bho

#endif
