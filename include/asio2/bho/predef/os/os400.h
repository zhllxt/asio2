/*
Copyright Rene Rivera 2011-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_OS_OS400_H
#define BHO_PREDEF_OS_OS400_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_OS_OS400`

http://en.wikipedia.org/wiki/IBM_i[IBM OS/400] operating system.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__OS400__+` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_OS_OS400 BHO_VERSION_NUMBER_NOT_AVAILABLE

#if !defined(BHO_PREDEF_DETAIL_OS_DETECTED) && ( \
    defined(__OS400__) \
    )
#   undef BHO_OS_OS400
#   define BHO_OS_OS400 BHO_VERSION_NUMBER_AVAILABLE
#endif

#if BHO_OS_OS400
#   define BHO_OS_OS400_AVAILABLE
#   include <asio2/bho/predef/detail/os_detected.h>
#endif

#define BHO_OS_OS400_NAME "IBM OS/400"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_OS_OS400,BHO_OS_OS400_NAME)
