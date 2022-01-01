/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_ARCHITECTURE_IA64_H
#define BHO_PREDEF_ARCHITECTURE_IA64_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_ARCH_IA64`

http://en.wikipedia.org/wiki/Ia64[Intel Itanium 64] architecture.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__ia64__+` | {predef_detection}
| `+_IA64+` | {predef_detection}
| `+__IA64__+` | {predef_detection}
| `+__ia64+` | {predef_detection}
| `+_M_IA64+` | {predef_detection}
| `+__itanium__+` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_ARCH_IA64 BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__ia64__) || defined(_IA64) || \
    defined(__IA64__) || defined(__ia64) || \
    defined(_M_IA64) || defined(__itanium__)
#   undef BHO_ARCH_IA64
#   define BHO_ARCH_IA64 BHO_VERSION_NUMBER_AVAILABLE
#endif

#if BHO_ARCH_IA64
#   define BHO_ARCH_IA64_AVAILABLE
#endif

#if BHO_ARCH_IA64
#   undef BHO_ARCH_WORD_BITS_64
#   define BHO_ARCH_WORD_BITS_64 BHO_VERSION_NUMBER_AVAILABLE
#endif

#define BHO_ARCH_IA64_NAME "Intel Itanium 64"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_ARCH_IA64,BHO_ARCH_IA64_NAME)
