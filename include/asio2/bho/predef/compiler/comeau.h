/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_COMEAU_H
#define BHO_PREDEF_COMPILER_COMEAU_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

#define BHO_COMP_COMO BHO_VERSION_NUMBER_NOT_AVAILABLE

/* tag::reference[]
= `BHO_COMP_COMO`

http://en.wikipedia.org/wiki/Comeau_C/C%2B%2B[Comeau {CPP}] compiler.
Version number available as major, minor, and patch.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__COMO__+` | {predef_detection}

| `+__COMO_VERSION__+` | V.R.P
|===
*/ // end::reference[]

#if defined(__COMO__)
#   if !defined(BHO_COMP_COMO_DETECTION) && defined(__COMO_VERSION__)
#       define BHO_COMP_COMO_DETECTION BHO_PREDEF_MAKE_0X_VRP(__COMO_VERSION__)
#   endif
#   if !defined(BHO_COMP_COMO_DETECTION)
#       define BHO_COMP_COMO_DETECTION BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#ifdef BHO_COMP_COMO_DETECTION
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_COMO_EMULATED BHO_COMP_COMO_DETECTION
#   else
#       undef BHO_COMP_COMO
#       define BHO_COMP_COMO BHO_COMP_COMO_DETECTION
#   endif
#   define BHO_COMP_COMO_AVAILABLE
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_COMO_NAME "Comeau C++"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_COMO,BHO_COMP_COMO_NAME)

#ifdef BHO_COMP_COMO_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_COMO_EMULATED,BHO_COMP_COMO_NAME)
#endif
