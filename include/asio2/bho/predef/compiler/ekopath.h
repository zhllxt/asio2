/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_EKOPATH_H
#define BHO_PREDEF_COMPILER_EKOPATH_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_COMP_PATH`

http://en.wikipedia.org/wiki/PathScale[EKOpath] compiler.
Version number available as major, minor, and patch.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__PATHCC__+` | {predef_detection}

| `+__PATHCC__+`, `+__PATHCC_MINOR__+`, `+__PATHCC_PATCHLEVEL__+` | V.R.P
|===
*/ // end::reference[]

#define BHO_COMP_PATH BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__PATHCC__)
#   define BHO_COMP_PATH_DETECTION \
        BHO_VERSION_NUMBER(__PATHCC__,__PATHCC_MINOR__,__PATHCC_PATCHLEVEL__)
#endif

#ifdef BHO_COMP_PATH_DETECTION
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_PATH_EMULATED BHO_COMP_PATH_DETECTION
#   else
#       undef BHO_COMP_PATH
#       define BHO_COMP_PATH BHO_COMP_PATH_DETECTION
#   endif
#   define BHO_COMP_PATH_AVAILABLE
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_PATH_NAME "EKOpath"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_PATH,BHO_COMP_PATH_NAME)

#ifdef BHO_COMP_PATH_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_PATH_EMULATED,BHO_COMP_PATH_NAME)
#endif
