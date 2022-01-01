/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_MPW_H
#define BHO_PREDEF_COMPILER_MPW_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_COMP_MPW`

http://en.wikipedia.org/wiki/Macintosh_Programmer%27s_Workshop[MPW {CPP}] compiler.
Version number available as major, and minor.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__MRC__+` | {predef_detection}
| `MPW_C` | {predef_detection}
| `MPW_CPLUS` | {predef_detection}

| `+__MRC__+` | V.R.0
|===
*/ // end::reference[]

#define BHO_COMP_MPW BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__MRC__) || defined(MPW_C) || defined(MPW_CPLUS)
#   if !defined(BHO_COMP_MPW_DETECTION) && defined(__MRC__)
#       define BHO_COMP_MPW_DETECTION BHO_PREDEF_MAKE_0X_VVRR(__MRC__)
#   endif
#   if !defined(BHO_COMP_MPW_DETECTION)
#       define BHO_COMP_MPW_DETECTION BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#ifdef BHO_COMP_MPW_DETECTION
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_MPW_EMULATED BHO_COMP_MPW_DETECTION
#   else
#       undef BHO_COMP_MPW
#       define BHO_COMP_MPW BHO_COMP_MPW_DETECTION
#   endif
#   define BHO_COMP_MPW_AVAILABLE
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_MPW_NAME "MPW C++"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_MPW,BHO_COMP_MPW_NAME)

#ifdef BHO_COMP_MPW_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_MPW_EMULATED,BHO_COMP_MPW_NAME)
#endif
