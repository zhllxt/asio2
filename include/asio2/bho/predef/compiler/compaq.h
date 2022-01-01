/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_COMPAQ_H
#define BHO_PREDEF_COMPILER_COMPAQ_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_COMP_DEC`

http://www.openvms.compaq.com/openvms/brochures/deccplus/[Compaq C/{CPP}] compiler.
Version number available as major, minor, and patch.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__DECCXX+` | {predef_detection}
| `+__DECC+` | {predef_detection}

| `+__DECCXX_VER+` | V.R.P
| `+__DECC_VER+` | V.R.P
|===
*/ // end::reference[]

#define BHO_COMP_DEC BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__DECC) || defined(__DECCXX)
#   if !defined(BHO_COMP_DEC_DETECTION) && defined(__DECCXX_VER)
#       define BHO_COMP_DEC_DETECTION BHO_PREDEF_MAKE_10_VVRR0PP00(__DECCXX_VER)
#   endif
#   if !defined(BHO_COMP_DEC_DETECTION) && defined(__DECC_VER)
#       define BHO_COMP_DEC_DETECTION BHO_PREDEF_MAKE_10_VVRR0PP00(__DECC_VER)
#   endif
#   if !defined(BHO_COMP_DEC_DETECTION)
#       define BHO_COMP_DEC_DETECTION BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#ifdef BHO_COMP_DEC_DETECTION
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_DEC_EMULATED BHO_COMP_DEC_DETECTION
#   else
#       undef BHO_COMP_DEC
#       define BHO_COMP_DEC BHO_COMP_DEC_DETECTION
#   endif
#   define BHO_COMP_DEC_AVAILABLE
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_DEC_NAME "Compaq C/C++"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_DEC,BHO_COMP_DEC_NAME)

#ifdef BHO_COMP_DEC_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_DEC_EMULATED,BHO_COMP_DEC_NAME)
#endif
