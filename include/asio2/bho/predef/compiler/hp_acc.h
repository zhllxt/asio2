/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_HP_ACC_H
#define BHO_PREDEF_COMPILER_HP_ACC_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_COMP_HPACC`

HP a{CPP} compiler.
Version number available as major, minor, and patch.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__HP_aCC+` | {predef_detection}

| `+__HP_aCC+` | V.R.P
|===
*/ // end::reference[]

#define BHO_COMP_HPACC BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__HP_aCC)
#   if !defined(BHO_COMP_HPACC_DETECTION) && (__HP_aCC > 1)
#       define BHO_COMP_HPACC_DETECTION BHO_PREDEF_MAKE_10_VVRRPP(__HP_aCC)
#   endif
#   if !defined(BHO_COMP_HPACC_DETECTION)
#       define BHO_COMP_HPACC_DETECTION BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#ifdef BHO_COMP_HPACC_DETECTION
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_HPACC_EMULATED BHO_COMP_HPACC_DETECTION
#   else
#       undef BHO_COMP_HPACC
#       define BHO_COMP_HPACC BHO_COMP_HPACC_DETECTION
#   endif
#   define BHO_COMP_HPACC_AVAILABLE
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_HPACC_NAME "HP aC++"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_HPACC,BHO_COMP_HPACC_NAME)

#ifdef BHO_COMP_HPACC_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_HPACC_EMULATED,BHO_COMP_HPACC_NAME)
#endif
