/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_BORLAND_H
#define BHO_PREDEF_COMPILER_BORLAND_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_COMP_BORLAND`

http://en.wikipedia.org/wiki/C_plus_plus_builder[Borland {CPP}] compiler.
Version number available as major, minor, and patch.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__BORLANDC__+` | {predef_detection}
| `+__CODEGEARC__+` | {predef_detection}

| `+__BORLANDC__+` | V.R.P
| `+__CODEGEARC__+` | V.R.P
|===
*/ // end::reference[]

#define BHO_COMP_BORLAND BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__BORLANDC__) || defined(__CODEGEARC__)
#   if !defined(BHO_COMP_BORLAND_DETECTION) && (defined(__CODEGEARC__))
#       define BHO_COMP_BORLAND_DETECTION BHO_PREDEF_MAKE_0X_VVRP(__CODEGEARC__)
#   endif
#   if !defined(BHO_COMP_BORLAND_DETECTION)
#       define BHO_COMP_BORLAND_DETECTION BHO_PREDEF_MAKE_0X_VVRP(__BORLANDC__)
#   endif
#endif

#ifdef BHO_COMP_BORLAND_DETECTION
#   define BHO_COMP_BORLAND_AVAILABLE
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_BORLAND_EMULATED BHO_COMP_BORLAND_DETECTION
#   else
#       undef BHO_COMP_BORLAND
#       define BHO_COMP_BORLAND BHO_COMP_BORLAND_DETECTION
#   endif
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_BORLAND_NAME "Borland C++"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_BORLAND,BHO_COMP_BORLAND_NAME)

#ifdef BHO_COMP_BORLAND_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_BORLAND_EMULATED,BHO_COMP_BORLAND_NAME)
#endif
