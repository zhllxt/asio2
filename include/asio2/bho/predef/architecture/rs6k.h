/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_ARCHITECTURE_RS6K_H
#define BHO_PREDEF_ARCHITECTURE_RS6K_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_ARCH_RS6000`

http://en.wikipedia.org/wiki/RS/6000[RS/6000] architecture.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__THW_RS6000+` | {predef_detection}
| `+_IBMR2+` | {predef_detection}
| `+_POWER+` | {predef_detection}
| `+_ARCH_PWR+` | {predef_detection}
| `+_ARCH_PWR2+` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_ARCH_RS6000 BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__THW_RS6000) || defined(_IBMR2) || \
    defined(_POWER) || defined(_ARCH_PWR) || \
    defined(_ARCH_PWR2)
#   undef BHO_ARCH_RS6000
#   define BHO_ARCH_RS6000 BHO_VERSION_NUMBER_AVAILABLE
#endif

#if BHO_ARCH_RS6000
#   define BHO_ARCH_RS6000_AVAILABLE
#endif

#if BHO_ARCH_RS6000
#   undef BHO_ARCH_WORD_BITS_32
#   define BHO_ARCH_WORD_BITS_32 BHO_VERSION_NUMBER_AVAILABLE
#endif

#define BHO_ARCH_RS6000_NAME "RS/6000"

#define BHO_ARCH_PWR BHO_ARCH_RS6000

#if BHO_ARCH_PWR
#   define BHO_ARCH_PWR_AVAILABLE
#endif

#if BHO_ARCH_PWR
#   undef BHO_ARCH_WORD_BITS_32
#   define BHO_ARCH_WORD_BITS_32 BHO_VERSION_NUMBER_AVAILABLE
#endif

#define BHO_ARCH_PWR_NAME BHO_ARCH_RS6000_NAME

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_ARCH_RS6000,BHO_ARCH_RS6000_NAME)
