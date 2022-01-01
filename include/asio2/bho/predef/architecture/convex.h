/*
Copyright Rene Rivera 2011-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_ARCHITECTURE_CONVEX_H
#define BHO_PREDEF_ARCHITECTURE_CONVEX_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_ARCH_CONVEX`

http://en.wikipedia.org/wiki/Convex_Computer[Convex Computer] architecture.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__convex__+` | {predef_detection}

| `+__convex_c1__+` | 1.0.0
| `+__convex_c2__+` | 2.0.0
| `+__convex_c32__+` | 3.2.0
| `+__convex_c34__+` | 3.4.0
| `+__convex_c38__+` | 3.8.0
|===
*/ // end::reference[]

#define BHO_ARCH_CONVEX BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__convex__)
#   undef BHO_ARCH_CONVEX
#   if !defined(BHO_ARCH_CONVEX) && defined(__convex_c1__)
#       define BHO_ARCH_CONVEX BHO_VERSION_NUMBER(1,0,0)
#   endif
#   if !defined(BHO_ARCH_CONVEX) && defined(__convex_c2__)
#       define BHO_ARCH_CONVEX BHO_VERSION_NUMBER(2,0,0)
#   endif
#   if !defined(BHO_ARCH_CONVEX) && defined(__convex_c32__)
#       define BHO_ARCH_CONVEX BHO_VERSION_NUMBER(3,2,0)
#   endif
#   if !defined(BHO_ARCH_CONVEX) && defined(__convex_c34__)
#       define BHO_ARCH_CONVEX BHO_VERSION_NUMBER(3,4,0)
#   endif
#   if !defined(BHO_ARCH_CONVEX) && defined(__convex_c38__)
#       define BHO_ARCH_CONVEX BHO_VERSION_NUMBER(3,8,0)
#   endif
#   if !defined(BHO_ARCH_CONVEX)
#       define BHO_ARCH_CONVEX BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if BHO_ARCH_CONVEX
#   define BHO_ARCH_CONVEX_AVAILABLE
#endif

#if BHO_ARCH_CONVEX
#   undef BHO_ARCH_WORD_BITS_32
#   define BHO_ARCH_WORD_BITS_32 BHO_VERSION_NUMBER_AVAILABLE
#endif

#define BHO_ARCH_CONVEX_NAME "Convex Computer"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_ARCH_CONVEX,BHO_ARCH_CONVEX_NAME)
