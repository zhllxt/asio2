/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_ARCHITECTURE_ALPHA_H
#define BHO_PREDEF_ARCHITECTURE_ALPHA_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_ARCH_ALPHA`

http://en.wikipedia.org/wiki/DEC_Alpha[DEC Alpha] architecture.

[options="header"]
|===
| {predef_symbol} | {predef_version}
| `+__alpha__+` | {predef_detection}
| `+__alpha+` | {predef_detection}
| `+_M_ALPHA+` | {predef_detection}

| `+__alpha_ev4__+` | 4.0.0
| `+__alpha_ev5__+` | 5.0.0
| `+__alpha_ev6__+` | 6.0.0
|===
*/ // end::reference[]

#define BHO_ARCH_ALPHA BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__alpha__) || defined(__alpha) || \
    defined(_M_ALPHA)
#   undef BHO_ARCH_ALPHA
#   if !defined(BHO_ARCH_ALPHA) && defined(__alpha_ev4__)
#       define BHO_ARCH_ALPHA BHO_VERSION_NUMBER(4,0,0)
#   endif
#   if !defined(BHO_ARCH_ALPHA) && defined(__alpha_ev5__)
#       define BHO_ARCH_ALPHA BHO_VERSION_NUMBER(5,0,0)
#   endif
#   if !defined(BHO_ARCH_ALPHA) && defined(__alpha_ev6__)
#       define BHO_ARCH_ALPHA BHO_VERSION_NUMBER(6,0,0)
#   endif
#   if !defined(BHO_ARCH_ALPHA)
#       define BHO_ARCH_ALPHA BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if BHO_ARCH_ALPHA
#   define BHO_ARCH_ALPHA_AVAILABLE
#endif

#if BHO_ARCH_ALPHA
#   undef BHO_ARCH_WORD_BITS_64
#   define BHO_ARCH_WORD_BITS_64 BHO_VERSION_NUMBER_AVAILABLE
#endif

#define BHO_ARCH_ALPHA_NAME "DEC Alpha"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_ARCH_ALPHA,BHO_ARCH_ALPHA_NAME)
