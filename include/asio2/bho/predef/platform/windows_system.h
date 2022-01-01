/*
Copyright James E. King III, 2017
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_PLAT_WINDOWS_SYSTEM_H
#define BHO_PREDEF_PLAT_WINDOWS_SYSTEM_H

#include <asio2/bho/predef/make.h>
#include <asio2/bho/predef/os/windows.h>
#include <asio2/bho/predef/platform/windows_uwp.h>
#include <asio2/bho/predef/version_number.h>

/* tag::reference[]
= `BHO_PLAT_WINDOWS_SYSTEM`

https://docs.microsoft.com/en-us/windows/uwp/get-started/universal-application-platform-guide[UWP]
for Windows System development.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `WINAPI_FAMILY == WINAPI_FAMILY_SYSTEM` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_PLAT_WINDOWS_SYSTEM BHO_VERSION_NUMBER_NOT_AVAILABLE

#if BHO_OS_WINDOWS && \
    defined(WINAPI_FAMILY_SYSTEM) && WINAPI_FAMILY == WINAPI_FAMILY_SYSTEM
#   undef BHO_PLAT_WINDOWS_SYSTEM
#   define BHO_PLAT_WINDOWS_SYSTEM BHO_VERSION_NUMBER_AVAILABLE
#endif
 
#if BHO_PLAT_WINDOWS_SYSTEM
#   define BHO_PLAT_WINDOWS_SYSTEM_AVAILABLE
#   include <asio2/bho/predef/detail/platform_detected.h>
#endif

#define BHO_PLAT_WINDOWS_SYSTEM_NAME "Windows Drivers and Tools"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_PLAT_WINDOWS_SYSTEM,BHO_PLAT_WINDOWS_SYSTEM_NAME)
