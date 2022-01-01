//  (C) Copyright John Maddock 2001 - 2003. 
//  (C) Copyright Jens Maurer 2001. 
//  (C) Copyright David Abrahams 2003. 
//  (C) Copyright Boris Gubenko 2007. 
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  Rogue Wave std lib:

#define BHO_RW_STDLIB 1 

#if !defined(__STD_RWCOMPILER_H__) && !defined(_RWSTD_VER)
#  include <asio2/bho/config/no_tr1/utility.hpp>
#  if !defined(__STD_RWCOMPILER_H__) && !defined(_RWSTD_VER)
#     error This is not the Rogue Wave standard library
#  endif
#endif
//
// figure out a consistent version number:
//
#ifndef _RWSTD_VER
#  define BHO_RWSTD_VER 0x010000
#elif _RWSTD_VER < 0x010000
#  define BHO_RWSTD_VER (_RWSTD_VER << 8)
#else
#  define BHO_RWSTD_VER _RWSTD_VER
#endif

#ifndef _RWSTD_VER
#  define BHO_STDLIB "Rogue Wave standard library version (Unknown version)"
#elif _RWSTD_VER < 0x04010200
 #  define BHO_STDLIB "Rogue Wave standard library version " BHO_STRINGIZE(_RWSTD_VER)
#else
#  ifdef _RWSTD_VER_STR
#    define BHO_STDLIB "Apache STDCXX standard library version " _RWSTD_VER_STR
#  else
#    define BHO_STDLIB "Apache STDCXX standard library version " BHO_STRINGIZE(_RWSTD_VER)
#  endif
#endif

//
// Prior to version 2.2.0 the primary template for std::numeric_limits
// does not have compile time constants, even though specializations of that
// template do:
//
#if BHO_RWSTD_VER < 0x020200
#  define BHO_NO_LIMITS_COMPILE_TIME_CONSTANTS
#endif

// Sun CC 5.5 patch 113817-07 adds long long specialization, but does not change the
// library version number (http://sunsolve6.sun.com/search/document.do?assetkey=1-21-113817):
#if BHO_RWSTD_VER <= 0x020101 && (!defined(__SUNPRO_CC) || (__SUNPRO_CC < 0x550))
#  define BHO_NO_LONG_LONG_NUMERIC_LIMITS
# endif

//
// Borland version of numeric_limits lacks __int64 specialisation:
//
#ifdef BHO_BORLANDC
#  define BHO_NO_MS_INT64_NUMERIC_LIMITS
#endif

//
// No std::iterator if it can't figure out default template args:
//
#if defined(_RWSTD_NO_SIMPLE_DEFAULT_TEMPLATES) || defined(RWSTD_NO_SIMPLE_DEFAULT_TEMPLATES) || (BHO_RWSTD_VER < 0x020000)
#  define BHO_NO_STD_ITERATOR
#endif

//
// No iterator traits without partial specialization:
//
#if defined(_RWSTD_NO_CLASS_PARTIAL_SPEC) || defined(RWSTD_NO_CLASS_PARTIAL_SPEC)
#  define BHO_NO_STD_ITERATOR_TRAITS
#endif

//
// Prior to version 2.0, std::auto_ptr was buggy, and there were no
// new-style iostreams, and no conformant std::allocator:
//
#if (BHO_RWSTD_VER < 0x020000)
#  define BHO_NO_AUTO_PTR
#  define BHO_NO_STRINGSTREAM
#  define BHO_NO_STD_ALLOCATOR
#  define BHO_NO_STD_LOCALE
#endif

//
// No template iterator constructors without member template support:
//
#if defined(RWSTD_NO_MEMBER_TEMPLATES) || defined(_RWSTD_NO_MEMBER_TEMPLATES)
#  define BHO_NO_TEMPLATED_ITERATOR_CONSTRUCTORS
#endif

//
// RW defines _RWSTD_ALLOCATOR if the allocator is conformant and in use
// (the or _HPACC_ part is a hack - the library seems to define _RWSTD_ALLOCATOR
// on HP aCC systems even though the allocator is in fact broken):
//
#if !defined(_RWSTD_ALLOCATOR) || (defined(__HP_aCC) && __HP_aCC <= 33100)
#  define BHO_NO_STD_ALLOCATOR
#endif

//
// If we have a std::locale, we still may not have std::use_facet:
//
#if defined(_RWSTD_NO_TEMPLATE_ON_RETURN_TYPE) && !defined(BHO_NO_STD_LOCALE)
#  define BHO_NO_STD_USE_FACET
#  define BHO_HAS_TWO_ARG_USE_FACET
#endif

//
// There's no std::distance prior to version 2, or without
// partial specialization support:
//
#if (BHO_RWSTD_VER < 0x020000) || defined(_RWSTD_NO_CLASS_PARTIAL_SPEC)
    #define BHO_NO_STD_DISTANCE
#endif

//
// Some versions of the rogue wave library don't have assignable
// OutputIterators:
//
#if BHO_RWSTD_VER < 0x020100
#  define BHO_NO_STD_OUTPUT_ITERATOR_ASSIGN
#endif

//
// Disable BHO_HAS_LONG_LONG when the library has no support for it.
//
#if !defined(_RWSTD_LONG_LONG) && defined(BHO_HAS_LONG_LONG)
#  undef BHO_HAS_LONG_LONG
#endif

//
// check that on HP-UX, the proper RW library is used
//
#if defined(__HP_aCC) && !defined(_HP_NAMESPACE_STD)
#  error "Boost requires Standard RW library. Please compile and link with -AA"
#endif

//
// Define macros specific to RW V2.2 on HP-UX
//
#if defined(__HP_aCC) && (BHO_RWSTD_VER == 0x02020100)
#  ifndef __HP_TC1_MAKE_PAIR
#    define __HP_TC1_MAKE_PAIR
#  endif
#  ifndef _HP_INSTANTIATE_STD2_VL
#    define _HP_INSTANTIATE_STD2_VL
#  endif
#endif

#if _RWSTD_VER < 0x05000000
#  define BHO_NO_CXX11_HDR_ARRAY
#endif
// type_traits header is incomplete:
#  define BHO_NO_CXX11_HDR_TYPE_TRAITS
//
//  C++0x headers not yet implemented
//
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
