/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_LIBRARY_C_VMS_H
#define BHO_PREDEF_LIBRARY_C_VMS_H

#include <asio2/bho/predef/library/c/_prefix.h>

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_LIB_C_VMS`

VMS libc Standard C library.
Version number available as major, minor, and patch.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__CRTL_VER+` | {predef_detection}

| `+__CRTL_VER+` | V.R.P
|===
*/ // end::reference[]

#define BHO_LIB_C_VMS BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__CRTL_VER)
#   undef BHO_LIB_C_VMS
#   define BHO_LIB_C_VMS BHO_PREDEF_MAKE_10_VVRR0PP00(__CRTL_VER)
#endif

#if BHO_LIB_C_VMS
#   define BHO_LIB_C_VMS_AVAILABLE
#endif

#define BHO_LIB_C_VMS_NAME "VMS"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_LIB_C_VMS,BHO_LIB_C_VMS_NAME)
