/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_IBM_H
#define BHO_PREDEF_COMPILER_IBM_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_COMP_IBM`

http://en.wikipedia.org/wiki/VisualAge[IBM XL C/{CPP}] compiler.
Version number available as major, minor, and patch.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__IBMCPP__+` | {predef_detection}
| `+__xlC__+` | {predef_detection}
| `+__xlc__+` | {predef_detection}

| `+__COMPILER_VER__+` | V.R.P
| `+__xlC__+` | V.R.P
| `+__xlc__+` | V.R.P
| `+__IBMCPP__+` | V.R.P
|===
*/ // end::reference[]

#define BHO_COMP_IBM BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__IBMCPP__) || defined(__xlC__) || defined(__xlc__)
#   if !defined(BHO_COMP_IBM_DETECTION) && defined(__COMPILER_VER__)
#       define BHO_COMP_IBM_DETECTION BHO_PREDEF_MAKE_0X_VRRPPPP(__COMPILER_VER__)
#   endif
#   if !defined(BHO_COMP_IBM_DETECTION) && defined(__xlC__)
#       define BHO_COMP_IBM_DETECTION BHO_PREDEF_MAKE_0X_VVRR(__xlC__)
#   endif
#   if !defined(BHO_COMP_IBM_DETECTION) && defined(__xlc__)
#       define BHO_COMP_IBM_DETECTION BHO_PREDEF_MAKE_0X_VVRR(__xlc__)
#   endif
#   if !defined(BHO_COMP_IBM_DETECTION)
#       define BHO_COMP_IBM_DETECTION BHO_PREDEF_MAKE_10_VRP(__IBMCPP__)
#   endif
#endif

#ifdef BHO_COMP_IBM_DETECTION
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_IBM_EMULATED BHO_COMP_IBM_DETECTION
#   else
#       undef BHO_COMP_IBM
#       define BHO_COMP_IBM BHO_COMP_IBM_DETECTION
#   endif
#   define BHO_COMP_IBM_AVAILABLE
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_IBM_NAME "IBM XL C/C++"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_IBM,BHO_COMP_IBM_NAME)

#ifdef BHO_COMP_IBM_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_IBM_EMULATED,BHO_COMP_IBM_NAME)
#endif
