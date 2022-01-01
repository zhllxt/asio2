/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_OS_HPUX_H
#define BHO_PREDEF_OS_HPUX_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_OS_HPUX`

http://en.wikipedia.org/wiki/HP-UX[HP-UX] operating system.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `hpux` | {predef_detection}
| `+_hpux+` | {predef_detection}
| `+__hpux+` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_OS_HPUX BHO_VERSION_NUMBER_NOT_AVAILABLE

#if !defined(BHO_PREDEF_DETAIL_OS_DETECTED) && ( \
    defined(hpux) || defined(_hpux) || defined(__hpux) \
    )
#   undef BHO_OS_HPUX
#   define BHO_OS_HPUX BHO_VERSION_NUMBER_AVAILABLE
#endif

#if BHO_OS_HPUX
#   define BHO_OS_HPUX_AVAILABLE
#   include <asio2/bho/predef/detail/os_detected.h>
#endif

#define BHO_OS_HPUX_NAME "HP-UX"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_OS_HPUX,BHO_OS_HPUX_NAME)
