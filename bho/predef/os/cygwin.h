/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_OS_CYGWIN_H
#define BHO_PREDEF_OS_CYGWIN_H

#include <bho/predef/version_number.h>
#include <bho/predef/make.h>

/* tag::reference[]
= `BHO_OS_CYGWIN`

http://en.wikipedia.org/wiki/Cygwin[Cygwin] evironment.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__CYGWIN__+` | {predef_detection}

| `CYGWIN_VERSION_API_MAJOR`, `CYGWIN_VERSION_API_MINOR` | V.R.0
|===
*/ // end::reference[]

#define BHO_OS_CYGWIN BHO_VERSION_NUMBER_NOT_AVAILABLE

#if !defined(BHO_PREDEF_DETAIL_OS_DETECTED) && ( \
    defined(__CYGWIN__) \
    )
#   include <cygwin/version.h>
#   undef BHO_OS_CYGWIN
#   define BHO_OS_CYGWIN \
        BHO_VERSION_NUMBER(CYGWIN_VERSION_API_MAJOR,\
                             CYGWIN_VERSION_API_MINOR, 0)
#endif

#if BHO_OS_CYGWIN
#   define BHO_OS_CYGWIN_AVAILABLE
#   include <bho/predef/detail/os_detected.h>
#endif

#define BHO_OS_CYGWIN_NAME "Cygwin"

#endif

#include <bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_OS_CYGWIN,BHO_OS_CYGWIN_NAME)
