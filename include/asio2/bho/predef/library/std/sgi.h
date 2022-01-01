/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_LIBRARY_STD_SGI_H
#define BHO_PREDEF_LIBRARY_STD_SGI_H

#include <asio2/bho/predef/library/std/_prefix.h>

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_LIB_STD_SGI`

http://www.sgi.com/tech/stl/[SGI] Standard {CPP} library.
If available version number as major, minor, and patch.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__STL_CONFIG_H+` | {predef_detection}

| `+__SGI_STL+` | V.R.P
|===
*/ // end::reference[]

#define BHO_LIB_STD_SGI BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__STL_CONFIG_H)
#   undef BHO_LIB_STD_SGI
#   if defined(__SGI_STL)
#       define BHO_LIB_STD_SGI BHO_PREDEF_MAKE_0X_VRP(__SGI_STL)
#   else
#       define BHO_LIB_STD_SGI BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if BHO_LIB_STD_SGI
#   define BHO_LIB_STD_SGI_AVAILABLE
#endif

#define BHO_LIB_STD_SGI_NAME "SGI"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_LIB_STD_SGI,BHO_LIB_STD_SGI_NAME)
