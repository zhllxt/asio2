/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_OS_QNXNTO_H
#define BHO_PREDEF_OS_QNXNTO_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_OS_QNX`

http://en.wikipedia.org/wiki/QNX[QNX] operating system.
Version number available as major, and minor if possible. And
version 4 is specifically detected.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__QNX__+` | {predef_detection}
| `+__QNXNTO__+` | {predef_detection}

| `+_NTO_VERSION+` | V.R.0
| `+__QNX__+` | 4.0.0
|===
*/ // end::reference[]

#define BHO_OS_QNX BHO_VERSION_NUMBER_NOT_AVAILABLE

#if !defined(BHO_PREDEF_DETAIL_OS_DETECTED) && ( \
    defined(__QNX__) || defined(__QNXNTO__) \
    )
#   undef BHO_OS_QNX
#   if !defined(BHO_OS_QNX) && defined(_NTO_VERSION)
#       define BHO_OS_QNX BHO_PREDEF_MAKE_10_VVRR(_NTO_VERSION)
#   endif
#   if !defined(BHO_OS_QNX) && defined(__QNX__)
#       define BHO_OS_QNX BHO_VERSION_NUMBER(4,0,0)
#   endif
#   if !defined(BHO_OS_QNX)
#       define BHO_OS_QNX BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if BHO_OS_QNX
#   define BHO_OS_QNX_AVAILABLE
#   include <asio2/bho/predef/detail/os_detected.h>
#endif

#define BHO_OS_QNX_NAME "QNX"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_OS_QNX,BHO_OS_QNX_NAME)
