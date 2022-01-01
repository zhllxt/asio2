/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_KAI_H
#define BHO_PREDEF_COMPILER_KAI_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_COMP_KCC`

Kai {CPP} compiler.
Version number available as major, minor, and patch.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__KCC+` | {predef_detection}

| `+__KCC_VERSION+` | V.R.P
|===
*/ // end::reference[]

#define BHO_COMP_KCC BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__KCC)
#   define BHO_COMP_KCC_DETECTION BHO_PREDEF_MAKE_0X_VRPP(__KCC_VERSION)
#endif

#ifdef BHO_COMP_KCC_DETECTION
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_KCC_EMULATED BHO_COMP_KCC_DETECTION
#   else
#       undef BHO_COMP_KCC
#       define BHO_COMP_KCC BHO_COMP_KCC_DETECTION
#   endif
#   define BHO_COMP_KCC_AVAILABLE
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_KCC_NAME "Kai C++"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_KCC,BHO_COMP_KCC_NAME)

#ifdef BHO_COMP_KCC_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_KCC_EMULATED,BHO_COMP_KCC_NAME)
#endif
