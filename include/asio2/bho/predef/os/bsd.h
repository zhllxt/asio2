/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_OS_BSD_H
#define BHO_PREDEF_OS_BSD_H

/* Special case: OSX will define BSD predefs if the sys/param.h
 * header is included. We can guard against that, but only if we
 * detect OSX first. Hence we will force include OSX detection
 * before doing any BSD detection.
 */
#include <asio2/bho/predef/os/macos.h>

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_OS_BSD`

http://en.wikipedia.org/wiki/Berkeley_Software_Distribution[BSD] operating system.

BSD has various branch operating systems possible and each detected
individually. This detects the following variations and sets a specific
version number macro to match:

* `BHO_OS_BSD_DRAGONFLY` http://en.wikipedia.org/wiki/DragonFly_BSD[DragonFly BSD]
* `BHO_OS_BSD_FREE` http://en.wikipedia.org/wiki/Freebsd[FreeBSD]
* `BHO_OS_BSD_BSDI` http://en.wikipedia.org/wiki/BSD/OS[BSDi BSD/OS]
* `BHO_OS_BSD_NET` http://en.wikipedia.org/wiki/Netbsd[NetBSD]
* `BHO_OS_BSD_OPEN` http://en.wikipedia.org/wiki/Openbsd[OpenBSD]

NOTE: The general `BHO_OS_BSD` is set in all cases to indicate some form
of BSD. If the above variants is detected the corresponding macro is also set.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `BSD` | {predef_detection}
| `+_SYSTYPE_BSD+` | {predef_detection}

| `BSD4_2` | 4.2.0
| `BSD4_3` | 4.3.0
| `BSD4_4` | 4.4.0
| `BSD` | V.R.0
|===
*/ // end::reference[]

#include <asio2/bho/predef/os/bsd/bsdi.h>
#include <asio2/bho/predef/os/bsd/dragonfly.h>
#include <asio2/bho/predef/os/bsd/free.h>
#include <asio2/bho/predef/os/bsd/open.h>
#include <asio2/bho/predef/os/bsd/net.h>

#ifndef BHO_OS_BSD
#define BHO_OS_BSD BHO_VERSION_NUMBER_NOT_AVAILABLE
#endif

#if !defined(BHO_PREDEF_DETAIL_OS_DETECTED) && ( \
    defined(BSD) || \
    defined(_SYSTYPE_BSD) \
    )
#   undef BHO_OS_BSD
#   include <sys/param.h>
#   if !defined(BHO_OS_BSD) && defined(BSD4_4)
#       define BHO_OS_BSD BHO_VERSION_NUMBER(4,4,0)
#   endif
#   if !defined(BHO_OS_BSD) && defined(BSD4_3)
#       define BHO_OS_BSD BHO_VERSION_NUMBER(4,3,0)
#   endif
#   if !defined(BHO_OS_BSD) && defined(BSD4_2)
#       define BHO_OS_BSD BHO_VERSION_NUMBER(4,2,0)
#   endif
#   if !defined(BHO_OS_BSD) && defined(BSD)
#       define BHO_OS_BSD BHO_PREDEF_MAKE_10_VVRR(BSD)
#   endif
#   if !defined(BHO_OS_BSD)
#       define BHO_OS_BSD BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if BHO_OS_BSD
#   define BHO_OS_BSD_AVAILABLE
#   include <asio2/bho/predef/detail/os_detected.h>
#endif

#define BHO_OS_BSD_NAME "BSD"

#endif

#include <asio2/bho/predef/os/bsd/bsdi.h>
#include <asio2/bho/predef/os/bsd/dragonfly.h>
#include <asio2/bho/predef/os/bsd/free.h>
#include <asio2/bho/predef/os/bsd/open.h>
#include <asio2/bho/predef/os/bsd/net.h>

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_OS_BSD,BHO_OS_BSD_NAME)
