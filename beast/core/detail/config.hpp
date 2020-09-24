//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BEAST_CORE_DETAIL_CONFIG_HPP
#define BEAST_CORE_DETAIL_CONFIG_HPP

#include <asio/asio.hpp>

namespace beast {

	namespace net = ::asio;

} // beast

/*
    _MSC_VER and _MSC_FULL_VER by version:

    14.0 (2015)             1900        190023026
    14.0 (2015 Update 1)    1900        190023506
    14.0 (2015 Update 2)    1900        190023918
    14.0 (2015 Update 3)    1900        190024210
*/

#if defined(_MSC_VER)
# if _MSC_FULL_VER < 190024210
#  error Beast requires C++11: Visual Studio 2015 Update 3 or later needed
# endif
#elif defined(__GNUC__)
# if((__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) < 40801)
#  error Beast requires C++11: gcc version 4.8 or later needed
# endif
#else
#endif

#define BEAST_DEPRECATION_STRING \
    "This is a deprecated interface, #define BEAST_ALLOW_DEPRECATED to allow it"

#ifndef BEAST_ASSUME
# ifdef __GNUC__
#  define BEAST_ASSUME(cond) \
    do { if (!(cond)) __builtin_unreachable(); } while (0)
# else
#  define BEAST_ASSUME(cond) do { } while(0)
# endif
#endif

// Default to a header-only implementation. The user must specifically
// request separate compilation by defining BEAST_SEPARATE_COMPILATION
#ifndef BEAST_HEADER_ONLY
# ifndef BEAST_SEPARATE_COMPILATION
#   define BEAST_HEADER_ONLY 1
# endif
#endif

#if BEAST_DOXYGEN
# define BEAST_DECL
#elif defined(BEAST_HEADER_ONLY)
# define BEAST_DECL inline
#else
# define BEAST_DECL
#endif

#ifndef BEAST_ASYNC_RESULT1
#define BEAST_ASYNC_RESULT1(type) \
	ASIO_INITFN_AUTO_RESULT_TYPE(type, void(::beast::error_code))
#endif

#ifndef BEAST_ASYNC_RESULT2
#define BEAST_ASYNC_RESULT2(type) \
	ASIO_INITFN_AUTO_RESULT_TYPE(type, void(::beast::error_code, ::std::size_t))
#endif

#ifndef BEAST_ASYNC_TPARAM1
#define BEAST_ASYNC_TPARAM1 ASIO_COMPLETION_TOKEN_FOR(void(::beast::error_code))
#endif

#ifndef BEAST_ASYNC_TPARAM2
#define BEAST_ASYNC_TPARAM2 ASIO_COMPLETION_TOKEN_FOR(void(::beast::error_code, ::std::size_t))
#endif

#endif
