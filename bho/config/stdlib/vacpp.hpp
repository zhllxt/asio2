//  (C) Copyright John Maddock 2001 - 2002. 
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

#if __IBMCPP__ <= 501
#  define BHO_NO_STD_ALLOCATOR
#endif

#define BHO_HAS_MACRO_USE_FACET
#define BHO_NO_STD_MESSAGES

// Apple doesn't seem to reliably defined a *unix* macro
#if !defined(CYGWIN) && (  defined(__unix__)  \
                        || defined(__unix)    \
                        || defined(unix)      \
                        || defined(__APPLE__) \
                        || defined(__APPLE)   \
                        || defined(APPLE))
#  include <unistd.h>
#endif

//  C++0x headers not yet implemented
//
#  define BHO_NO_CXX11_HDR_ARRAY
#  define BHO_NO_CXX11_HDR_CHRONO
#  define BHO_NO_CXX11_HDR_CODECVT
#  define BHO_NO_CXX11_HDR_CONDITION_VARIABLE
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
#  define BHO_NO_CXX11_HDR_EXCEPTION

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

#define BHO_STDLIB "Visual Age default standard library"
