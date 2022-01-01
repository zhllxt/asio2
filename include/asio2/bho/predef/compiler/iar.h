/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_IAR_H
#define BHO_PREDEF_COMPILER_IAR_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_COMP_IAR`

IAR C/{CPP} compiler.
Version number available as major, minor, and patch.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__IAR_SYSTEMS_ICC__+` | {predef_detection}

| `+__VER__+` | V.R.P
|===
*/ // end::reference[]

#define BHO_COMP_IAR BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__IAR_SYSTEMS_ICC__)
#   define BHO_COMP_IAR_DETECTION BHO_PREDEF_MAKE_10_VVRR(__VER__)
#endif

#ifdef BHO_COMP_IAR_DETECTION
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_IAR_EMULATED BHO_COMP_IAR_DETECTION
#   else
#       undef BHO_COMP_IAR
#       define BHO_COMP_IAR BHO_COMP_IAR_DETECTION
#   endif
#   define BHO_COMP_IAR_AVAILABLE
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_IAR_NAME "IAR C/C++"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_IAR,BHO_COMP_IAR_NAME)

#ifdef BHO_COMP_IAR_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_IAR_EMULATED,BHO_COMP_IAR_NAME)
#endif
