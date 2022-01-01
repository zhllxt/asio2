/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_ARCHITECTURE_SYS390_H
#define BHO_PREDEF_ARCHITECTURE_SYS390_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_ARCH_SYS390`

http://en.wikipedia.org/wiki/System/390[System/390] architecture.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__s390__+` | {predef_detection}
| `+__s390x__+` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_ARCH_SYS390 BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__s390__) || defined(__s390x__)
#   undef BHO_ARCH_SYS390
#   define BHO_ARCH_SYS390 BHO_VERSION_NUMBER_AVAILABLE
#endif

#if BHO_ARCH_SYS390
#   define BHO_ARCH_SYS390_AVAILABLE
#endif

#if BHO_ARCH_SYS390
#   undef BHO_ARCH_WORD_BITS_32
#   define BHO_ARCH_WORD_BITS_32 BHO_VERSION_NUMBER_AVAILABLE
#endif

#define BHO_ARCH_SYS390_NAME "System/390"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_ARCH_SYS390,BHO_ARCH_SYS390_NAME)
