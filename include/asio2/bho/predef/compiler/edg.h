/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_EDG_H
#define BHO_PREDEF_COMPILER_EDG_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_COMP_EDG`

http://en.wikipedia.org/wiki/Edison_Design_Group[EDG {CPP} Frontend] compiler.
Version number available as major, minor, and patch.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__EDG__+` | {predef_detection}

| `+__EDG_VERSION__+` | V.R.0
|===
*/ // end::reference[]

#define BHO_COMP_EDG BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__EDG__)
#   define BHO_COMP_EDG_DETECTION BHO_PREDEF_MAKE_10_VRR(__EDG_VERSION__)
#endif

#ifdef BHO_COMP_EDG_DETECTION
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_EDG_EMULATED BHO_COMP_EDG_DETECTION
#   else
#       undef BHO_COMP_EDG
#       define BHO_COMP_EDG BHO_COMP_EDG_DETECTION
#   endif
#   define BHO_COMP_EDG_AVAILABLE
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_EDG_NAME "EDG C++ Frontend"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_EDG,BHO_COMP_EDG_NAME)

#ifdef BHO_COMP_EDG_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_EDG_EMULATED,BHO_COMP_EDG_NAME)
#endif
