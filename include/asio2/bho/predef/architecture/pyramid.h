/*
Copyright Rene Rivera 2011-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_ARCHITECTURE_PYRAMID_H
#define BHO_PREDEF_ARCHITECTURE_PYRAMID_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_ARCH_PYRAMID`

Pyramid 9810 architecture.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `pyr` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_ARCH_PYRAMID BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(pyr)
#   undef BHO_ARCH_PYRAMID
#   define BHO_ARCH_PYRAMID BHO_VERSION_NUMBER_AVAILABLE
#endif

#if BHO_ARCH_PYRAMID
#   define BHO_ARCH_PYRAMID_AVAILABLE
#endif

#if BHO_ARCH_PYRAMID
#   undef BHO_ARCH_WORD_BITS_32
#   define BHO_ARCH_WORD_BITS_32 BHO_VERSION_NUMBER_AVAILABLE
#endif

#define BHO_ARCH_PYRAMID_NAME "Pyramid 9810"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_ARCH_PYRAMID,BHO_ARCH_PYRAMID_NAME)
