/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_LIBRARY_STD_MSL_H
#define BHO_PREDEF_LIBRARY_STD_MSL_H

#include <asio2/bho/predef/library/std/_prefix.h>

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_LIB_STD_MSL`

http://www.freescale.com/[Metrowerks] Standard {CPP} Library.
Version number available as major, minor, and patch.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__MSL_CPP__+` | {predef_detection}
| `+__MSL__+` | {predef_detection}

| `+__MSL_CPP__+` | V.R.P
| `+__MSL__+` | V.R.P
|===
*/ // end::reference[]

#define BHO_LIB_STD_MSL BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__MSL_CPP__) || defined(__MSL__)
#   undef BHO_LIB_STD_MSL
#   if defined(__MSL_CPP__)
#       define BHO_LIB_STD_MSL BHO_PREDEF_MAKE_0X_VRPP(__MSL_CPP__)
#   else
#       define BHO_LIB_STD_MSL BHO_PREDEF_MAKE_0X_VRPP(__MSL__)
#   endif
#endif

#if BHO_LIB_STD_MSL
#   define BHO_LIB_STD_MSL_AVAILABLE
#endif

#define BHO_LIB_STD_MSL_NAME "Metrowerks"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_LIB_STD_MSL,BHO_LIB_STD_MSL_NAME)
