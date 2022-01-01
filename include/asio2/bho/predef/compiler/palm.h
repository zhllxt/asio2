/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_PALM_H
#define BHO_PREDEF_COMPILER_PALM_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_COMP_PALM`

Palm C/{CPP} compiler.
Version number available as major, minor, and patch.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+_PACC_VER+` | {predef_detection}

| `+_PACC_VER+` | V.R.P
|===
*/ // end::reference[]

#define BHO_COMP_PALM BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(_PACC_VER)
#   define BHO_COMP_PALM_DETECTION BHO_PREDEF_MAKE_0X_VRRPP000(_PACC_VER)
#endif

#ifdef BHO_COMP_PALM_DETECTION
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_PALM_EMULATED BHO_COMP_PALM_DETECTION
#   else
#       undef BHO_COMP_PALM
#       define BHO_COMP_PALM BHO_COMP_PALM_DETECTION
#   endif
#   define BHO_COMP_PALM_AVAILABLE
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_PALM_NAME "Palm C/C++"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_PALM,BHO_COMP_PALM_NAME)

#ifdef BHO_COMP_PALM_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_PALM_EMULATED,BHO_COMP_PALM_NAME)
#endif
