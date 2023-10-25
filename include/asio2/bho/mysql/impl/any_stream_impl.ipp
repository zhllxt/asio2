//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_MYSQL_IMPL_ANY_STREAM_IMPL_IPP
#define BHO_MYSQL_IMPL_ANY_STREAM_IMPL_IPP

#pragma once

#include <asio2/bho/mysql/detail/any_stream_impl.hpp>

#ifdef BHO_MYSQL_SEPARATE_COMPILATION
template class bho::mysql::detail::any_stream_impl<asio::ssl::stream<asio::ip::tcp::socket>>;
template class bho::mysql::detail::any_stream_impl<asio::ip::tcp::socket>;
#endif

#endif
