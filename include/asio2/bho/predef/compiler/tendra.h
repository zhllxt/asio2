/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_TENDRA_H
#define BHO_PREDEF_COMPILER_TENDRA_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_COMP_TENDRA`

http://en.wikipedia.org/wiki/TenDRA_Compiler[TenDRA C/{CPP}] compiler.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__TenDRA__+` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_COMP_TENDRA BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__TenDRA__)
#   define BHO_COMP_TENDRA_DETECTION BHO_VERSION_NUMBER_AVAILABLE
#endif

#ifdef BHO_COMP_TENDRA_DETECTION
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_TENDRA_EMULATED BHO_COMP_TENDRA_DETECTION
#   else
#       undef BHO_COMP_TENDRA
#       define BHO_COMP_TENDRA BHO_COMP_TENDRA_DETECTION
#   endif
#   define BHO_COMP_TENDRA_AVAILABLE
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_TENDRA_NAME "TenDRA C/C++"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_TENDRA,BHO_COMP_TENDRA_NAME)

#ifdef BHO_COMP_TENDRA_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_TENDRA_EMULATED,BHO_COMP_TENDRA_NAME)
#endif
