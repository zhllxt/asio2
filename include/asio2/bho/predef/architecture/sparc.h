/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_ARCHITECTURE_SPARC_H
#define BHO_PREDEF_ARCHITECTURE_SPARC_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_ARCH_SPARC`

http://en.wikipedia.org/wiki/SPARC[SPARC] architecture.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__sparc__+` | {predef_detection}
| `+__sparc+` | {predef_detection}

| `+__sparcv9+` | 9.0.0
| `+__sparc_v9__+` | 9.0.0
| `+__sparcv8+` | 8.0.0
| `+__sparc_v8__+` | 8.0.0
|===
*/ // end::reference[]

#define BHO_ARCH_SPARC BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__sparc__) || defined(__sparc)
#   undef BHO_ARCH_SPARC
#   if !defined(BHO_ARCH_SPARC) && (defined(__sparcv9) || defined(__sparc_v9__))
#       define BHO_ARCH_SPARC BHO_VERSION_NUMBER(9,0,0)
#   endif
#   if !defined(BHO_ARCH_SPARC) && (defined(__sparcv8) || defined(__sparc_v8__))
#       define BHO_ARCH_SPARC BHO_VERSION_NUMBER(8,0,0)
#   endif
#   if !defined(BHO_ARCH_SPARC)
#       define BHO_ARCH_SPARC BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if BHO_ARCH_SPARC
#   define BHO_ARCH_SPARC_AVAILABLE
#endif

#if BHO_ARCH_SPARC
#   if BHO_ARCH_SPARC >= BHO_VERSION_NUMBER(9,0,0)
#       undef BHO_ARCH_WORD_BITS_64
#       define BHO_ARCH_WORD_BITS_64 BHO_VERSION_NUMBER_AVAILABLE
#   else
#       undef BHO_ARCH_WORD_BITS_32
#       define BHO_ARCH_WORD_BITS_32 BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#define BHO_ARCH_SPARC_NAME "SPARC"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_ARCH_SPARC,BHO_ARCH_SPARC_NAME)
