/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_LIBRARY_STD_VACPP_H
#define BHO_PREDEF_LIBRARY_STD_VACPP_H

#include <asio2/bho/predef/library/std/_prefix.h>

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_LIB_STD_IBM`

http://www.ibm.com/software/awdtools/xlcpp/[IBM VACPP Standard {CPP}] library.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__IBMCPP__+` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_LIB_STD_IBM BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__IBMCPP__)
#   undef BHO_LIB_STD_IBM
#   define BHO_LIB_STD_IBM BHO_VERSION_NUMBER_AVAILABLE
#endif

#if BHO_LIB_STD_IBM
#   define BHO_LIB_STD_IBM_AVAILABLE
#endif

#define BHO_LIB_STD_IBM_NAME "IBM VACPP"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_LIB_STD_IBM,BHO_LIB_STD_IBM_NAME)
