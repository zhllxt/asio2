/*
Copyright Rene Rivera 2011-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_LIBRARY_STD_CXX_H
#define BHO_PREDEF_LIBRARY_STD_CXX_H

#include <asio2/bho/predef/library/std/_prefix.h>

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_LIB_STD_CXX`

http://libcxx.llvm.org/[libc++] {CPP} Standard Library.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+_LIBCPP_VERSION+` | {predef_detection}

| `+_LIBCPP_VERSION+` | V.0.P
|===
*/ // end::reference[]

#define BHO_LIB_STD_CXX BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(_LIBCPP_VERSION)
#   undef BHO_LIB_STD_CXX
#   define BHO_LIB_STD_CXX BHO_PREDEF_MAKE_10_VVPPP(_LIBCPP_VERSION)
#endif

#if BHO_LIB_STD_CXX
#   define BHO_LIB_STD_CXX_AVAILABLE
#endif

#define BHO_LIB_STD_CXX_NAME "libc++"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_LIB_STD_CXX,BHO_LIB_STD_CXX_NAME)
