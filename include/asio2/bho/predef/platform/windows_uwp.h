/*
Copyright James E. King III, 2017
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_PLAT_WINDOWS_UWP_H
#define BHO_PREDEF_PLAT_WINDOWS_UWP_H

#include <asio2/bho/predef/make.h>
#include <asio2/bho/predef/os/windows.h>
#include <asio2/bho/predef/version_number.h>

/* tag::reference[]
= `BHO_PLAT_WINDOWS_UWP`

http://docs.microsoft.com/windows/uwp/[Universal Windows Platform]
is available if the current development environment is capable of targeting 
UWP development.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__MINGW64_VERSION_MAJOR+` from `+_mingw.h+` | `>= 3`
| `VER_PRODUCTBUILD` from `ntverp.h` | `>= 9200`
|===
*/ // end::reference[]

#define BHO_PLAT_WINDOWS_UWP BHO_VERSION_NUMBER_NOT_AVAILABLE
#define BHO_PLAT_WINDOWS_SDK_VERSION BHO_VERSION_NUMBER_NOT_AVAILABLE

#if BHO_OS_WINDOWS
//  MinGW (32-bit), WinCE, and wineg++ don't have a ntverp.h header
#if !defined(__MINGW32__) && !defined(_WIN32_WCE) && !defined(__WINE__)
#   include <ntverp.h>
#   undef BHO_PLAT_WINDOWS_SDK_VERSION
#   define BHO_PLAT_WINDOWS_SDK_VERSION BHO_VERSION_NUMBER(0, 0, VER_PRODUCTBUILD)
#endif

// 9200 is Windows SDK 8.0 from ntverp.h which introduced family support
#if ((BHO_PLAT_WINDOWS_SDK_VERSION >= BHO_VERSION_NUMBER(0, 0, 9200)) || \
     (defined(__MINGW64__) && __MINGW64_VERSION_MAJOR >= 3))
#   undef BHO_PLAT_WINDOWS_UWP
#   define BHO_PLAT_WINDOWS_UWP BHO_VERSION_NUMBER_AVAILABLE
#endif
#endif

#if BHO_PLAT_WINDOWS_UWP
#   define BHO_PLAT_WINDOWS_UWP_AVAILABLE
#   include <asio2/bho/predef/detail/platform_detected.h>
#   include <winapifamily.h>    // Windows SDK
#endif

#define BHO_PLAT_WINDOWS_UWP_NAME "Universal Windows Platform"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_PLAT_WINDOWS_UWP, BHO_PLAT_WINDOWS_UWP_NAME)
