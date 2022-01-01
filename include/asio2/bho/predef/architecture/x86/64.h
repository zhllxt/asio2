/*
Copyright Rene Rivera 2008-2021
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_ARCHITECTURE_X86_64_H
#define BHO_PREDEF_ARCHITECTURE_X86_64_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_ARCH_X86_64`

https://en.wikipedia.org/wiki/X86-64[X86-64] architecture.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__x86_64+` | {predef_detection}
| `+__x86_64__+` | {predef_detection}
| `+__amd64__+` | {predef_detection}
| `+__amd64+` | {predef_detection}
| `+_M_X64+` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_ARCH_X86_64 BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__x86_64) || defined(__x86_64__) || \
    defined(__amd64__) || defined(__amd64) || \
    defined(_M_X64)
#   undef BHO_ARCH_X86_64
#   define BHO_ARCH_X86_64 BHO_VERSION_NUMBER_AVAILABLE
#endif

#if BHO_ARCH_X86_64
#   define BHO_ARCH_X86_64_AVAILABLE
#endif

#if BHO_ARCH_X86_64
#   undef BHO_ARCH_WORD_BITS_64
#   define BHO_ARCH_WORD_BITS_64 BHO_VERSION_NUMBER_AVAILABLE
#endif

#define BHO_ARCH_X86_64_NAME "Intel x86-64"

#include <asio2/bho/predef/architecture/x86.h>

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_ARCH_X86_64,BHO_ARCH_X86_64_NAME)
