/*
Copyright Franz Detro 2014
Copyright Rene Rivera 2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_OS_IOS_H
#define BHO_PREDEF_OS_IOS_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_OS_IOS`

http://en.wikipedia.org/wiki/iOS[iOS] operating system.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__APPLE__+` | {predef_detection}
| `+__MACH__+` | {predef_detection}
| `+__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__+` | {predef_detection}

| `+__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__+` | +__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__+*1000
|===
*/ // end::reference[]

#define BHO_OS_IOS BHO_VERSION_NUMBER_NOT_AVAILABLE

#if !defined(BHO_PREDEF_DETAIL_OS_DETECTED) && ( \
    defined(__APPLE__) && defined(__MACH__) && \
    defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__) \
    )
#   undef BHO_OS_IOS
#   define BHO_OS_IOS (__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__*1000)
#endif

#if BHO_OS_IOS
#   define BHO_OS_IOS_AVAILABLE
#   include <asio2/bho/predef/detail/os_detected.h>
#endif

#define BHO_OS_IOS_NAME "iOS"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_OS_IOS,BHO_OS_IOS_NAME)
