//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
// Copyright (c) 2019-2020 Krystian Stasiowski (sdkrystian at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/static_string
//

#ifndef BHO_STATIC_STRING_CONFIG_HPP
#define BHO_STATIC_STRING_CONFIG_HPP

#ifndef BHO_STATIC_STRING_STANDALONE
#define BHO_STATIC_STRING_STANDALONE
#endif

// Are we dependent on Boost?
// #define BHO_STATIC_STRING_STANDALONE

// Can we have deduction guides?
#if __cpp_deduction_guides >= 201703L
#define BHO_STATIC_STRING_USE_DEDUCT
#endif

// Include <version> if we can
#ifdef __has_include
#if __has_include(<version>)
#include <version>
#endif
#endif

// Can we use __has_builtin?
#ifdef __has_builtin
#define BHO_STATIC_STRING_HAS_BUILTIN(arg) __has_builtin(arg)
#else
#define BHO_STATIC_STRING_HAS_BUILTIN(arg) 0
#endif

// Can we use is_constant_evaluated?
#if __cpp_lib_is_constant_evaluated >= 201811L
#define BHO_STATIC_STRING_IS_CONST_EVAL std::is_constant_evaluated()
#elif BHO_STATIC_STRING_HAS_BUILTIN(__builtin_is_constant_evaluated)
#define BHO_STATIC_STRING_IS_CONST_EVAL __builtin_is_constant_evaluated()
#endif

// Check for an attribute
#if defined(__has_cpp_attribute)
#define BHO_STATIC_STRING_CHECK_FOR_ATTR(x) __has_cpp_attribute(x)
#elif defined(__has_attribute)
#define BHO_STATIC_STRING_CHECK_FOR_ATTR(x) __has_attribute(x)
#else
#define BHO_STATIC_STRING_CHECK_FOR_ATTR(x) 0
#endif

// Decide which attributes we can use
#define BHO_STATIC_STRING_UNLIKELY
#define BHO_STATIC_STRING_NODISCARD
#define BHO_STATIC_STRING_NORETURN
#define BHO_STATIC_STRING_NO_NORETURN
// unlikely
#if BHO_STATIC_STRING_CHECK_FOR_ATTR(unlikely)
#undef BHO_STATIC_STRING_UNLIKELY
#define BHO_STATIC_STRING_UNLIKELY [[unlikely]]
#endif
// nodiscard
#if BHO_STATIC_STRING_CHECK_FOR_ATTR(nodiscard)
#undef BHO_STATIC_STRING_NODISCARD
#define BHO_STATIC_STRING_NODISCARD [[nodiscard]]
#elif defined(_MSC_VER) && _MSC_VER >= 1700
#undef BHO_STATIC_STRING_NODISCARD
#define BHO_STATIC_STRING_NODISCARD _Check_return_
#elif defined(__GNUC__) || defined(__clang__)
#undef BHO_STATIC_STRING_NODISCARD
#define BHO_STATIC_STRING_NODISCARD __attribute__((warn_unused_result))
#endif
// noreturn
#if BHO_STATIC_STRING_CHECK_FOR_ATTR(noreturn)
#undef BHO_STATIC_STRING_NORETURN
#undef BHO_STATIC_STRING_NO_NORETURN
#define BHO_STATIC_STRING_NORETURN [[noreturn]]
#elif defined(_MSC_VER)
#undef BHO_STATIC_STRING_NORETURN
#undef BHO_STATIC_STRING_NO_NORETURN
#define BHO_STATIC_STRING_NORETURN __declspec(noreturn)
#elif defined(__GNUC__) || defined(__clang__)
#undef BHO_STATIC_STRING_NORETURN
#undef BHO_STATIC_STRING_NO_NORETURN
#define BHO_STATIC_STRING_NORETURN __attribute__((__noreturn__))
#endif

// _MSVC_LANG isn't avaliable until after VS2015
#if defined(_MSC_VER) && _MSC_VER < 1910L
// The constexpr support in this version is effectively that of
// c++11, so we treat it as such
#define BHO_STATIC_STRING_STANDARD_VERSION 201103L
#elif defined(_MSVC_LANG)
// MSVC doesn't define __cplusplus by default
#define BHO_STATIC_STRING_STANDARD_VERSION _MSVC_LANG
#else
#define BHO_STATIC_STRING_STANDARD_VERSION __cplusplus
#endif

// Decide what level of constexpr we can use
#define BHO_STATIC_STRING_CPP20_CONSTEXPR
#define BHO_STATIC_STRING_CPP17_CONSTEXPR
#define BHO_STATIC_STRING_CPP14_CONSTEXPR
#define BHO_STATIC_STRING_CPP11_CONSTEXPR
#if BHO_STATIC_STRING_STANDARD_VERSION >= 202002L
#define BHO_STATIC_STRING_CPP20
#undef BHO_STATIC_STRING_CPP20_CONSTEXPR
#define BHO_STATIC_STRING_CPP20_CONSTEXPR constexpr
#endif
#if BHO_STATIC_STRING_STANDARD_VERSION >= 201703L
#define BHO_STATIC_STRING_CPP17
#undef BHO_STATIC_STRING_CPP17_CONSTEXPR
#define BHO_STATIC_STRING_CPP17_CONSTEXPR constexpr
#endif
#if BHO_STATIC_STRING_STANDARD_VERSION >= 201402L
#define BHO_STATIC_STRING_CPP14
#undef BHO_STATIC_STRING_CPP14_CONSTEXPR
#define BHO_STATIC_STRING_CPP14_CONSTEXPR constexpr
#endif
#if BHO_STATIC_STRING_STANDARD_VERSION >= 201103L
#define BHO_STATIC_STRING_CPP11
#undef BHO_STATIC_STRING_CPP11_CONSTEXPR
#define BHO_STATIC_STRING_CPP11_CONSTEXPR constexpr
#endif

// Boost and non-Boost versions of utilities
#ifndef BHO_STATIC_STRING_STANDALONE
#ifndef BHO_STATIC_STRING_THROW
#define BHO_STATIC_STRING_THROW(ex) BHO_THROW_EXCEPTION(ex)
#endif
#ifndef BHO_STATIC_STRING_STATIC_ASSERT
#define BHO_STATIC_STRING_STATIC_ASSERT(cond, msg) BHO_STATIC_ASSERT_MSG(cond, msg)
#endif
#ifndef BHO_STATIC_STRING_ASSERT
#define BHO_STATIC_STRING_ASSERT(cond) BHO_ASSERT(cond)
#endif
#else
#ifndef BHO_STATIC_STRING_THROW
#define BHO_STATIC_STRING_THROW(ex) throw ex
#endif
#ifndef BHO_STATIC_STRING_STATIC_ASSERT
#define BHO_STATIC_STRING_STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#endif
#ifndef BHO_STATIC_STRING_ASSERT
#define BHO_STATIC_STRING_ASSERT(cond) assert(cond)
#endif
#endif

#ifndef BHO_STATIC_STRING_STANDALONE
#include <asio2/bho/config.hpp>
#include <asio2/bho/assert.hpp>
#include <asio2/bho/container_hash/hash.hpp>
#include <asio2/bho/static_assert.hpp>
#include <asio2/bho/utility/string_view.hpp>
#include <asio2/bho/core/detail/string_view.hpp>
#include <asio2/bho/throw_exception.hpp>

#if !defined(BHO_NO_CXX17_HDR_STRING_VIEW) || \
     defined(BHO_STATIC_STRING_CXX17_STRING_VIEW)
#include <string_view>
#define BHO_STATIC_STRING_HAS_STD_STRING_VIEW
#endif
#else
#include <cassert>
#include <stdexcept>

/*
 * Replicate the logic from BHO.Config
 */
// GNU libstdc++3:
#if defined(__GLIBCPP__) || defined(__GLIBCXX__)
#if (BHO_LIBSTDCXX_VERSION < 70100) || (__cplusplus <= 201402L)
#  define BHO_STATIC_STRING_NO_CXX17_HDR_STRING_VIEW
#endif
// libc++:
#elif defined(_LIBCPP_VERSION)
#if (_LIBCPP_VERSION < 4000) || (__cplusplus <= 201402L)
#  define BHO_STATIC_STRING_NO_CXX17_HDR_STRING_VIEW
#endif
// MSVC uses logic from catch all for BHO_NO_CXX17_HDR_STRING_VIEW
// catch all:
#elif !defined(_YVALS) && !defined(_CPPLIB_VER)
#if (!defined(__has_include) || (__cplusplus < 201700))
#  define BHO_STATIC_STRING_NO_CXX17_HDR_STRING_VIEW
#elif !__has_include(<string_view>)
#  define BHO_STATIC_STRING_NO_CXX17_HDR_STRING_VIEW
#endif
#endif

#if !defined(BHO_STATIC_STRING_NO_CXX17_HDR_STRING_VIEW) || \
     defined(BHO_STATIC_STRING_CXX17_STRING_VIEW)
#include <string_view>
#define BHO_STATIC_STRING_HAS_STD_STRING_VIEW
#endif
#endif

// Compiler bug prevents constexpr from working with clang 4.x and 5.x
// if it is detected, we disable constexpr.
#if (BHO_STATIC_STRING_STANDARD_VERSION >= 201402L && \
BHO_STATIC_STRING_STANDARD_VERSION < 201703L) && \
defined(__clang__) && \
((__clang_major__ == 4) || (__clang_major__ == 5))
// This directive works on clang
#warning "C++14 constexpr is not supported in clang 4.x and 5.x due to a compiler bug."
#ifdef BHO_STATIC_STRING_CPP14
#undef BHO_STATIC_STRING_CPP14
#endif
#undef BHO_STATIC_STRING_CPP14_CONSTEXPR
#define BHO_STATIC_STRING_CPP14_CONSTEXPR
#endif

// This is for compiler/library configurations
// that cannot use the library comparison function
// objects at all in constant expresssions. In these
// cases, we use whatever will make more constexpr work.
#if defined(__clang__) && \
(defined(__GLIBCXX__) || defined(_MSC_VER))
#define BHO_STATIC_STRING_NO_PTR_COMP_FUNCTIONS
#endif

// In gcc-5, we cannot use throw expressions in a
// constexpr function. However, we have a workaround
// for this using constructors. Also, non-static member
// functions that return the class they are a member of
// causes an ICE during constant evaluation.
#if defined(__GNUC__) && (__GNUC__== 5) && \
defined(BHO_STATIC_STRING_CPP14)
#define BHO_STATIC_STRING_GCC5_BAD_CONSTEXPR
#endif

// Define the basic string_view type used by the library
// Conversions to and from other available string_view types
// are still defined.
#if !defined(BHO_STATIC_STRING_STANDALONE) || \
     defined(BHO_STATIC_STRING_HAS_STD_STRING_VIEW)
#define BHO_STATIC_STRING_HAS_ANY_STRING_VIEW
namespace bho {
namespace static_strings {

/// The type of `basic_string_view` used by the library
template<typename CharT, typename Traits>
using basic_string_view =
#ifndef BHO_STATIC_STRING_STANDALONE
  bho::basic_string_view<CharT, Traits>;
#else
  std::basic_string_view<CharT, Traits>;
#endif
} // static_strings
} // bho
#endif

#endif
