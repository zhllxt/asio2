/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_LIBRARY_C_UC_H
#define BHO_PREDEF_LIBRARY_C_UC_H

#include <asio2/bho/predef/library/c/_prefix.h>

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_LIB_C_UC`

http://en.wikipedia.org/wiki/Uclibc[uClibc] Standard C library.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__UCLIBC__+` | {predef_detection}

| `+__UCLIBC_MAJOR__+`, `+__UCLIBC_MINOR__+`, `+__UCLIBC_SUBLEVEL__+` | V.R.P
|===
*/ // end::reference[]

#define BHO_LIB_C_UC BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__UCLIBC__)
#   undef BHO_LIB_C_UC
#   define BHO_LIB_C_UC BHO_VERSION_NUMBER(\
        __UCLIBC_MAJOR__,__UCLIBC_MINOR__,__UCLIBC_SUBLEVEL__)
#endif

#if BHO_LIB_C_UC
#   define BHO_LIB_C_UC_AVAILABLE
#endif

#define BHO_LIB_C_UC_NAME "uClibc"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_LIB_C_UC,BHO_LIB_C_UC_NAME)
