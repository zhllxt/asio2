//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2014-2014. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/interprocess for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BHO_MOVE_DETAIL_WORKAROUND_HPP
#define BHO_MOVE_DETAIL_WORKAROUND_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif
#
#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#if    !defined(BHO_NO_CXX11_RVALUE_REFERENCES) && !defined(BHO_NO_CXX11_VARIADIC_TEMPLATES)
   #define BHO_MOVE_PERFECT_FORWARDING
#endif

#if defined(__has_feature)
   #define BHO_MOVE_HAS_FEATURE __has_feature
#else
   #define BHO_MOVE_HAS_FEATURE(x) 0
#endif

#if BHO_MOVE_HAS_FEATURE(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
   #define BHO_MOVE_ADDRESS_SANITIZER_ON
#endif

//Macros for documentation purposes. For code, expands to the argument
#define BHO_MOVE_IMPDEF(TYPE) TYPE
#define BHO_MOVE_SEEDOC(TYPE) TYPE
#define BHO_MOVE_DOC0PTR(TYPE) TYPE
#define BHO_MOVE_DOC1ST(TYPE1, TYPE2) TYPE2
#define BHO_MOVE_I ,
#define BHO_MOVE_DOCIGN(T1) T1

#if defined(__GNUC__) && (__GNUC__ == 4) && (__GNUC_MINOR__ < 5) && !defined(__clang__)
   //Pre-standard rvalue binding rules
   #define BHO_MOVE_OLD_RVALUE_REF_BINDING_RULES
#elif defined(_MSC_VER) && (_MSC_VER == 1600)
   //Standard rvalue binding rules but with some bugs
   #define BHO_MOVE_MSVC_10_MEMBER_RVALUE_REF_BUG
   #define BHO_MOVE_MSVC_AUTO_MOVE_RETURN_BUG
#elif defined(_MSC_VER) && (_MSC_VER == 1700)
   #define BHO_MOVE_MSVC_AUTO_MOVE_RETURN_BUG
#endif

//#define BHO_MOVE_DISABLE_FORCEINLINE

#if defined(BHO_MOVE_DISABLE_FORCEINLINE)
   #define BHO_MOVE_FORCEINLINE inline
#elif defined(BHO_MOVE_FORCEINLINE_IS_BHO_FORCELINE)
   #define BHO_MOVE_FORCEINLINE BHO_FORCEINLINE
#elif defined(BHO_MSVC) && (_MSC_VER < 1900 || defined(_DEBUG))
   //"__forceinline" and MSVC seems to have some bugs in old versions and in debug mode
   #define BHO_MOVE_FORCEINLINE inline
#elif defined(BHO_GCC) && (__GNUC__ <= 5)
   //Older GCCs have problems with forceinline
   #define BHO_MOVE_FORCEINLINE inline
#else
   #define BHO_MOVE_FORCEINLINE BHO_FORCEINLINE
#endif

namespace bho {
namespace movelib {

template <typename T1>
BHO_FORCEINLINE BHO_CXX14_CONSTEXPR void ignore(T1 const&)
{}

}} //namespace bho::movelib {

#if !(defined BHO_NO_EXCEPTIONS)
#    define BHO_MOVE_TRY { try
#    define BHO_MOVE_CATCH(x) catch(x)
#    define BHO_MOVE_RETHROW throw;
#    define BHO_MOVE_CATCH_END }
#else
#    if !defined(BHO_MSVC) || BHO_MSVC >= 1900
#        define BHO_MOVE_TRY { if (true)
#        define BHO_MOVE_CATCH(x) else if (false)
#    else
// warning C4127: conditional expression is constant
#        define BHO_MOVE_TRY { \
             __pragma(warning(push)) \
             __pragma(warning(disable: 4127)) \
             if (true) \
             __pragma(warning(pop))
#        define BHO_MOVE_CATCH(x) else \
             __pragma(warning(push)) \
             __pragma(warning(disable: 4127)) \
             if (false) \
             __pragma(warning(pop))
#    endif
#    define BHO_MOVE_RETHROW
#    define BHO_MOVE_CATCH_END }
#endif

#ifndef BHO_NO_CXX11_STATIC_ASSERT
#  ifndef BHO_NO_CXX11_VARIADIC_MACROS
#     define BHO_MOVE_STATIC_ASSERT( ... ) static_assert(__VA_ARGS__, #__VA_ARGS__)
#  else
#     define BHO_MOVE_STATIC_ASSERT( B ) static_assert(B, #B)
#  endif
#else
namespace bho {
namespace move_detail {

template<bool B>
struct STATIC_ASSERTION_FAILURE;

template<>
struct STATIC_ASSERTION_FAILURE<true>{};

template<unsigned> struct static_assert_test {};

}}

#define BHO_MOVE_STATIC_ASSERT(B) \
         typedef ::bho::move_detail::static_assert_test<\
            (unsigned)sizeof(::bho::move_detail::STATIC_ASSERTION_FAILURE<bool(B)>)>\
               BHO_JOIN(bho_static_assert_typedef_, __LINE__) BHO_ATTRIBUTE_UNUSED

#endif

#if !defined(__has_cpp_attribute) || defined(__CUDACC__)
#define BHO_MOVE_HAS_MSVC_ATTRIBUTE(ATTR) 0
#else
#define BHO_MOVE_HAS_MSVC_ATTRIBUTE(ATTR) __has_cpp_attribute(msvc::ATTR)
#endif

// See https://devblogs.microsoft.com/cppblog/improving-the-state-of-debug-performance-in-c/
// for details on how MSVC has improved debug experience, specifically for move/forward-like utilities
#if BHO_MOVE_HAS_MSVC_ATTRIBUTE(intrinsic)
#define BHO_MOVE_INTRINSIC_CAST [[msvc::intrinsic]]
#else
#define BHO_MOVE_INTRINSIC_CAST BHO_MOVE_FORCEINLINE
#endif

#endif   //#ifndef BHO_MOVE_DETAIL_WORKAROUND_HPP

