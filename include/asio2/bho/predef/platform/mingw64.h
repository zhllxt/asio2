/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_PLAT_MINGW64_H
#define BHO_PREDEF_PLAT_MINGW64_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_PLAT_MINGW64`

https://mingw-w64.org/[MinGW-w64] platform.
Version number available as major, minor, and patch.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__MINGW64__+` | {predef_detection}

| `+__MINGW64_VERSION_MAJOR+`, `+__MINGW64_VERSION_MINOR+` | V.R.0
|===
*/ // end::reference[]

#define BHO_PLAT_MINGW64 BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__MINGW64__)
#   include <_mingw.h>
#   if !defined(BHO_PLAT_MINGW64_DETECTION) && (defined(__MINGW64_VERSION_MAJOR) && defined(__MINGW64_VERSION_MINOR))
#       define BHO_PLAT_MINGW64_DETECTION \
            BHO_VERSION_NUMBER(__MINGW64_VERSION_MAJOR,__MINGW64_VERSION_MINOR,0)
#   endif
#   if !defined(BHO_PLAT_MINGW64_DETECTION)
#       define BHO_PLAT_MINGW64_DETECTION BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#ifdef BHO_PLAT_MINGW64_DETECTION
#   define BHO_PLAT_MINGW64_AVAILABLE
#   if defined(BHO_PREDEF_DETAIL_PLAT_DETECTED)
#       define BHO_PLAT_MINGW64_EMULATED BHO_PLAT_MINGW64_DETECTION
#   else
#       undef BHO_PLAT_MINGW64
#       define BHO_PLAT_MINGW64 BHO_PLAT_MINGW64_DETECTION
#   endif
#   include <asio2/bho/predef/detail/platform_detected.h>
#endif

#define BHO_PLAT_MINGW64_NAME "MinGW-w64"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_PLAT_MINGW64,BHO_PLAT_MINGW64_NAME)

#ifdef BHO_PLAT_MINGW64_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_PLAT_MINGW64_EMULATED,BHO_PLAT_MINGW64_NAME)
#endif
