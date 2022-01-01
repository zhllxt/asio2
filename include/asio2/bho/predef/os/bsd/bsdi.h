/*
Copyright Rene Rivera 2012-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_OS_BSD_BSDI_H
#define BHO_PREDEF_OS_BSD_BSDI_H

#include <asio2/bho/predef/os/bsd.h>

/* tag::reference[]
= `BHO_OS_BSD_BSDI`

http://en.wikipedia.org/wiki/BSD/OS[BSDi BSD/OS] operating system.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__bsdi__+` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_OS_BSD_BSDI BHO_VERSION_NUMBER_NOT_AVAILABLE

#if !defined(BHO_PREDEF_DETAIL_OS_DETECTED) && ( \
    defined(__bsdi__) \
    )
#   ifndef BHO_OS_BSD_AVAILABLE
#       undef BHO_OS_BSD
#       define BHO_OS_BSD BHO_VERSION_NUMBER_AVAILABLE
#       define BHO_OS_BSD_AVAILABLE
#   endif
#   undef BHO_OS_BSD_BSDI
#   define BHO_OS_BSD_BSDI BHO_VERSION_NUMBER_AVAILABLE
#endif

#if BHO_OS_BSD_BSDI
#   define BHO_OS_BSD_BSDI_AVAILABLE
#   include <asio2/bho/predef/detail/os_detected.h>
#endif

#define BHO_OS_BSD_BSDI_NAME "BSDi BSD/OS"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_OS_BSD_BSDI,BHO_OS_BSD_BSDI_NAME)
