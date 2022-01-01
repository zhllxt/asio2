/*
Copyright (c) Microsoft Corporation 2014
Copyright Rene Rivera 2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_PLAT_WINDOWS_PHONE_H
#define BHO_PREDEF_PLAT_WINDOWS_PHONE_H

#include <asio2/bho/predef/make.h>
#include <asio2/bho/predef/os/windows.h>
#include <asio2/bho/predef/platform/windows_uwp.h>
#include <asio2/bho/predef/version_number.h>

/* tag::reference[]
= `BHO_PLAT_WINDOWS_PHONE`

https://docs.microsoft.com/en-us/windows/uwp/get-started/universal-application-platform-guide[UWP]
for Windows Phone development.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_PLAT_WINDOWS_PHONE BHO_VERSION_NUMBER_NOT_AVAILABLE

#if BHO_OS_WINDOWS && \
    defined(WINAPI_FAMILY_PHONE_APP) && WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
#   undef BHO_PLAT_WINDOWS_PHONE
#   define BHO_PLAT_WINDOWS_PHONE BHO_VERSION_NUMBER_AVAILABLE
#endif
 
#if BHO_PLAT_WINDOWS_PHONE
#   define BHO_PLAT_WINDOWS_PHONE_AVAILABLE
#   include <asio2/bho/predef/detail/platform_detected.h>
#endif

#define BHO_PLAT_WINDOWS_PHONE_NAME "Windows Phone"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_PLAT_WINDOWS_PHONE,BHO_PLAT_WINDOWS_PHONE_NAME)
