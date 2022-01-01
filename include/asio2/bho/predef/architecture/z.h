/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_ARCHITECTURE_Z_H
#define BHO_PREDEF_ARCHITECTURE_Z_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_ARCH_Z`

http://en.wikipedia.org/wiki/Z/Architecture[z/Architecture] architecture.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__SYSC_ZARCH__+` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_ARCH_Z BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__SYSC_ZARCH__)
#   undef BHO_ARCH_Z
#   define BHO_ARCH_Z BHO_VERSION_NUMBER_AVAILABLE
#endif

#if BHO_ARCH_Z
#   define BHO_ARCH_Z_AVAILABLE
#endif

#if BHO_ARCH_Z
#   undef BHO_ARCH_WORD_BITS_64
#   define BHO_ARCH_WORD_BITS_64 BHO_VERSION_NUMBER_AVAILABLE
#endif

#define BHO_ARCH_Z_NAME "z/Architecture"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_ARCH_Z,BHO_ARCH_Z_NAME)
