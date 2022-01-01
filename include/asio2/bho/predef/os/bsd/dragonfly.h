/*
Copyright Rene Rivera 2012-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_OS_BSD_DRAGONFLY_H
#define BHO_PREDEF_OS_BSD_DRAGONFLY_H

#include <asio2/bho/predef/os/bsd.h>

/* tag::reference[]
= `BHO_OS_BSD_DRAGONFLY`

http://en.wikipedia.org/wiki/DragonFly_BSD[DragonFly BSD] operating system.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__DragonFly__+` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_OS_BSD_DRAGONFLY BHO_VERSION_NUMBER_NOT_AVAILABLE

#if !defined(BHO_PREDEF_DETAIL_OS_DETECTED) && ( \
    defined(__DragonFly__) \
    )
#   ifndef BHO_OS_BSD_AVAILABLE
#       undef BHO_OS_BSD
#       define BHO_OS_BSD BHO_VERSION_NUMBER_AVAILABLE
#       define BHO_OS_BSD_AVAILABLE
#   endif
#   undef BHO_OS_BSD_DRAGONFLY
#   if defined(__DragonFly__)
#       define BHO_OS_DRAGONFLY_BSD BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if BHO_OS_BSD_DRAGONFLY
#   define BHO_OS_BSD_DRAGONFLY_AVAILABLE
#   include <asio2/bho/predef/detail/os_detected.h>
#endif

#define BHO_OS_BSD_DRAGONFLY_NAME "DragonFly BSD"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_OS_BSD_DRAGONFLY,BHO_OS_BSD_DRAGONFLY_NAME)
