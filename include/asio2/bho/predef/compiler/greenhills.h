/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_GREENHILLS_H
#define BHO_PREDEF_COMPILER_GREENHILLS_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_COMP_GHS`

http://en.wikipedia.org/wiki/Green_Hills_Software[Green Hills C/{CPP}] compiler.
Version number available as major, minor, and patch.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__ghs+` | {predef_detection}
| `+__ghs__+` | {predef_detection}

| `+__GHS_VERSION_NUMBER__+` | V.R.P
| `+__ghs+` | V.R.P
|===
*/ // end::reference[]

#define BHO_COMP_GHS BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__ghs) || defined(__ghs__)
#   if !defined(BHO_COMP_GHS_DETECTION) && defined(__GHS_VERSION_NUMBER__)
#       define BHO_COMP_GHS_DETECTION BHO_PREDEF_MAKE_10_VRP(__GHS_VERSION_NUMBER__)
#   endif
#   if !defined(BHO_COMP_GHS_DETECTION) && defined(__ghs)
#       define BHO_COMP_GHS_DETECTION BHO_PREDEF_MAKE_10_VRP(__ghs)
#   endif
#   if !defined(BHO_COMP_GHS_DETECTION)
#       define BHO_COMP_GHS_DETECTION BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#ifdef BHO_COMP_GHS_DETECTION
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_GHS_EMULATED BHO_COMP_GHS_DETECTION
#   else
#       undef BHO_COMP_GHS
#       define BHO_COMP_GHS BHO_COMP_GHS_DETECTION
#   endif
#   define BHO_COMP_GHS_AVAILABLE
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_GHS_NAME "Green Hills C/C++"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_GHS,BHO_COMP_GHS_NAME)

#ifdef BHO_COMP_GHS_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_GHS_EMULATED,BHO_COMP_GHS_NAME)
#endif
