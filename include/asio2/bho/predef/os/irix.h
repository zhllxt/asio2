/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_OS_IRIX_H
#define BHO_PREDEF_OS_IRIX_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_OS_IRIX`

http://en.wikipedia.org/wiki/Irix[IRIX] operating system.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `sgi` | {predef_detection}
| `+__sgi+` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_OS_IRIX BHO_VERSION_NUMBER_NOT_AVAILABLE

#if !defined(BHO_PREDEF_DETAIL_OS_DETECTED) && ( \
    defined(sgi) || defined(__sgi) \
    )
#   undef BHO_OS_IRIX
#   define BHO_OS_IRIX BHO_VERSION_NUMBER_AVAILABLE
#endif

#if BHO_OS_IRIX
#   define BHO_OS_IRIX_AVAILABLE
#   include <asio2/bho/predef/detail/os_detected.h>
#endif

#define BHO_OS_IRIX_NAME "IRIX"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_OS_IRIX,BHO_OS_IRIX_NAME)
