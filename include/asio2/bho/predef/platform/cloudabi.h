/*
  Copyright 2017 James E. King, III
  Distributed under the Boost Software License, Version 1.0.
  (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_PLAT_CLOUDABI_H
#define BHO_PREDEF_PLAT_CLOUDABI_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_PLAT_CLOUDABI`

https://github.com/NuxiNL/cloudabi[CloudABI] platform.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__CloudABI__+` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_PLAT_CLOUDABI BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__CloudABI__)
#   undef BHO_PLAT_CLOUDABI
#   define BHO_PLAT_CLOUDABI BHO_VERSION_NUMBER_AVAILABLE
#endif

#if BHO_PLAT_CLOUDABI
#   define BHO_PLAT_CLOUDABI_AVAILABLE
#   include <asio2/bho/predef/detail/platform_detected.h>
#endif

#define BHO_PLAT_CLOUDABI_NAME "CloudABI"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_PLAT_CLOUDABI,BHO_PLAT_CLOUDABI_NAME)
