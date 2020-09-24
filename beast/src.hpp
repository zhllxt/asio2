//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BEAST_SRC_HPP
#define BEAST_SRC_HPP

/*

This file is meant to be included once, in a translation unit of
the program, with the macro BEAST_SEPARATE_COMPILATION defined.

*/

#define BEAST_SOURCE

#include <beast/core/detail/config.hpp>

#if defined(BEAST_HEADER_ONLY)
# error Do not compile Beast library source with BEAST_HEADER_ONLY defined
#endif

#include <beast/core/detail/base64.ipp>
#include <beast/core/detail/sha1.ipp>
#include <beast/core/detail/impl/temporary_buffer.ipp>
#include <beast/core/impl/error.ipp>
#include <beast/core/impl/file_stdio.ipp>
#include <beast/core/impl/flat_static_buffer.ipp>
#include <beast/core/impl/saved_handler.ipp>
#include <beast/core/impl/static_buffer.ipp>
#include <beast/core/impl/string.ipp>

#include <beast/http/detail/basic_parser.ipp>
#include <beast/http/detail/rfc7230.ipp>
#include <beast/http/impl/basic_parser.ipp>
#include <beast/http/impl/error.ipp>
#include <beast/http/impl/field.ipp>
#include <beast/http/impl/fields.ipp>
#include <beast/http/impl/rfc7230.ipp>
#include <beast/http/impl/status.ipp>
#include <beast/http/impl/verb.ipp>

#include <beast/websocket/detail/hybi13.ipp>
#include <beast/websocket/detail/mask.ipp>
#include <beast/websocket/detail/pmd_extension.ipp>
#include <beast/websocket/detail/prng.ipp>
#include <beast/websocket/detail/service.ipp>
#include <beast/websocket/detail/utf8_checker.ipp>
#include <beast/websocket/impl/error.ipp>

#include <beast/zlib/detail/deflate_stream.ipp>
#include <beast/zlib/detail/inflate_stream.ipp>
#include <beast/zlib/impl/error.ipp>

#endif
