/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_OS_AIX_H
#define BHO_PREDEF_OS_AIX_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_OS_AIX`

http://en.wikipedia.org/wiki/AIX_operating_system[IBM AIX] operating system.
Version number available as major, minor, and patch.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+_AIX+` | {predef_detection}
| `+__TOS_AIX__+` | {predef_detection}

| `+_AIX43+` | 4.3.0
| `+_AIX41+` | 4.1.0
| `+_AIX32+` | 3.2.0
| `+_AIX3+` | 3.0.0
|===
*/ // end::reference[]

#define BHO_OS_AIX BHO_VERSION_NUMBER_NOT_AVAILABLE

#if !defined(BHO_PREDEF_DETAIL_OS_DETECTED) && ( \
    defined(_AIX) || defined(__TOS_AIX__) \
    )
#   undef BHO_OS_AIX
#   if !defined(BHO_OS_AIX) && defined(_AIX43)
#       define BHO_OS_AIX BHO_VERSION_NUMBER(4,3,0)
#   endif
#   if !defined(BHO_OS_AIX) && defined(_AIX41)
#       define BHO_OS_AIX BHO_VERSION_NUMBER(4,1,0)
#   endif
#   if !defined(BHO_OS_AIX) && defined(_AIX32)
#       define BHO_OS_AIX BHO_VERSION_NUMBER(3,2,0)
#   endif
#   if !defined(BHO_OS_AIX) && defined(_AIX3)
#       define BHO_OS_AIX BHO_VERSION_NUMBER(3,0,0)
#   endif
#   if !defined(BHO_OS_AIX)
#       define BHO_OS_AIX BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if BHO_OS_AIX
#   define BHO_OS_AIX_AVAILABLE
#   include <asio2/bho/predef/detail/os_detected.h>
#endif

#define BHO_OS_AIX_NAME "IBM AIX"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_OS_AIX,BHO_OS_AIX_NAME)
