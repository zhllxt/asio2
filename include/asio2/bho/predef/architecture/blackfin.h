/*
Copyright Rene Rivera 2013-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_ARCHITECTURE_BLACKFIN_H
#define BHO_PREDEF_ARCHITECTURE_BLACKFIN_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_ARCH_BLACKFIN`

Blackfin Processors from Analog Devices.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__bfin__+` | {predef_detection}
| `+__BFIN__+` | {predef_detection}
| `bfin` | {predef_detection}
| `BFIN` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_ARCH_BLACKFIN BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__bfin__) || defined(__BFIN__) || \
    defined(bfin) || defined(BFIN)
#   undef BHO_ARCH_BLACKFIN
#   define BHO_ARCH_BLACKFIN BHO_VERSION_NUMBER_AVAILABLE
#endif

#if BHO_ARCH_BLACKFIN
#   define BHO_ARCH_BLACKFIN_AVAILABLE
#endif

#if BHO_ARCH_BLACKFIN
#   undef BHO_ARCH_WORD_BITS_16
#   define BHO_ARCH_WORD_BITS_16 BHO_VERSION_NUMBER_AVAILABLE
#endif

#define BHO_ARCH_BLACKFIN_NAME "Blackfin"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_ARCH_BLACKFIN,BHO_ARCH_BLACKFIN_NAME)
