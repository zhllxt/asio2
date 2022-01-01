/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_DIGITALMARS_H
#define BHO_PREDEF_COMPILER_DIGITALMARS_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_COMP_DMC`

http://en.wikipedia.org/wiki/Digital_Mars[Digital Mars] compiler.
Version number available as major, minor, and patch.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__DMC__+` | {predef_detection}

| `+__DMC__+` | V.R.P
|===
*/ // end::reference[]

#define BHO_COMP_DMC BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__DMC__)
#   define BHO_COMP_DMC_DETECTION BHO_PREDEF_MAKE_0X_VRP(__DMC__)
#endif

#ifdef BHO_COMP_DMC_DETECTION
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_DMC_EMULATED BHO_COMP_DMC_DETECTION
#   else
#       undef BHO_COMP_DMC
#       define BHO_COMP_DMC BHO_COMP_DMC_DETECTION
#   endif
#   define BHO_COMP_DMC_AVAILABLE
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_DMC_NAME "Digital Mars"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_DMC,BHO_COMP_DMC_NAME)

#ifdef BHO_COMP_DMC_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_DMC_EMULATED,BHO_COMP_DMC_NAME)
#endif
