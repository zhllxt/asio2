/*
Copyright Henrik S. Gaßmann 2023
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_LIBRARY_STD_MSVC_H
#define BHO_PREDEF_LIBRARY_STD_MSVC_H

#include <asio2/bho/predef/library/std/_prefix.h>

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_LIB_STD_MSVC`

https://github.com/microsoft/STL[Microsoft's {CPP} Standard Library].
If available version number as major, minor, and patch.
The patch number is derived from `_MSVC_STL_UPDATE` by taking its five last
digits (see below). This implies that pasting a `_MSVC_STL_UPDATE` value into
`BHO_VERSION_NUMBER` will produce a version number that is directly comparable
to `BHO_LIB_STD_MSVC`.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+_MSVC_STL_VERSION+` | {predef_detection}

| `+_MSVC_STL_VERSION+` | VV.R.0
| `+_MSVC_STL_UPDATE+` | 00.0.0YYYMM
|===
*/ // end::reference[]

#define BHO_LIB_STD_MSVC BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(_MSVC_STL_VERSION)
#   undef BHO_LIB_STD_MSVC
#   define BHO_LIB_STD_MSVC BHO_PREDEF_MAKE_10_VVR_0PPPPP(_MSVC_STL_VERSION, _MSVC_STL_UPDATE)
#endif

#if BHO_LIB_STD_MSVC
#   define BHO_LIB_STD_MSVC_AVAILABLE
#endif

#define BHO_LIB_STD_MSVC_NAME "Microsoft stdlib"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_LIB_STD_MSVC, BHO_LIB_STD_MSVC_NAME)
