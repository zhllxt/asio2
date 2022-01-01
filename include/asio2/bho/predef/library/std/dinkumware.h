/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_LIBRARY_STD_DINKUMWARE_H
#define BHO_PREDEF_LIBRARY_STD_DINKUMWARE_H

#include <asio2/bho/predef/library/std/_prefix.h>

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_LIB_STD_DINKUMWARE`

http://en.wikipedia.org/wiki/Dinkumware[Dinkumware] Standard {CPP} Library.
If available version number as major, minor, and patch.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+_YVALS+`, `+__IBMCPP__+` | {predef_detection}
| `+_CPPLIB_VER+` | {predef_detection}

| `+_CPPLIB_VER+` | V.R.0
|===
*/ // end::reference[]

#define BHO_LIB_STD_DINKUMWARE BHO_VERSION_NUMBER_NOT_AVAILABLE

#if (defined(_YVALS) && !defined(__IBMCPP__)) || defined(_CPPLIB_VER)
#   undef BHO_LIB_STD_DINKUMWARE
#   if defined(_CPPLIB_VER)
#       define BHO_LIB_STD_DINKUMWARE BHO_PREDEF_MAKE_10_VVRR(_CPPLIB_VER)
#   else
#       define BHO_LIB_STD_DINKUMWARE BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if BHO_LIB_STD_DINKUMWARE
#   define BHO_LIB_STD_DINKUMWARE_AVAILABLE
#endif

#define BHO_LIB_STD_DINKUMWARE_NAME "Dinkumware"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_LIB_STD_DINKUMWARE,BHO_LIB_STD_DINKUMWARE_NAME)
