/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_LIBRARY_STD_LIBCOMO_H
#define BHO_PREDEF_LIBRARY_STD_LIBCOMO_H

#include <asio2/bho/predef/library/std/_prefix.h>

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_LIB_STD_COMO`

http://www.comeaucomputing.com/libcomo/[Comeau Computing] Standard {CPP} Library.
Version number available as major.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__LIBCOMO__+` | {predef_detection}

| `+__LIBCOMO_VERSION__+` | V.0.0
|===
*/ // end::reference[]

#define BHO_LIB_STD_COMO BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__LIBCOMO__)
#   undef BHO_LIB_STD_COMO
#   define BHO_LIB_STD_COMO BHO_VERSION_NUMBER(__LIBCOMO_VERSION__,0,0)
#endif

#if BHO_LIB_STD_COMO
#   define BHO_LIB_STD_COMO_AVAILABLE
#endif

#define BHO_LIB_STD_COMO_NAME "Comeau Computing"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_LIB_STD_COMO,BHO_LIB_STD_COMO_NAME)
