/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_CLANG_H
#define BHO_PREDEF_COMPILER_CLANG_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_COMP_CLANG`

http://en.wikipedia.org/wiki/Clang[Clang] compiler.
Version number available as major, minor, and patch.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__clang__+` | {predef_detection}

| `+__clang_major__+`, `+__clang_minor__+`, `+__clang_patchlevel__+` | V.R.P
|===
*/ // end::reference[]

#define BHO_COMP_CLANG BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__clang__)
#   define BHO_COMP_CLANG_DETECTION BHO_VERSION_NUMBER(__clang_major__,__clang_minor__,__clang_patchlevel__)
#endif

#ifdef BHO_COMP_CLANG_DETECTION
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_CLANG_EMULATED BHO_COMP_CLANG_DETECTION
#   else
#       undef BHO_COMP_CLANG
#       define BHO_COMP_CLANG BHO_COMP_CLANG_DETECTION
#   endif
#   define BHO_COMP_CLANG_AVAILABLE
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_CLANG_NAME "Clang"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_CLANG,BHO_COMP_CLANG_NAME)

#ifdef BHO_COMP_CLANG_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_CLANG_EMULATED,BHO_COMP_CLANG_NAME)
#endif
