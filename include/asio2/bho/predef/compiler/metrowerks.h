/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_METROWERKS_H
#define BHO_PREDEF_COMPILER_METROWERKS_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_COMP_MWERKS`

http://en.wikipedia.org/wiki/CodeWarrior[Metrowerks CodeWarrior] compiler.
Version number available as major, minor, and patch.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__MWERKS__+` | {predef_detection}
| `+__CWCC__+` | {predef_detection}

| `+__CWCC__+` | V.R.P
| `+__MWERKS__+` | V.R.P >= 4.2.0
| `+__MWERKS__+` | 9.R.0
| `+__MWERKS__+` | 8.R.0
|===
*/ // end::reference[]

#define BHO_COMP_MWERKS BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__MWERKS__) || defined(__CWCC__)
#   if !defined(BHO_COMP_MWERKS_DETECTION) && defined(__CWCC__)
#       define BHO_COMP_MWERKS_DETECTION BHO_PREDEF_MAKE_0X_VRPP(__CWCC__)
#   endif
#   if !defined(BHO_COMP_MWERKS_DETECTION) && (__MWERKS__ >= 0x4200)
#       define BHO_COMP_MWERKS_DETECTION BHO_PREDEF_MAKE_0X_VRPP(__MWERKS__)
#   endif
#   if !defined(BHO_COMP_MWERKS_DETECTION) && (__MWERKS__ >= 0x3204) // note the "skip": 04->9.3
#       define BHO_COMP_MWERKS_DETECTION BHO_VERSION_NUMBER(9,(__MWERKS__)%100-1,0)
#   endif
#   if !defined(BHO_COMP_MWERKS_DETECTION) && (__MWERKS__ >= 0x3200)
#       define BHO_COMP_MWERKS_DETECTION BHO_VERSION_NUMBER(9,(__MWERKS__)%100,0)
#   endif
#   if !defined(BHO_COMP_MWERKS_DETECTION) && (__MWERKS__ >= 0x3000)
#       define BHO_COMP_MWERKS_DETECTION BHO_VERSION_NUMBER(8,(__MWERKS__)%100,0)
#   endif
#   if !defined(BHO_COMP_MWERKS_DETECTION)
#       define BHO_COMP_MWERKS_DETECTION BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#ifdef BHO_COMP_MWERKS_DETECTION
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_MWERKS_EMULATED BHO_COMP_MWERKS_DETECTION
#   else
#       undef BHO_COMP_MWERKS
#       define BHO_COMP_MWERKS BHO_COMP_MWERKS_DETECTION
#   endif
#   define BHO_COMP_MWERKS_AVAILABLE
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_MWERKS_NAME "Metrowerks CodeWarrior"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_MWERKS,BHO_COMP_MWERKS_NAME)

#ifdef BHO_COMP_MWERKS_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_MWERKS_EMULATED,BHO_COMP_MWERKS_NAME)
#endif
