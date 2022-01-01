/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_SUNPRO_H
#define BHO_PREDEF_COMPILER_SUNPRO_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_COMP_SUNPRO`

http://en.wikipedia.org/wiki/Oracle_Solaris_Studio[Oracle Solaris Studio] compiler.
Version number available as major, minor, and patch.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__SUNPRO_CC+` | {predef_detection}
| `+__SUNPRO_C+` | {predef_detection}

| `+__SUNPRO_CC+` | V.R.P
| `+__SUNPRO_C+` | V.R.P
| `+__SUNPRO_CC+` | VV.RR.P
| `+__SUNPRO_C+` | VV.RR.P
|===
*/ // end::reference[]

#define BHO_COMP_SUNPRO BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__SUNPRO_CC) || defined(__SUNPRO_C)
#   if !defined(BHO_COMP_SUNPRO_DETECTION) && defined(__SUNPRO_CC)
#       if (__SUNPRO_CC < 0x5100)
#           define BHO_COMP_SUNPRO_DETECTION BHO_PREDEF_MAKE_0X_VRP(__SUNPRO_CC)
#       else
#           define BHO_COMP_SUNPRO_DETECTION BHO_PREDEF_MAKE_0X_VVRRP(__SUNPRO_CC)
#       endif
#   endif
#   if !defined(BHO_COMP_SUNPRO_DETECTION) && defined(__SUNPRO_C)
#       if (__SUNPRO_C < 0x5100)
#           define BHO_COMP_SUNPRO_DETECTION BHO_PREDEF_MAKE_0X_VRP(__SUNPRO_C)
#       else
#           define BHO_COMP_SUNPRO_DETECTION BHO_PREDEF_MAKE_0X_VVRRP(__SUNPRO_C)
#       endif
#   endif
#   if !defined(BHO_COMP_SUNPRO_DETECTION)
#       define BHO_COMP_SUNPRO_DETECTION BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#ifdef BHO_COMP_SUNPRO_DETECTION
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_SUNPRO_EMULATED BHO_COMP_SUNPRO_DETECTION
#   else
#       undef BHO_COMP_SUNPRO
#       define BHO_COMP_SUNPRO BHO_COMP_SUNPRO_DETECTION
#   endif
#   define BHO_COMP_SUNPRO_AVAILABLE
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_SUNPRO_NAME "Oracle Solaris Studio"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_SUNPRO,BHO_COMP_SUNPRO_NAME)

#ifdef BHO_COMP_SUNPRO_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_SUNPRO_EMULATED,BHO_COMP_SUNPRO_NAME)
#endif
