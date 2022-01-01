/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_LIBRARY_STD_MODENA_H
#define BHO_PREDEF_LIBRARY_STD_MODENA_H

#include <asio2/bho/predef/library/std/_prefix.h>

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_LIB_STD_MSIPL`

http://modena.us/[Modena Software Lib++] Standard {CPP} Library.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `MSIPL_COMPILE_H` | {predef_detection}
| `+__MSIPL_COMPILE_H+` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_LIB_STD_MSIPL BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(MSIPL_COMPILE_H) || defined(__MSIPL_COMPILE_H)
#   undef BHO_LIB_STD_MSIPL
#   define BHO_LIB_STD_MSIPL BHO_VERSION_NUMBER_AVAILABLE
#endif

#if BHO_LIB_STD_MSIPL
#   define BHO_LIB_STD_MSIPL_AVAILABLE
#endif

#define BHO_LIB_STD_MSIPL_NAME "Modena Software Lib++"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_LIB_STD_MSIPL,BHO_LIB_STD_MSIPL_NAME)
