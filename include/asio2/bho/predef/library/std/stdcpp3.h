/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_LIBRARY_STD_STDCPP3_H
#define BHO_PREDEF_LIBRARY_STD_STDCPP3_H

#include <asio2/bho/predef/library/std/_prefix.h>

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_LIB_STD_GNU`

https://gcc.gnu.org/onlinedocs/libstdc%2b%2b/[GNU libstdc++] Standard {CPP} library.
Version number available as year (from 1970), month, and day.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__GLIBCXX__+` | {predef_detection}
| `+__GLIBCPP__+` | {predef_detection}

| `+__GLIBCXX__+` | V.R.P
| `+__GLIBCPP__+` | V.R.P
|===
*/ // end::reference[]

#define BHO_LIB_STD_GNU BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__GLIBCPP__) || defined(__GLIBCXX__)
#   undef BHO_LIB_STD_GNU
#   if defined(__GLIBCXX__)
#       define BHO_LIB_STD_GNU BHO_PREDEF_MAKE_YYYYMMDD(__GLIBCXX__)
#   else
#       define BHO_LIB_STD_GNU BHO_PREDEF_MAKE_YYYYMMDD(__GLIBCPP__)
#   endif
#endif

#if BHO_LIB_STD_GNU
#   define BHO_LIB_STD_GNU_AVAILABLE
#endif

#define BHO_LIB_STD_GNU_NAME "GNU"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_LIB_STD_GNU,BHO_LIB_STD_GNU_NAME)
