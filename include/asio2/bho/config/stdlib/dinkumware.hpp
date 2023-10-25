//  (C) Copyright John Maddock 2001 - 2003.
//  (C) Copyright Jens Maurer 2001.
//  (C) Copyright Peter Dimov 2001.
//  (C) Copyright David Abrahams 2002.
//  (C) Copyright Guillaume Melquiond 2003.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  Dinkumware standard library config:

#if !defined(_YVALS) && !defined(_CPPLIB_VER)
#include <asio2/bho/config/no_tr1/utility.hpp>
#if !defined(_YVALS) && !defined(_CPPLIB_VER)
#error This is not the Dinkumware lib!
#endif
#endif


#if defined(_CPPLIB_VER) && (_CPPLIB_VER >= 306)
   // full dinkumware 3.06 and above
   // fully conforming provided the compiler supports it:
#  if !(defined(_GLOBAL_USING) && (_GLOBAL_USING+0 > 0)) && !defined(BHO_BORLANDC) && !defined(_STD) && !(defined(__ICC) && (__ICC >= 700))   // can be defined in yvals.h
#     define BHO_NO_STDC_NAMESPACE
#  endif
#  if !(defined(_HAS_MEMBER_TEMPLATES_REBIND) && (_HAS_MEMBER_TEMPLATES_REBIND+0 > 0)) && !(defined(_MSC_VER) && (_MSC_VER > 1300)) && defined(BHO_MSVC)
#     define BHO_NO_STD_ALLOCATOR
#  endif
#  define BHO_HAS_PARTIAL_STD_ALLOCATOR
#  if defined(BHO_MSVC) && (BHO_MSVC < 1300)
      // if this lib version is set up for vc6 then there is no std::use_facet:
#     define BHO_NO_STD_USE_FACET
#     define BHO_HAS_TWO_ARG_USE_FACET
      // C lib functions aren't in namespace std either:
#     define BHO_NO_STDC_NAMESPACE
      // and nor is <exception>
#     define BHO_NO_EXCEPTION_STD_NAMESPACE
#  endif
// There's no numeric_limits<long long> support unless _LONGLONG is defined:
#  if !defined(_LONGLONG) && (_CPPLIB_VER <= 310)
#     define BHO_NO_MS_INT64_NUMERIC_LIMITS
#  endif
// 3.06 appears to have (non-sgi versions of) <hash_set> & <hash_map>,
// and no <slist> at all
#else
#  define BHO_MSVC_STD_ITERATOR 1
#  define BHO_NO_STD_ITERATOR
#  define BHO_NO_TEMPLATED_ITERATOR_CONSTRUCTORS
#  define BHO_NO_STD_ALLOCATOR
#  define BHO_NO_STDC_NAMESPACE
#  define BHO_NO_STD_USE_FACET
#  define BHO_NO_STD_OUTPUT_ITERATOR_ASSIGN
#  define BHO_HAS_MACRO_USE_FACET
#  ifndef _CPPLIB_VER
      // Updated Dinkum library defines this, and provides
      // its own min and max definitions, as does MTA version.
#     ifndef __MTA__ 
#        define BHO_NO_STD_MIN_MAX
#     endif
#     define BHO_NO_MS_INT64_NUMERIC_LIMITS
#  endif
#endif

//
// std extension namespace is stdext for vc7.1 and later, 
// the same applies to other compilers that sit on top
// of vc7.1 (Intel and Comeau):
//
#if defined(_MSC_VER) && (_MSC_VER >= 1310) && !defined(BHO_BORLANDC)
#  define BHO_STD_EXTENSION_NAMESPACE stdext
#endif


#if (defined(_MSC_VER) && (_MSC_VER <= 1300) && !defined(BHO_BORLANDC)) || !defined(_CPPLIB_VER) || (_CPPLIB_VER < 306)
   // if we're using a dinkum lib that's
   // been configured for VC6/7 then there is
   // no iterator traits (true even for icl)
#  define BHO_NO_STD_ITERATOR_TRAITS
#endif

#if defined(__ICL) && (__ICL < 800) && defined(_CPPLIB_VER) && (_CPPLIB_VER <= 310)
// Intel C++ chokes over any non-trivial use of <locale>
// this may be an overly restrictive define, but regex fails without it:
#  define BHO_NO_STD_LOCALE
#endif

#if ((defined(BHO_MSVC) && BHO_MSVC >= 1400) || (defined(__clang__) && defined(_MSC_VER))) && (_MSC_VER < 1800)
// Fix for VC++ 8.0 on up ( I do not have a previous version to test )
// or clang-cl. If exceptions are off you must manually include the 
// <exception> header before including the <typeinfo> header. Admittedly 
// trying to use Boost libraries or the standard C++ libraries without 
// exception support is not suggested but currently clang-cl ( v 3.4 ) 
// does not support exceptions and must be compiled with exceptions off.
#if !_HAS_EXCEPTIONS
#include <exception>
#endif
#include <typeinfo>
#if !_HAS_EXCEPTIONS
#  define BHO_NO_STD_TYPEINFO
#endif  
#endif
#if defined(__ghs__) && !_HAS_NAMESPACE
#  define BHO_NO_STD_TYPEINFO
#endif

//  C++0x headers implemented in 520 (as shipped by Microsoft)
//
#if !defined(_CPPLIB_VER) || _CPPLIB_VER < 520
#  define BHO_NO_CXX11_HDR_ARRAY
#  define BHO_NO_CXX11_HDR_CODECVT
#  define BHO_NO_CXX11_HDR_FORWARD_LIST
#  define BHO_NO_CXX11_HDR_INITIALIZER_LIST
#  define BHO_NO_CXX11_HDR_RANDOM
#  define BHO_NO_CXX11_HDR_REGEX
#  define BHO_NO_CXX11_HDR_SYSTEM_ERROR
#  define BHO_NO_CXX11_HDR_UNORDERED_MAP
#  define BHO_NO_CXX11_HDR_UNORDERED_SET
#  define BHO_NO_CXX11_HDR_TUPLE
#  define BHO_NO_CXX11_HDR_TYPEINDEX
#  define BHO_NO_CXX11_HDR_FUNCTIONAL
#  define BHO_NO_CXX11_NUMERIC_LIMITS
#  define BHO_NO_CXX11_SMART_PTR
#endif

#if ((!defined(_HAS_TR1_IMPORTS) || (_HAS_TR1_IMPORTS+0 == 0)) && !defined(BHO_NO_CXX11_HDR_TUPLE)) \
  && (!defined(_CPPLIB_VER) || _CPPLIB_VER < 610)
#  define BHO_NO_CXX11_HDR_TUPLE
#endif

//  C++0x headers implemented in 540 (as shipped by Microsoft)
//
#if !defined(_CPPLIB_VER) || _CPPLIB_VER < 540
#  define BHO_NO_CXX11_HDR_TYPE_TRAITS
#  define BHO_NO_CXX11_HDR_CHRONO
#  define BHO_NO_CXX11_HDR_CONDITION_VARIABLE
#  define BHO_NO_CXX11_HDR_FUTURE
#  define BHO_NO_CXX11_HDR_MUTEX
#  define BHO_NO_CXX11_HDR_RATIO
#  define BHO_NO_CXX11_HDR_THREAD
#  define BHO_NO_CXX11_ATOMIC_SMART_PTR
#  define BHO_NO_CXX11_HDR_EXCEPTION
#endif

//  C++0x headers implemented in 610 (as shipped by Microsoft)
//
#if !defined(_CPPLIB_VER) || _CPPLIB_VER < 610
#  define BHO_NO_CXX11_HDR_INITIALIZER_LIST
#  define BHO_NO_CXX11_HDR_ATOMIC
#  define BHO_NO_CXX11_ALLOCATOR
// 540 has std::align but it is not a conforming implementation
#  define BHO_NO_CXX11_STD_ALIGN
#endif

// Before 650 std::pointer_traits has a broken rebind template
#if !defined(_CPPLIB_VER) || _CPPLIB_VER < 650
#  define BHO_NO_CXX11_POINTER_TRAITS
#elif defined(BHO_MSVC) && BHO_MSVC < 1910
#  define BHO_NO_CXX11_POINTER_TRAITS
#endif

#if defined(__has_include)
#if !__has_include(<shared_mutex>)
#  define BHO_NO_CXX14_HDR_SHARED_MUTEX
#elif (__cplusplus < 201402) && !defined(_MSC_VER)
#  define BHO_NO_CXX14_HDR_SHARED_MUTEX
#endif
#elif !defined(_CPPLIB_VER) || (_CPPLIB_VER < 650)
#  define BHO_NO_CXX14_HDR_SHARED_MUTEX
#endif

// C++14 features
#if !defined(_CPPLIB_VER) || (_CPPLIB_VER < 650)
#  define BHO_NO_CXX14_STD_EXCHANGE
#endif

// C++17 features
#if !defined(_CPPLIB_VER) || (_CPPLIB_VER < 650) \
 || ((!defined(BHO_MSVC) || (BHO_MSVC < 1910))) && (!defined(__clang__) || !defined(_MSC_VER) || (_MSC_VER < 1929))\
 || !defined(_HAS_CXX17) || (_HAS_CXX17 == 0)
#  define BHO_NO_CXX17_STD_APPLY
#  define BHO_NO_CXX17_ITERATOR_TRAITS
#  define BHO_NO_CXX17_HDR_STRING_VIEW
#  define BHO_NO_CXX17_HDR_OPTIONAL
#  define BHO_NO_CXX17_HDR_VARIANT
#  define BHO_NO_CXX17_HDR_ANY
#  define BHO_NO_CXX17_HDR_MEMORY_RESOURCE
#  define BHO_NO_CXX17_HDR_CHARCONV
#  define BHO_NO_CXX17_HDR_EXECUTION
#  define BHO_NO_CXX17_HDR_FILESYSTEM
#endif
#if !defined(_CPPLIB_VER) || (_CPPLIB_VER < 650) || !defined(_HAS_CXX17) || (_HAS_CXX17 == 0) || !defined(_MSVC_STL_UPDATE) || (_MSVC_STL_UPDATE < 201709)
#  define BHO_NO_CXX17_STD_INVOKE
#endif

// C++20 features which aren't configured in suffix.hpp correctly:
#if !defined(_MSVC_STL_UPDATE) || (_MSVC_STL_UPDATE < 202008L) || !defined(_HAS_CXX20) || (_HAS_CXX20 == 0)
#  define BHO_NO_CXX20_HDR_CONCEPTS
#endif

#if !(!defined(_CPPLIB_VER) || (_CPPLIB_VER < 650) || !defined(BHO_MSVC) || (BHO_MSVC < 1912) || !defined(_HAS_CXX17) || (_HAS_CXX17 == 0))
// Deprecated std::iterator:
#  define BHO_NO_STD_ITERATOR
#endif

#if defined(BHO_INTEL) && (BHO_INTEL <= 1400)
// Intel's compiler can't handle this header yet:
#  define BHO_NO_CXX11_HDR_ATOMIC
#endif


//  520..610 have std::addressof, but it doesn't support functions
//
#if !defined(_CPPLIB_VER) || _CPPLIB_VER < 650
#  define BHO_NO_CXX11_ADDRESSOF
#endif

// Bug specific to VC14, 
// See https://connect.microsoft.com/VisualStudio/feedback/details/1348277/link-error-when-using-std-codecvt-utf8-utf16-char16-t
// and discussion here: http://blogs.msdn.com/b/vcblog/archive/2014/11/12/visual-studio-2015-preview-now-available.aspx?PageIndex=2
#if defined(_CPPLIB_VER) && (_CPPLIB_VER == 650) && (!defined(_MSVC_STL_VERSION) || (_MSVC_STL_VERSION < 142))
#  define BHO_NO_CXX11_HDR_CODECVT
#endif

#if (_MSVC_LANG > 201700) && !defined(BHO_NO_CXX11_HDR_CODECVT)
//
// <codecvt> is deprected as of C++17, and by default MSVC emits hard errors
// if you try to use it, so mark it as unavailable:
//
#  define BHO_NO_CXX11_HDR_CODECVT
#endif

#if defined(_CPPLIB_VER) && (_CPPLIB_VER >= 650)
// If _HAS_AUTO_PTR_ETC is defined to 0, std::auto_ptr and std::random_shuffle are not available.
// See https://www.visualstudio.com/en-us/news/vs2015-vs.aspx#C++
// and http://blogs.msdn.com/b/vcblog/archive/2015/06/19/c-11-14-17-features-in-vs-2015-rtm.aspx
#  if defined(_HAS_AUTO_PTR_ETC) && (_HAS_AUTO_PTR_ETC == 0)
#    define BHO_NO_AUTO_PTR
#    define BHO_NO_CXX98_RANDOM_SHUFFLE
#    define BHO_NO_CXX98_FUNCTION_BASE
#    define BHO_NO_CXX98_BINDERS
#  elif defined(_HAS_DEPRECATED_ADAPTOR_TYPEDEFS) && (_HAS_DEPRECATED_ADAPTOR_TYPEDEFS == 0)
#    define BHO_NO_CXX98_BINDERS
#  endif
#endif
//
// Things deprecated in C++20:
//
#if defined(_HAS_CXX20)
#  define BHO_NO_CXX11_ATOMIC_SMART_PTR
#endif


//
// Things not supported by the CLR:
#ifdef _M_CEE
#ifndef BHO_NO_CXX11_HDR_MUTEX
#  define BHO_NO_CXX11_HDR_MUTEX
#endif
#ifndef BHO_NO_CXX11_HDR_ATOMIC
#  define BHO_NO_CXX11_HDR_ATOMIC
#endif
#ifndef BHO_NO_CXX11_HDR_FUTURE
#  define BHO_NO_CXX11_HDR_FUTURE
#endif
#ifndef BHO_NO_CXX11_HDR_CONDITION_VARIABLE
#  define BHO_NO_CXX11_HDR_CONDITION_VARIABLE
#endif
#ifndef BHO_NO_CXX11_HDR_THREAD
#  define BHO_NO_CXX11_HDR_THREAD
#endif
#ifndef BHO_NO_CXX14_HDR_SHARED_MUTEX
#  define BHO_NO_CXX14_HDR_SHARED_MUTEX
#endif
#ifndef BHO_NO_CXX14_STD_EXCHANGE
#  define BHO_NO_CXX14_STD_EXCHANGE
#endif
#ifndef BHO_NO_FENV_H
#  define BHO_NO_FENV_H
#endif
#endif

#ifdef _CPPLIB_VER
#  define BHO_DINKUMWARE_STDLIB _CPPLIB_VER
#else
#  define BHO_DINKUMWARE_STDLIB 1
#endif

// BHO_MSSTL_VERSION: as _MSVC_STL_VERSION, but for earlier releases as well

#if defined(_MSVC_STL_VERSION) // VS2017 (14.1) and above
#  define BHO_MSSTL_VERSION _MSVC_STL_VERSION

#elif defined(_CPPLIB_VER) && _CPPLIB_VER >= 650 // VS2015 (14.0)
#  define BHO_MSSTL_VERSION 140

#elif defined(_CPPLIB_VER) && _CPPLIB_VER >= 610 // VS2013 (12.0)
#  define BHO_MSSTL_VERSION 120

#elif defined(_CPPLIB_VER) && _CPPLIB_VER >= 540 // VS2012 (11.0)
#  define BHO_MSSTL_VERSION 110

#elif defined(_CPPLIB_VER) && _CPPLIB_VER >= 520 // VS2010 (10.0)
#  define BHO_MSSTL_VERSION 100

#elif defined(_CPPLIB_VER) && _CPPLIB_VER >= 505 // VS2008SP1 (9.0)
#  define BHO_MSSTL_VERSION 91

#elif defined(_CPPLIB_VER) && _CPPLIB_VER >= 503 // VS2008 (also 9.0)
#  define BHO_MSSTL_VERSION 90

#elif defined(_CPPLIB_VER) && _CPPLIB_VER >= 405 // VS2005 (8.0)
#  define BHO_MSSTL_VERSION 80

#endif

//

#ifdef _CPPLIB_VER
#  define BHO_STDLIB "Dinkumware standard library version " BHO_STRINGIZE(_CPPLIB_VER)
#else
#  define BHO_STDLIB "Dinkumware standard library version 1.x"
#endif
