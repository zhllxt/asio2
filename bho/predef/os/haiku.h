/*
Copyright Jessica Hamilton 2014
Copyright Rene Rivera 2014-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_OS_HAIKU_H
#define BHO_PREDEF_OS_HAIKU_H

#include <bho/predef/version_number.h>
#include <bho/predef/make.h>

/* tag::reference[]
= `BHO_OS_HAIKU`

http://en.wikipedia.org/wiki/Haiku_(operating_system)[Haiku] operating system.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__HAIKU__+` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_OS_HAIKU BHO_VERSION_NUMBER_NOT_AVAILABLE

#if !defined(BHO_PREDEF_DETAIL_OS_DETECTED) && ( \
    defined(__HAIKU__) \
    )
#   undef BHO_OS_HAIKU
#   define BHO_OS_HAIKU BHO_VERSION_NUMBER_AVAILABLE
#endif

#if BHO_OS_HAIKU
#   define BHO_OS_HAIKU_AVAILABLE
#   include <bho/predef/detail/os_detected.h>
#endif

#define BHO_OS_HAIKU_NAME "Haiku"

#endif

#include <bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_OS_HAIKU,BHO_OS_HAIKU_NAME)
