/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_ARCHITECTURE_SUPERH_H
#define BHO_PREDEF_ARCHITECTURE_SUPERH_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_ARCH_SH`

http://en.wikipedia.org/wiki/SuperH[SuperH] architecture:
If available versions [1-5] are specifically detected.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__sh__+` | {predef_detection}

| `+__SH5__+` | 5.0.0
| `+__SH4__+` | 4.0.0
| `+__sh3__+` | 3.0.0
| `+__SH3__+` | 3.0.0
| `+__sh2__+` | 2.0.0
| `+__sh1__+` | 1.0.0
|===
*/ // end::reference[]

#define BHO_ARCH_SH BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__sh__)
#   undef BHO_ARCH_SH
#   if !defined(BHO_ARCH_SH) && (defined(__SH5__))
#       define BHO_ARCH_SH BHO_VERSION_NUMBER(5,0,0)
#   endif
#   if !defined(BHO_ARCH_SH) && (defined(__SH4__))
#       define BHO_ARCH_SH BHO_VERSION_NUMBER(4,0,0)
#   endif
#   if !defined(BHO_ARCH_SH) && (defined(__sh3__) || defined(__SH3__))
#       define BHO_ARCH_SH BHO_VERSION_NUMBER(3,0,0)
#   endif
#   if !defined(BHO_ARCH_SH) && (defined(__sh2__))
#       define BHO_ARCH_SH BHO_VERSION_NUMBER(2,0,0)
#   endif
#   if !defined(BHO_ARCH_SH) && (defined(__sh1__))
#       define BHO_ARCH_SH BHO_VERSION_NUMBER(1,0,0)
#   endif
#   if !defined(BHO_ARCH_SH)
#       define BHO_ARCH_SH BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if BHO_ARCH_SH
#   define BHO_ARCH_SH_AVAILABLE
#endif

#if BHO_ARCH_SH
#   if BHO_ARCH_SH >= BHO_VERSION_NUMBER(5,0,0)
#       undef BHO_ARCH_WORD_BITS_64
#       define BHO_ARCH_WORD_BITS_64 BHO_VERSION_NUMBER_AVAILABLE
#   elif BHO_ARCH_SH >= BHO_VERSION_NUMBER(3,0,0)
#       undef BHO_ARCH_WORD_BITS_32
#       define BHO_ARCH_WORD_BITS_32 BHO_VERSION_NUMBER_AVAILABLE
#   else
#       undef BHO_ARCH_WORD_BITS_16
#       define BHO_ARCH_WORD_BITS_16 BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#define BHO_ARCH_SH_NAME "SuperH"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_ARCH_SH,BHO_ARCH_SH_NAME)
