/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_DIGNUS_H
#define BHO_PREDEF_COMPILER_DIGNUS_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_COMP_SYSC`

http://www.dignus.com/dcxx/[Dignus Systems/{CPP}] compiler.
Version number available as major, minor, and patch.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__SYSC__+` | {predef_detection}

| `+__SYSC_VER__+` | V.R.P
|===
*/ // end::reference[]

#define BHO_COMP_SYSC BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__SYSC__)
#   define BHO_COMP_SYSC_DETECTION BHO_PREDEF_MAKE_10_VRRPP(__SYSC_VER__)
#endif

#ifdef BHO_COMP_SYSC_DETECTION
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_SYSC_EMULATED BHO_COMP_SYSC_DETECTION
#   else
#       undef BHO_COMP_SYSC
#       define BHO_COMP_SYSC BHO_COMP_SYSC_DETECTION
#   endif
#   define BHO_COMP_SYSC_AVAILABLE
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_SYSC_NAME "Dignus Systems/C++"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_SYSC,BHO_COMP_SYSC_NAME)

#ifdef BHO_COMP_SYSC_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_SYSC_EMULATED,BHO_COMP_SYSC_NAME)
#endif
