/*
Copyright Konstantin Ivlev 2021
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_ARCHITECTURE_E2K_H
#define BHO_PREDEF_ARCHITECTURE_E2K_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_ARCH_E2K`

https://en.wikipedia.org/wiki/Elbrus_2000[E2K] architecture.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__e2k__+` | {predef_detection}

| `+__e2k__+` | V.0.0
|===
*/ // end::reference[]

#define BHO_ARCH_E2K BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__e2k__)
#   undef BHO_ARCH_E2K
#   if !defined(BHO_ARCH_E2K) && defined(__iset__)
#       define BHO_ARCH_E2K BHO_VERSION_NUMBER(__iset__,0,0)
#   endif
#   if !defined(BHO_ARCH_E2K)
#       define BHO_ARCH_E2K BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if BHO_ARCH_E2K
#   define BHO_ARCH_E2K_AVAILABLE
#endif

#if BHO_ARCH_E2K
#   define BHO_ARCH_WORD_BITS_64 BHO_VERSION_NUMBER_AVAILABLE
#endif

#define BHO_ARCH_E2K_NAME "E2K"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_ARCH_E2K,BHO_ARCH_E2K_NAME)
