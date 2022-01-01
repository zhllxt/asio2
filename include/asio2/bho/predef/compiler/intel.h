/*
Copyright Rene Rivera 2008-2017
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_INTEL_H
#define BHO_PREDEF_COMPILER_INTEL_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_COMP_INTEL`

http://en.wikipedia.org/wiki/Intel_C%2B%2B[Intel C/{CPP}] compiler.
Version number available as major, minor, and patch.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__INTEL_COMPILER+` | {predef_detection}
| `+__ICL+` | {predef_detection}
| `+__ICC+` | {predef_detection}
| `+__ECC+` | {predef_detection}

| `+__INTEL_COMPILER+` | V.R
| `+__INTEL_COMPILER+` and `+__INTEL_COMPILER_UPDATE+` | V.R.P
|===
*/ // end::reference[]

#define BHO_COMP_INTEL BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__INTEL_COMPILER) || defined(__ICL) || defined(__ICC) || \
    defined(__ECC)
/* tag::reference[]
NOTE: Because of an Intel mistake in the release version numbering when
`__INTEL_COMPILER` is `9999` it is detected as version 12.1.0.
*/ // end::reference[]
#   if !defined(BHO_COMP_INTEL_DETECTION) && defined(__INTEL_COMPILER) && (__INTEL_COMPILER == 9999)
#       define BHO_COMP_INTEL_DETECTION BHO_VERSION_NUMBER(12,1,0)
#   endif
#   if !defined(BHO_COMP_INTEL_DETECTION) && defined(__INTEL_COMPILER) && defined(__INTEL_COMPILER_UPDATE)
#       define BHO_COMP_INTEL_DETECTION BHO_VERSION_NUMBER( \
            BHO_VERSION_NUMBER_MAJOR(BHO_PREDEF_MAKE_10_VVRR(__INTEL_COMPILER)), \
            BHO_VERSION_NUMBER_MINOR(BHO_PREDEF_MAKE_10_VVRR(__INTEL_COMPILER)), \
            __INTEL_COMPILER_UPDATE)
#   endif
#   if !defined(BHO_COMP_INTEL_DETECTION) && defined(__INTEL_COMPILER)
#       define BHO_COMP_INTEL_DETECTION BHO_PREDEF_MAKE_10_VVRR(__INTEL_COMPILER)
#   endif
#   if !defined(BHO_COMP_INTEL_DETECTION)
#       define BHO_COMP_INTEL_DETECTION BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#ifdef BHO_COMP_INTEL_DETECTION
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_INTEL_EMULATED BHO_COMP_INTEL_DETECTION
#   else
#       undef BHO_COMP_INTEL
#       define BHO_COMP_INTEL BHO_COMP_INTEL_DETECTION
#   endif
#   define BHO_COMP_INTEL_AVAILABLE
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_INTEL_NAME "Intel C/C++"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_INTEL,BHO_COMP_INTEL_NAME)

#ifdef BHO_COMP_INTEL_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_INTEL_EMULATED,BHO_COMP_INTEL_NAME)
#endif
