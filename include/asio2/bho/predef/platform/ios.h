/*
Copyright Ruslan Baratov 2017
Copyright Rene Rivera 2017
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_PLAT_IOS_H
#define BHO_PREDEF_PLAT_IOS_H

#include <asio2/bho/predef/os/ios.h> // BHO_OS_IOS
#include <asio2/bho/predef/version_number.h> // BHO_VERSION_NUMBER_NOT_AVAILABLE

/* tag::reference[]
= `BHO_PLAT_IOS_DEVICE`
= `BHO_PLAT_IOS_SIMULATOR`

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `TARGET_IPHONE_SIMULATOR` | {predef_detection}
| `TARGET_OS_SIMULATOR` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_PLAT_IOS_DEVICE BHO_VERSION_NUMBER_NOT_AVAILABLE
#define BHO_PLAT_IOS_SIMULATOR BHO_VERSION_NUMBER_NOT_AVAILABLE

// https://opensource.apple.com/source/CarbonHeaders/CarbonHeaders-18.1/TargetConditionals.h
#if BHO_OS_IOS
#    include <TargetConditionals.h>
#    if defined(TARGET_OS_SIMULATOR) && (TARGET_OS_SIMULATOR == 1)
#        undef BHO_PLAT_IOS_SIMULATOR
#        define BHO_PLAT_IOS_SIMULATOR BHO_VERSION_NUMBER_AVAILABLE
#    elif defined(TARGET_IPHONE_SIMULATOR) && (TARGET_IPHONE_SIMULATOR == 1)
#        undef BHO_PLAT_IOS_SIMULATOR
#        define BHO_PLAT_IOS_SIMULATOR BHO_VERSION_NUMBER_AVAILABLE
#    else
#        undef BHO_PLAT_IOS_DEVICE
#        define BHO_PLAT_IOS_DEVICE BHO_VERSION_NUMBER_AVAILABLE
#    endif
#endif

#if BHO_PLAT_IOS_SIMULATOR
#    define BHO_PLAT_IOS_SIMULATOR_AVAILABLE
#    include <asio2/bho/predef/detail/platform_detected.h>
#endif

#if BHO_PLAT_IOS_DEVICE
#    define BHO_PLAT_IOS_DEVICE_AVAILABLE
#    include <asio2/bho/predef/detail/platform_detected.h>
#endif

#define BHO_PLAT_IOS_SIMULATOR_NAME "iOS Simulator"
#define BHO_PLAT_IOS_DEVICE_NAME "iOS Device"

#endif // BHO_PREDEF_PLAT_IOS_H

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_PLAT_IOS_SIMULATOR,BHO_PLAT_IOS_SIMULATOR_NAME)
BHO_PREDEF_DECLARE_TEST(BHO_PLAT_IOS_DEVICE,BHO_PLAT_IOS_DEVICE_NAME)
