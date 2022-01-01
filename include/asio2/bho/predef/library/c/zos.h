/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_LIBRARY_C_ZOS_H
#define BHO_PREDEF_LIBRARY_C_ZOS_H

#include <asio2/bho/predef/library/c/_prefix.h>

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_LIB_C_ZOS`

z/OS libc Standard C library.
Version number available as major, minor, and patch.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__LIBREL__+` | {predef_detection}

| `+__LIBREL__+` | V.R.P
| `+__TARGET_LIB__+` | V.R.P
|===
*/ // end::reference[]

#define BHO_LIB_C_ZOS BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__LIBREL__)
#   undef BHO_LIB_C_ZOS
#   if !defined(BHO_LIB_C_ZOS) && defined(__LIBREL__)
#       define BHO_LIB_C_ZOS BHO_PREDEF_MAKE_0X_VRRPPPP(__LIBREL__)
#   endif
#   if !defined(BHO_LIB_C_ZOS) && defined(__TARGET_LIB__)
#       define BHO_LIB_C_ZOS BHO_PREDEF_MAKE_0X_VRRPPPP(__TARGET_LIB__)
#   endif
#   if !defined(BHO_LIB_C_ZOS)
#       define BHO_LIB_C_ZOS BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if BHO_LIB_C_ZOS
#   define BHO_LIB_C_ZOS_AVAILABLE
#endif

#define BHO_LIB_C_ZOS_NAME "z/OS"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_LIB_C_ZOS,BHO_LIB_C_ZOS_NAME)
