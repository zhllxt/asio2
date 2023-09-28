//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BHO_BEAST_SRC_HPP
#define BHO_BEAST_SRC_HPP

/*

This file is meant to be included once, in a translation unit of
the program, with the macro BHO_BEAST_SEPARATE_COMPILATION defined.

*/

#define BHO_BEAST_SOURCE

#include <asio2/bho/beast/core/detail/config.hpp>

#if defined(BEAST_HEADER_ONLY)
# error Do not compile Beast library source with BEAST_HEADER_ONLY defined
#endif


#include <asio2/bho/beast/core/detail/base64.ipp>
#include <asio2/bho/beast/core/detail/sha1.ipp>
#include <asio2/bho/beast/core/detail/impl/temporary_buffer.ipp>
#include <asio2/bho/beast/core/impl/error.ipp>
#include <asio2/bho/beast/core/impl/file_stdio.ipp>
#include <asio2/bho/beast/core/impl/flat_static_buffer.ipp>
#include <asio2/bho/beast/core/impl/saved_handler.ipp>
#include <asio2/bho/beast/core/impl/static_buffer.ipp>
#include <asio2/bho/beast/core/impl/string.ipp>

#include <asio2/bho/beast/http/detail/basic_parser.ipp>
#include <asio2/bho/beast/http/detail/rfc7230.ipp>
#include <asio2/bho/beast/http/impl/basic_parser.ipp>
#include <asio2/bho/beast/http/impl/error.ipp>
#include <asio2/bho/beast/http/impl/field.ipp>
#include <asio2/bho/beast/http/impl/fields.ipp>
#include <asio2/bho/beast/http/impl/rfc7230.ipp>
#include <asio2/bho/beast/http/impl/status.ipp>
#include <asio2/bho/beast/http/impl/verb.ipp>

#include <asio2/bho/beast/websocket/detail/hybi13.ipp>
#include <asio2/bho/beast/websocket/detail/mask.ipp>
#include <asio2/bho/beast/websocket/detail/pmd_extension.ipp>
#include <asio2/bho/beast/websocket/detail/prng.ipp>
#include <asio2/bho/beast/websocket/detail/service.ipp>
#include <asio2/bho/beast/websocket/detail/utf8_checker.ipp>
#include <asio2/bho/beast/websocket/impl/error.ipp>

#include <asio2/bho/beast/zlib/detail/deflate_stream.ipp>
#include <asio2/bho/beast/zlib/detail/inflate_stream.ipp>
#include <asio2/bho/beast/zlib/impl/error.ipp>

#endif
