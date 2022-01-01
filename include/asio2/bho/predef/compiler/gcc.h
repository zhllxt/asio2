/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_GCC_H
#define BHO_PREDEF_COMPILER_GCC_H

/* Other compilers that emulate this one need to be detected first. */

#include <asio2/bho/predef/compiler/clang.h>

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_COMP_GNUC`

http://en.wikipedia.org/wiki/GNU_Compiler_Collection[Gnu GCC C/{CPP}] compiler.
Version number available as major, minor, and patch (if available).

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__GNUC__+` | {predef_detection}

| `+__GNUC__+`, `+__GNUC_MINOR__+`, `+__GNUC_PATCHLEVEL__+` | V.R.P
| `+__GNUC__+`, `+__GNUC_MINOR__+` | V.R.0
|===
*/ // end::reference[]

#define BHO_COMP_GNUC BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__GNUC__)
#   if !defined(BHO_COMP_GNUC_DETECTION) && defined(__GNUC_PATCHLEVEL__)
#       define BHO_COMP_GNUC_DETECTION \
            BHO_VERSION_NUMBER(__GNUC__,__GNUC_MINOR__,__GNUC_PATCHLEVEL__)
#   endif
#   if !defined(BHO_COMP_GNUC_DETECTION)
#       define BHO_COMP_GNUC_DETECTION \
            BHO_VERSION_NUMBER(__GNUC__,__GNUC_MINOR__,0)
#   endif
#endif

#ifdef BHO_COMP_GNUC_DETECTION
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_GNUC_EMULATED BHO_COMP_GNUC_DETECTION
#   else
#       undef BHO_COMP_GNUC
#       define BHO_COMP_GNUC BHO_COMP_GNUC_DETECTION
#   endif
#   define BHO_COMP_GNUC_AVAILABLE
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_GNUC_NAME "Gnu GCC C/C++"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_GNUC,BHO_COMP_GNUC_NAME)

#ifdef BHO_COMP_GNUC_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_GNUC_EMULATED,BHO_COMP_GNUC_NAME)
#endif
