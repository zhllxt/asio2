/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_VISUALC_H
#define BHO_PREDEF_COMPILER_VISUALC_H

/* Other compilers that emulate this one need to be detected first. */

#include <asio2/bho/predef/compiler/clang.h>

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_COMP_MSVC`

http://en.wikipedia.org/wiki/Visual_studio[Microsoft Visual C/{CPP}] compiler.
Version number available as major, minor, and patch.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+_MSC_VER+` | {predef_detection}

| `+_MSC_FULL_VER+` | V.R.P
| `+_MSC_VER+` | V.R.0
|===

NOTE: Release of Visual Studio after 2015 will no longer be identified
by Boost Predef as the marketing version number. Instead we use the
compiler version number directly, i.e. the _MSC_VER number.
*/ // end::reference[]

#define BHO_COMP_MSVC BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(_MSC_VER)
#   if !defined (_MSC_FULL_VER)
#       define BHO_COMP_MSVC_BUILD 0
#   else
        /* how many digits does the build number have? */
#       if _MSC_FULL_VER / 10000 == _MSC_VER
            /* four digits */
#           define BHO_COMP_MSVC_BUILD (_MSC_FULL_VER % 10000)
#       elif _MSC_FULL_VER / 100000 == _MSC_VER
            /* five digits */
#           define BHO_COMP_MSVC_BUILD (_MSC_FULL_VER % 100000)
#       else
#           error "Cannot determine build number from _MSC_FULL_VER"
#       endif
#   endif
    /*
    VS2014 was skipped in the release sequence for MS. Which
    means that the compiler and VS product versions are no longer
    in sync. Hence we need to use different formulas for
    mapping from MSC version to VS product version.

    VS2017 is a total nightmare when it comes to version numbers.
    Hence to avoid arguments relating to that both present and
    future.. Any version after VS2015 will use solely the compiler
    version, i.e. cl.exe, as the version number here.
    */
#   if (_MSC_VER > 1900)
#       define BHO_COMP_MSVC_DETECTION BHO_VERSION_NUMBER(\
            _MSC_VER/100,\
            _MSC_VER%100,\
            BHO_COMP_MSVC_BUILD)
#   elif (_MSC_VER >= 1900)
#       define BHO_COMP_MSVC_DETECTION BHO_VERSION_NUMBER(\
            _MSC_VER/100-5,\
            _MSC_VER%100,\
            BHO_COMP_MSVC_BUILD)
#   else
#       define BHO_COMP_MSVC_DETECTION BHO_VERSION_NUMBER(\
            _MSC_VER/100-6,\
            _MSC_VER%100,\
            BHO_COMP_MSVC_BUILD)
#   endif
#endif

#ifdef BHO_COMP_MSVC_DETECTION
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_MSVC_EMULATED BHO_COMP_MSVC_DETECTION
#   else
#       undef BHO_COMP_MSVC
#       define BHO_COMP_MSVC BHO_COMP_MSVC_DETECTION
#   endif
#   define BHO_COMP_MSVC_AVAILABLE
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_MSVC_NAME "Microsoft Visual C/C++"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_MSVC,BHO_COMP_MSVC_NAME)

#ifdef BHO_COMP_MSVC_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_MSVC_EMULATED,BHO_COMP_MSVC_NAME)
#endif
