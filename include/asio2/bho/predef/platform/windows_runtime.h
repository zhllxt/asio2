/*
Copyright (c) Microsoft Corporation 2014
Copyright Rene Rivera 2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_PLAT_WINDOWS_RUNTIME_H
#define BHO_PREDEF_PLAT_WINDOWS_RUNTIME_H

#include <asio2/bho/predef/make.h>
#include <asio2/bho/predef/os/windows.h>
#include <asio2/bho/predef/platform/windows_phone.h>
#include <asio2/bho/predef/platform/windows_store.h>
#include <asio2/bho/predef/version_number.h>

/* tag::reference[]
= `BHO_PLAT_WINDOWS_RUNTIME`

Deprecated.

https://docs.microsoft.com/en-us/windows/uwp/get-started/universal-application-platform-guide[UWP]
for Windows Phone or Store development.  This does not align to the existing development model for
UWP and is deprecated.  Use one of the other `BHO_PLAT_WINDOWS_*`definitions instead.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `BHO_PLAT_WINDOWS_PHONE` | {predef_detection}
| `BHO_PLAT_WINDOWS_STORE` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_PLAT_WINDOWS_RUNTIME BHO_VERSION_NUMBER_NOT_AVAILABLE

#if BHO_OS_WINDOWS && \
    (BHO_PLAT_WINDOWS_STORE || BHO_PLAT_WINDOWS_PHONE)
#   undef BHO_PLAT_WINDOWS_RUNTIME
#   define BHO_PLAT_WINDOWS_RUNTIME BHO_VERSION_NUMBER_AVAILABLE
#endif
 
#if BHO_PLAT_WINDOWS_RUNTIME
#   define BHO_PLAT_WINDOWS_RUNTIME_AVAILABLE
#   include <asio2/bho/predef/detail/platform_detected.h>
#endif

#define BHO_PLAT_WINDOWS_RUNTIME_NAME "Windows Runtime"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_PLAT_WINDOWS_RUNTIME,BHO_PLAT_WINDOWS_RUNTIME_NAME)
