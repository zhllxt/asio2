/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_ARCHITECTURE_SYS370_H
#define BHO_PREDEF_ARCHITECTURE_SYS370_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_ARCH_SYS370`

http://en.wikipedia.org/wiki/System/370[System/370] architecture.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__370__+` | {predef_detection}
| `+__THW_370__+` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_ARCH_SYS370 BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__370__) || defined(__THW_370__)
#   undef BHO_ARCH_SYS370
#   define BHO_ARCH_SYS370 BHO_VERSION_NUMBER_AVAILABLE
#endif

#if BHO_ARCH_SYS370
#   define BHO_ARCH_SYS370_AVAILABLE
#endif

#if BHO_ARCH_SYS370
#   undef BHO_ARCH_WORD_BITS_32
#   define BHO_ARCH_WORD_BITS_32 BHO_VERSION_NUMBER_AVAILABLE
#endif

#define BHO_ARCH_SYS370_NAME "System/370"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_ARCH_SYS370,BHO_ARCH_SYS370_NAME)
