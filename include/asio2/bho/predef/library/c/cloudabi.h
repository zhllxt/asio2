/*
 * Copyright (C) 2017 James E. King III
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 *   http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef BHO_PREDEF_LIBRARY_C_CLOUDABI_H
#define BHO_PREDEF_LIBRARY_C_CLOUDABI_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

#include <asio2/bho/predef/library/c/_prefix.h>

#if defined(__CloudABI__)
#include <stddef.h>
#endif

/* tag::reference[]
= `BHO_LIB_C_CLOUDABI`

https://github.com/NuxiNL/cloudlibc[cloudlibc] - CloudABI's standard C library.
Version number available as major, and minor.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__cloudlibc__+` | {predef_detection}

| `+__cloudlibc_major__+`, `+__cloudlibc_minor__+` | V.R.0
|===
*/ // end::reference[]

#define BHO_LIB_C_CLOUDABI BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__cloudlibc__)
#   undef BHO_LIB_C_CLOUDABI
#   define BHO_LIB_C_CLOUDABI \
            BHO_VERSION_NUMBER(__cloudlibc_major__,__cloudlibc_minor__,0)
#endif

#if BHO_LIB_C_CLOUDABI
#   define BHO_LIB_C_CLOUDABI_AVAILABLE
#endif

#define BHO_LIB_C_CLOUDABI_NAME "cloudlibc"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_LIB_C_CLOUDABI,BHO_LIB_C_CLOUDABI_NAME)
