/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_OS_WINDOWS_H
#define BHO_PREDEF_OS_WINDOWS_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_OS_WINDOWS`

http://en.wikipedia.org/wiki/Category:Microsoft_Windows[Microsoft Windows] operating system.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+_WIN32+` | {predef_detection}
| `+_WIN64+` | {predef_detection}
| `+__WIN32__+` | {predef_detection}
| `+__TOS_WIN__+` | {predef_detection}
| `+__WINDOWS__+` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_OS_WINDOWS BHO_VERSION_NUMBER_NOT_AVAILABLE

#if !defined(BHO_PREDEF_DETAIL_OS_DETECTED) && ( \
    defined(_WIN32) || defined(_WIN64) || \
    defined(__WIN32__) || defined(__TOS_WIN__) || \
    defined(__WINDOWS__) \
    )
#   undef BHO_OS_WINDOWS
#   define BHO_OS_WINDOWS BHO_VERSION_NUMBER_AVAILABLE
#endif

#if BHO_OS_WINDOWS
#   define BHO_OS_WINDOWS_AVAILABLE
#   include <asio2/bho/predef/detail/os_detected.h>
#endif

#define BHO_OS_WINDOWS_NAME "Microsoft Windows"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_OS_WINDOWS,BHO_OS_WINDOWS_NAME)
