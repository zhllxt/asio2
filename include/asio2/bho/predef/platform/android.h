/*
Copyright Rene Rivera 2015-2019
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_PLAT_ANDROID_H
#define BHO_PREDEF_PLAT_ANDROID_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_PLAT_ANDROID`

http://en.wikipedia.org/wiki/Android_%28operating_system%29[Android] platform.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__ANDROID__+` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_PLAT_ANDROID BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__ANDROID__)
#   undef BHO_PLAT_ANDROID
#   define BHO_PLAT_ANDROID BHO_VERSION_NUMBER_AVAILABLE
#endif

#if BHO_PLAT_ANDROID
#   define BHO_PLAT_ANDROID_AVAILABLE
#   include <asio2/bho/predef/detail/platform_detected.h>
#endif

#define BHO_PLAT_ANDROID_NAME "Android"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_PLAT_ANDROID,BHO_PLAT_ANDROID_NAME)
