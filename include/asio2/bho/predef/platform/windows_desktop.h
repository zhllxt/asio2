/*
Copyright (c) Microsoft Corporation 2014
Copyright Rene Rivera 2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_PLAT_WINDOWS_DESKTOP_H
#define BHO_PREDEF_PLAT_WINDOWS_DESKTOP_H

#include <asio2/bho/predef/make.h>
#include <asio2/bho/predef/os/windows.h>
#include <asio2/bho/predef/platform/windows_uwp.h>
#include <asio2/bho/predef/version_number.h>

/* tag::reference[]
= `BHO_PLAT_WINDOWS_DESKTOP`

https://docs.microsoft.com/en-us/windows/uwp/get-started/universal-application-platform-guide[UWP]
for Windows Desktop development.  Also available if the Platform SDK is too
old to support UWP.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP` | {predef_detection}
| `!BHO_PLAT_WINDOWS_UWP` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_PLAT_WINDOWS_DESKTOP BHO_VERSION_NUMBER_NOT_AVAILABLE

#if BHO_OS_WINDOWS && \
    ((defined(WINAPI_FAMILY_DESKTOP_APP) && WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP) || \
     !BHO_PLAT_WINDOWS_UWP)
#   undef BHO_PLAT_WINDOWS_DESKTOP
#   define BHO_PLAT_WINDOWS_DESKTOP BHO_VERSION_NUMBER_AVAILABLE
#endif
 
#if BHO_PLAT_WINDOWS_DESKTOP
#   define BHO_PLAT_WINDOWS_DESKTOP_AVAILABLE
#   include <asio2/bho/predef/detail/platform_detected.h>
#endif

#define BHO_PLAT_WINDOWS_DESKTOP_NAME "Windows Desktop"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_PLAT_WINDOWS_DESKTOP,BHO_PLAT_WINDOWS_DESKTOP_NAME)
