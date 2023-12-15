//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BHO_BEAST_CORE_DETAIL_CONFIG_HPP
#define BHO_BEAST_CORE_DETAIL_CONFIG_HPP

// Available to every header
#include <asio2/bho/config.hpp>
#include <asio2/bho/version.hpp>
#include <asio2/bho/core/ignore_unused.hpp>
#include <asio2/bho/static_assert.hpp>

#ifndef BHO_BEAST_NO_SOURCE_LOCATION
#define BHO_BEAST_NO_SOURCE_LOCATION
#endif

namespace bho {
namespace beast {
namespace net = ::asio;
} // beast
} // bho

/*
    _MSC_VER and _MSC_FULL_VER by version:

    14.0 (2015)             1900        190023026
    14.0 (2015 Update 1)    1900        190023506
    14.0 (2015 Update 2)    1900        190023918
    14.0 (2015 Update 3)    1900        190024210
*/

#if defined(BHO_MSVC)
# if BHO_MSVC_FULL_VER < 190024210
#  error Beast requires C++11: Visual Studio 2015 Update 3 or later needed
# endif

#elif defined(BHO_GCC)
# if(BHO_GCC < 50000)
#  error Beast requires C++11: gcc version 5 or later needed
# endif

#else
# if \
    defined(BHO_NO_CXX11_DECLTYPE) || \
    defined(BHO_NO_CXX11_HDR_TUPLE) || \
    defined(BHO_NO_CXX11_TEMPLATE_ALIASES) || \
    defined(BHO_NO_CXX11_VARIADIC_TEMPLATES)
#  error Beast requires C++11: a conforming compiler is needed
# endif

#endif

#define BHO_BEAST_DEPRECATION_STRING \
    "This is a deprecated interface, #define BHO_BEAST_ALLOW_DEPRECATED to allow it"

#ifndef BHO_BEAST_ASSUME
# ifdef BHO_GCC
#  define BHO_BEAST_ASSUME(cond) \
    do { if (!(cond)) __builtin_unreachable(); } while (0)
# else
#  define BHO_BEAST_ASSUME(cond) do { } while(0)
# endif
#endif

// Default to a header-only implementation. The user must specifically
// request separate compilation by defining BHO_BEAST_SEPARATE_COMPILATION
#ifndef BEAST_HEADER_ONLY
# ifndef BHO_BEAST_SEPARATE_COMPILATION
#   define BEAST_HEADER_ONLY 1
# endif
#endif

#if BHO_BEAST_DOXYGEN
# define BHO_BEAST_DECL
#elif defined(BEAST_HEADER_ONLY)
# define BHO_BEAST_DECL inline
#else
# define BHO_BEAST_DECL
#endif

#ifndef BHO_BEAST_ASYNC_RESULT1
#define BHO_BEAST_ASYNC_RESULT1(type) \
    ASIO_INITFN_AUTO_RESULT_TYPE(type, void(::bho::beast::error_code))
#endif

#ifndef BHO_BEAST_ASYNC_RESULT2
#define BHO_BEAST_ASYNC_RESULT2(type) \
    ASIO_INITFN_AUTO_RESULT_TYPE(type, void(::bho::beast::error_code, ::std::size_t))
#endif

#ifndef BHO_BEAST_ASYNC_TPARAM1
#define BHO_BEAST_ASYNC_TPARAM1 ASIO_COMPLETION_TOKEN_FOR(void(::bho::beast::error_code))
#endif

#ifndef BHO_BEAST_ASYNC_TPARAM2
#define BHO_BEAST_ASYNC_TPARAM2 ASIO_COMPLETION_TOKEN_FOR(void(::bho::beast::error_code, ::std::size_t))
#endif


#ifdef BHO_BEAST_NO_SOURCE_LOCATION
#define BHO_BEAST_ASSIGN_EC(ec, error) ec = error
#else

#define BHO_BEAST_ASSIGN_EC(ec, error) \
    static constexpr auto BHO_PP_CAT(loc_, __LINE__) ((BHO_CURRENT_LOCATION)); \
    ec.assign(error, & BHO_PP_CAT(loc_, __LINE__) )

#endif

#ifndef BHO_BEAST_FILE_BUFFER_SIZE
#define BHO_BEAST_FILE_BUFFER_SIZE 4096
#endif

#endif
