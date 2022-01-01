/*
Copyright Charly Chevalier 2015
Copyright Joel Falcou 2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_HARDWARE_SIMD_PPC_H
#define BHO_PREDEF_HARDWARE_SIMD_PPC_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/hardware/simd/ppc/versions.h>

/* tag::reference[]
= `BHO_HW_SIMD_PPC`

The SIMD extension for PowerPC (*if detected*).
Version number depends on the most recent detected extension.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__VECTOR4DOUBLE__+` | {predef_detection}

| `+__ALTIVEC__+` | {predef_detection}
| `+__VEC__+` | {predef_detection}

| `+__VSX__+` | {predef_detection}
|===

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__VECTOR4DOUBLE__+` | BHO_HW_SIMD_PPC_QPX_VERSION

| `+__ALTIVEC__+` | BHO_HW_SIMD_PPC_VMX_VERSION
| `+__VEC__+` | BHO_HW_SIMD_PPC_VMX_VERSION

| `+__VSX__+` | BHO_HW_SIMD_PPC_VSX_VERSION
|===

*/ // end::reference[]

#define BHO_HW_SIMD_PPC BHO_VERSION_NUMBER_NOT_AVAILABLE

#undef BHO_HW_SIMD_PPC
#if !defined(BHO_HW_SIMD_PPC) && defined(__VECTOR4DOUBLE__)
#   define BHO_HW_SIMD_PPC BHO_HW_SIMD_PPC_QPX_VERSION
#endif
#if !defined(BHO_HW_SIMD_PPC) && defined(__VSX__)
#   define BHO_HW_SIMD_PPC BHO_HW_SIMD_PPC_VSX_VERSION
#endif
#if !defined(BHO_HW_SIMD_PPC) && (defined(__ALTIVEC__) || defined(__VEC__))
#   define BHO_HW_SIMD_PPC BHO_HW_SIMD_PPC_VMX_VERSION
#endif

#if !defined(BHO_HW_SIMD_PPC)
#   define BHO_HW_SIMD_PPC BHO_VERSION_NUMBER_NOT_AVAILABLE
#else
#   define BHO_HW_SIMD_PPC_AVAILABLE
#endif

#define BHO_HW_SIMD_PPC_NAME "PPC SIMD"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_HW_SIMD_PPC, BHO_HW_SIMD_PPC_NAME)
