/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_MICROTEC_H
#define BHO_PREDEF_COMPILER_MICROTEC_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_COMP_MRI`

http://www.mentor.com/microtec/[Microtec C/{CPP}] compiler.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+_MRI+` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_COMP_MRI BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(_MRI)
#   define BHO_COMP_MRI_DETECTION BHO_VERSION_NUMBER_AVAILABLE
#endif

#ifdef BHO_COMP_MRI_DETECTION
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_MRI_EMULATED BHO_COMP_MRI_DETECTION
#   else
#       undef BHO_COMP_MRI
#       define BHO_COMP_MRI BHO_COMP_MRI_DETECTION
#   endif
#   define BHO_COMP_MRI_AVAILABLE
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_MRI_NAME "Microtec C/C++"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_MRI,BHO_COMP_MRI_NAME)

#ifdef BHO_COMP_MRI_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_MRI_EMULATED,BHO_COMP_MRI_NAME)
#endif
