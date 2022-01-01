/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_METAWARE_H
#define BHO_PREDEF_COMPILER_METAWARE_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_COMP_HIGHC`

MetaWare High C/{CPP} compiler.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__HIGHC__+` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_COMP_HIGHC BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__HIGHC__)
#   define BHO_COMP_HIGHC_DETECTION BHO_VERSION_NUMBER_AVAILABLE
#endif

#ifdef BHO_COMP_HIGHC_DETECTION
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_HIGHC_EMULATED BHO_COMP_HIGHC_DETECTION
#   else
#       undef BHO_COMP_HIGHC
#       define BHO_COMP_HIGHC BHO_COMP_HIGHC_DETECTION
#   endif
#   define BHO_COMP_HIGHC_AVAILABLE
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_HIGHC_NAME "MetaWare High C/C++"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_HIGHC,BHO_COMP_HIGHC_NAME)

#ifdef BHO_COMP_HIGHC_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_HIGHC_EMULATED,BHO_COMP_HIGHC_NAME)
#endif
