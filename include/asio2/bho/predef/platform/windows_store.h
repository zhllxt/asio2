/*
Copyright (c) Microsoft Corporation 2014
Copyright Rene Rivera 2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_PLAT_WINDOWS_STORE_H
#define BHO_PREDEF_PLAT_WINDOWS_STORE_H

#include <asio2/bho/predef/make.h>
#include <asio2/bho/predef/os/windows.h>
#include <asio2/bho/predef/platform/windows_uwp.h>
#include <asio2/bho/predef/version_number.h>

/* tag::reference[]
= `BHO_PLAT_WINDOWS_STORE`

https://docs.microsoft.com/en-us/windows/uwp/get-started/universal-application-platform-guide[UWP]
for Windows Store development.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `WINAPI_FAMILY == WINAPI_FAMILY_PC_APP` | {predef_detection}
| `WINAPI_FAMILY == WINAPI_FAMILY_APP` (deprecated) | {predef_detection}
|===
*/ // end::reference[]

#define BHO_PLAT_WINDOWS_STORE BHO_VERSION_NUMBER_NOT_AVAILABLE

#if BHO_OS_WINDOWS && \
    ((defined(WINAPI_FAMILY_PC_APP) && WINAPI_FAMILY == WINAPI_FAMILY_PC_APP) || \
     (defined(WINAPI_FAMILY_APP)    && WINAPI_FAMILY == WINAPI_FAMILY_APP))
#   undef BHO_PLAT_WINDOWS_STORE
#   define BHO_PLAT_WINDOWS_STORE BHO_VERSION_NUMBER_AVAILABLE
#endif
 
#if BHO_PLAT_WINDOWS_STORE
#   define BHO_PLAT_WINDOWS_STORE_AVAILABLE
#   include <asio2/bho/predef/detail/platform_detected.h>
#endif

#define BHO_PLAT_WINDOWS_STORE_NAME "Windows Store"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_PLAT_WINDOWS_STORE,BHO_PLAT_WINDOWS_STORE_NAME)
