/*
Copyright Rene Rivera 2012-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_OS_BSD_FREE_H
#define BHO_PREDEF_OS_BSD_FREE_H

#include <asio2/bho/predef/os/bsd.h>

/* tag::reference[]
= `BHO_OS_BSD_FREE`

http://en.wikipedia.org/wiki/Freebsd[FreeBSD] operating system.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__FreeBSD__+` | {predef_detection}

| `+__FreeBSD_version+` | V.R.P
|===
*/ // end::reference[]

#define BHO_OS_BSD_FREE BHO_VERSION_NUMBER_NOT_AVAILABLE

#if !defined(BHO_PREDEF_DETAIL_OS_DETECTED) && ( \
    defined(__FreeBSD__) \
    )
#   ifndef BHO_OS_BSD_AVAILABLE
#       undef BHO_OS_BSD
#       define BHO_OS_BSD BHO_VERSION_NUMBER_AVAILABLE
#       define BHO_OS_BSD_AVAILABLE
#   endif
#   undef BHO_OS_BSD_FREE
#   include <sys/param.h>
#   if defined(__FreeBSD_version)
#       if __FreeBSD_version == 491000
#           define BHO_OS_BSD_FREE \
                BHO_VERSION_NUMBER(4, 10, 0)
#       elif __FreeBSD_version == 492000
#           define BHO_OS_BSD_FREE \
                BHO_VERSION_NUMBER(4, 11, 0)
#       elif __FreeBSD_version < 500000
#           define BHO_OS_BSD_FREE \
                BHO_PREDEF_MAKE_10_VRPPPP(__FreeBSD_version)
#       else
#           define BHO_OS_BSD_FREE \
                BHO_PREDEF_MAKE_10_VVRRPPP(__FreeBSD_version)
#       endif
#   else
#       define BHO_OS_BSD_FREE BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if BHO_OS_BSD_FREE
#   define BHO_OS_BSD_FREE_AVAILABLE
#   include <asio2/bho/predef/detail/os_detected.h>
#endif

#define BHO_OS_BSD_FREE_NAME "Free BSD"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_OS_BSD_FREE,BHO_OS_BSD_FREE_NAME)
