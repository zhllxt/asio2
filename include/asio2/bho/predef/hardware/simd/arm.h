/*
Copyright Charly Chevalier 2015
Copyright Joel Falcou 2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_HARDWARE_SIMD_ARM_H
#define BHO_PREDEF_HARDWARE_SIMD_ARM_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/hardware/simd/arm/versions.h>

/* tag::reference[]
= `BHO_HW_SIMD_ARM`

The SIMD extension for ARM (*if detected*).
Version number depends on the most recent detected extension.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__ARM_NEON__+` | {predef_detection}
| `+__aarch64__+` | {predef_detection}
| `+_M_ARM+` | {predef_detection}
| `+_M_ARM64+` | {predef_detection}
|===

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__ARM_NEON__+` | BHO_HW_SIMD_ARM_NEON_VERSION
| `+__aarch64__+` | BHO_HW_SIMD_ARM_NEON_VERSION
| `+_M_ARM+` | BHO_HW_SIMD_ARM_NEON_VERSION
| `+_M_ARM64+` | BHO_HW_SIMD_ARM_NEON_VERSION
|===

*/ // end::reference[]

#define BHO_HW_SIMD_ARM BHO_VERSION_NUMBER_NOT_AVAILABLE

#undef BHO_HW_SIMD_ARM
#if !defined(BHO_HW_SIMD_ARM) && (defined(__ARM_NEON__) || defined(__aarch64__) || defined (_M_ARM) || defined (_M_ARM64))
#   define BHO_HW_SIMD_ARM BHO_HW_SIMD_ARM_NEON_VERSION
#endif

#if !defined(BHO_HW_SIMD_ARM)
#   define BHO_HW_SIMD_ARM BHO_VERSION_NUMBER_NOT_AVAILABLE
#else
#   define BHO_HW_SIMD_ARM_AVAILABLE
#endif

#define BHO_HW_SIMD_ARM_NAME "ARM SIMD"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_HW_SIMD_ARM, BHO_HW_SIMD_ARM_NAME)
