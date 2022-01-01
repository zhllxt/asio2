//  (C) Copyright John Maddock 2002 - 2003. 
//  (C) Copyright Jens Maurer 2002 - 2003. 
//  (C) Copyright Beman Dawes 2002 - 2003. 
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  Comeau STL:

#if !defined(__LIBCOMO__)
#  include <asio2/bho/config/no_tr1/utility.hpp>
#  if !defined(__LIBCOMO__)
#      error "This is not the Comeau STL!"
#  endif
#endif

//
// std::streambuf<wchar_t> is non-standard
// NOTE: versions of libcomo prior to beta28 have octal version numbering,
// e.g. version 25 is 21 (dec)
#if __LIBCOMO_VERSION__ <= 22
#  define BHO_NO_STD_WSTREAMBUF
#endif

#if (__LIBCOMO_VERSION__ <= 31) && defined(_WIN32)
#define BHO_NO_SWPRINTF
#endif

#if __LIBCOMO_VERSION__ >= 31
#  define BHO_HAS_HASH
#  define BHO_HAS_SLIST
#endif

//  C++0x headers not yet implemented
//
#  define BHO_NO_CXX11_HDR_ARRAY
#  define BHO_NO_CXX11_HDR_CHRONO
#  define BHO_NO_CXX11_HDR_CODECVT
#  define BHO_NO_CXX11_HDR_CONDITION_VARIABLE
#  define BHO_NO_CXX11_HDR_EXCEPTION
#  define BHO_NO_CXX11_HDR_FORWARD_LIST
#  define BHO_NO_CXX11_HDR_FUTURE
#  define BHO_NO_CXX11_HDR_INITIALIZER_LIST
#  define BHO_NO_CXX11_HDR_MUTEX
#  define BHO_NO_CXX11_HDR_RANDOM
#  define BHO_NO_CXX11_HDR_RATIO
#  define BHO_NO_CXX11_HDR_REGEX
#  define BHO_NO_CXX11_HDR_SYSTEM_ERROR
#  define BHO_NO_CXX11_HDR_THREAD
#  define BHO_NO_CXX11_HDR_TUPLE
#  define BHO_NO_CXX11_HDR_TYPE_TRAITS
#  define BHO_NO_CXX11_HDR_TYPEINDEX
#  define BHO_NO_CXX11_HDR_UNORDERED_MAP
#  define BHO_NO_CXX11_HDR_UNORDERED_SET
#  define BHO_NO_CXX11_NUMERIC_LIMITS
#  define BHO_NO_CXX11_ALLOCATOR
#  define BHO_NO_CXX11_POINTER_TRAITS
#  define BHO_NO_CXX11_ATOMIC_SMART_PTR
#  define BHO_NO_CXX11_SMART_PTR
#  define BHO_NO_CXX11_HDR_FUNCTIONAL
#  define BHO_NO_CXX11_HDR_ATOMIC
#  define BHO_NO_CXX11_STD_ALIGN
#  define BHO_NO_CXX11_ADDRESSOF

#if defined(__has_include)
#if !__has_include(<shared_mutex>)
#  define BHO_NO_CXX14_HDR_SHARED_MUTEX
#elif __cplusplus < 201402
#  define BHO_NO_CXX14_HDR_SHARED_MUTEX
#endif
#else
#  define BHO_NO_CXX14_HDR_SHARED_MUTEX
#endif

// C++14 features
#  define BHO_NO_CXX14_STD_EXCHANGE

// C++17 features
#  define BHO_NO_CXX17_STD_APPLY
#  define BHO_NO_CXX17_STD_INVOKE
#  define BHO_NO_CXX17_ITERATOR_TRAITS

//
// Intrinsic type_traits support.
// The SGI STL has it's own __type_traits class, which
// has intrinsic compiler support with SGI's compilers.
// Whatever map SGI style type traits to boost equivalents:
//
#define BHO_HAS_SGI_TYPE_TRAITS

#define BHO_STDLIB "Comeau standard library " BHO_STRINGIZE(__LIBCOMO_VERSION__)
