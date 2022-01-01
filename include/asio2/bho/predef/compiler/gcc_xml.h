/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_GCC_XML_H
#define BHO_PREDEF_COMPILER_GCC_XML_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_COMP_GCCXML`

http://www.gccxml.org/[GCC XML] compiler.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__GCCXML__+` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_COMP_GCCXML BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__GCCXML__)
#   define BHO_COMP_GCCXML_DETECTION BHO_VERSION_NUMBER_AVAILABLE
#endif

#ifdef BHO_COMP_GCCXML_DETECTION
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_GCCXML_EMULATED BHO_COMP_GCCXML_DETECTION
#   else
#       undef BHO_COMP_GCCXML
#       define BHO_COMP_GCCXML BHO_COMP_GCCXML_DETECTION
#   endif
#   define BHO_COMP_GCCXML_AVAILABLE
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_GCCXML_NAME "GCC XML"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_GCCXML,BHO_COMP_GCCXML_NAME)

#ifdef BHO_COMP_GCCXML_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_GCCXML_EMULATED,BHO_COMP_GCCXML_NAME)
#endif
