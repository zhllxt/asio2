/*
Copyright Rene Rivera 2011-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_OS_VMS_H
#define BHO_PREDEF_OS_VMS_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_OS_VMS`

http://en.wikipedia.org/wiki/OpenVMS[VMS] operating system.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `VMS` | {predef_detection}
| `+__VMS+` | {predef_detection}

| `+__VMS_VER+` | V.R.P
|===
*/ // end::reference[]

#define BHO_OS_VMS BHO_VERSION_NUMBER_NOT_AVAILABLE

#if !defined(BHO_PREDEF_DETAIL_OS_DETECTED) && ( \
    defined(VMS) || defined(__VMS) \
    )
#   undef BHO_OS_VMS
#   if defined(__VMS_VER)
#       define BHO_OS_VMS BHO_PREDEF_MAKE_10_VVRR00PP00(__VMS_VER)
#   else
#       define BHO_OS_VMS BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if BHO_OS_VMS
#   define BHO_OS_VMS_AVAILABLE
#   include <asio2/bho/predef/detail/os_detected.h>
#endif

#define BHO_OS_VMS_NAME "VMS"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_OS_VMS,BHO_OS_VMS_NAME)
