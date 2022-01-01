/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_LIBRARY_C_GNU_H
#define BHO_PREDEF_LIBRARY_C_GNU_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

#include <asio2/bho/predef/library/c/_prefix.h>

#if defined(__STDC__)
#include <stddef.h>
#elif defined(__cplusplus)
#include <cstddef>
#endif

/* tag::reference[]
= `BHO_LIB_C_GNU`

http://en.wikipedia.org/wiki/Glibc[GNU glibc] Standard C library.
Version number available as major, and minor.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__GLIBC__+` | {predef_detection}
| `+__GNU_LIBRARY__+` | {predef_detection}

| `+__GLIBC__+`, `+__GLIBC_MINOR__+` | V.R.0
| `+__GNU_LIBRARY__+`, `+__GNU_LIBRARY_MINOR__+` | V.R.0
|===
*/ // end::reference[]

#define BHO_LIB_C_GNU BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__GLIBC__) || defined(__GNU_LIBRARY__)
#   undef BHO_LIB_C_GNU
#   if defined(__GLIBC__)
#       define BHO_LIB_C_GNU \
            BHO_VERSION_NUMBER(__GLIBC__,__GLIBC_MINOR__,0)
#   else
#       define BHO_LIB_C_GNU \
            BHO_VERSION_NUMBER(__GNU_LIBRARY__,__GNU_LIBRARY_MINOR__,0)
#   endif
#endif

#if BHO_LIB_C_GNU
#   define BHO_LIB_C_GNU_AVAILABLE
#endif

#define BHO_LIB_C_GNU_NAME "GNU"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_LIB_C_GNU,BHO_LIB_C_GNU_NAME)
