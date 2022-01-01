/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_WATCOM_H
#define BHO_PREDEF_COMPILER_WATCOM_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_COMP_WATCOM`

http://en.wikipedia.org/wiki/Watcom[Watcom {CPP}] compiler.
Version number available as major, and minor.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__WATCOMC__+` | {predef_detection}

| `+__WATCOMC__+` | V.R.P
|===
*/ // end::reference[]

#define BHO_COMP_WATCOM BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__WATCOMC__)
#   define BHO_COMP_WATCOM_DETECTION BHO_PREDEF_MAKE_10_VVRR(__WATCOMC__)
#endif

#ifdef BHO_COMP_WATCOM_DETECTION
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_WATCOM_EMULATED BHO_COMP_WATCOM_DETECTION
#   else
#       undef BHO_COMP_WATCOM
#       define BHO_COMP_WATCOM BHO_COMP_WATCOM_DETECTION
#   endif
#   define BHO_COMP_WATCOM_AVAILABLE
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_WATCOM_NAME "Watcom C++"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_WATCOM,BHO_COMP_WATCOM_NAME)

#ifdef BHO_COMP_WATCOM_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_WATCOM_EMULATED,BHO_COMP_WATCOM_NAME)
#endif
