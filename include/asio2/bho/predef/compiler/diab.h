/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_DIAB_H
#define BHO_PREDEF_COMPILER_DIAB_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_COMP_DIAB`

http://www.windriver.com/products/development_suite/wind_river_compiler/[Diab C/{CPP}] compiler.
Version number available as major, minor, and patch.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__DCC__+` | {predef_detection}

| `+__VERSION_NUMBER__+` | V.R.P
|===
*/ // end::reference[]

#define BHO_COMP_DIAB BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__DCC__)
#   define BHO_COMP_DIAB_DETECTION BHO_PREDEF_MAKE_10_VRPP(__VERSION_NUMBER__)
#endif

#ifdef BHO_COMP_DIAB_DETECTION
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_DIAB_EMULATED BHO_COMP_DIAB_DETECTION
#   else
#       undef BHO_COMP_DIAB
#       define BHO_COMP_DIAB BHO_COMP_DIAB_DETECTION
#   endif
#   define BHO_COMP_DIAB_AVAILABLE
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_DIAB_NAME "Diab C/C++"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_DIAB,BHO_COMP_DIAB_NAME)

#ifdef BHO_COMP_DIAB_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_DIAB_EMULATED,BHO_COMP_DIAB_NAME)
#endif
