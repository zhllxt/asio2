/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_SGI_MIPSPRO_H
#define BHO_PREDEF_COMPILER_SGI_MIPSPRO_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_COMP_SGI`

http://en.wikipedia.org/wiki/MIPSpro[SGI MIPSpro] compiler.
Version number available as major, minor, and patch.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__sgi+` | {predef_detection}
| `sgi` | {predef_detection}

| `+_SGI_COMPILER_VERSION+` | V.R.P
| `+_COMPILER_VERSION+` | V.R.P
|===
*/ // end::reference[]

#define BHO_COMP_SGI BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__sgi) || defined(sgi)
#   if !defined(BHO_COMP_SGI_DETECTION) && defined(_SGI_COMPILER_VERSION)
#       define BHO_COMP_SGI_DETECTION BHO_PREDEF_MAKE_10_VRP(_SGI_COMPILER_VERSION)
#   endif
#   if !defined(BHO_COMP_SGI_DETECTION) && defined(_COMPILER_VERSION)
#       define BHO_COMP_SGI_DETECTION BHO_PREDEF_MAKE_10_VRP(_COMPILER_VERSION)
#   endif
#   if !defined(BHO_COMP_SGI_DETECTION)
#       define BHO_COMP_SGI_DETECTION BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#ifdef BHO_COMP_SGI_DETECTION
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_SGI_EMULATED BHO_COMP_SGI_DETECTION
#   else
#       undef BHO_COMP_SGI
#       define BHO_COMP_SGI BHO_COMP_SGI_DETECTION
#   endif
#   define BHO_COMP_SGI_AVAILABLE
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_SGI_NAME "SGI MIPSpro"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_SGI,BHO_COMP_SGI_NAME)

#ifdef BHO_COMP_SGI_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_SGI_EMULATED,BHO_COMP_SGI_NAME)
#endif
