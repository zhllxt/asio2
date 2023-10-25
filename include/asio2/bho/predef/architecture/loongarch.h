/*
Copyright Zhang Na 2022
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_ARCHITECTURE_LOONGARCH_H
#define BHO_PREDEF_ARCHITECTURE_LOONGARCH_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_ARCH_LOONGARCH`

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__loongarch__+` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_ARCH_LOONGARCH BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__loongarch__)
#   undef BHO_ARCH_LOONGARCH
#   define BHO_ARCH_LOONGARCH BHO_VERSION_NUMBER_AVAILABLE
#endif

#if BHO_ARCH_LOONGARCH
#   define BHO_ARCH_LOONGARCH_AVAILABLE
#endif

#define BHO_ARCH_LOONGARCH_NAME "LoongArch"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_ARCH_LOONGARCH,BHO_ARCH_LOONGARCH_NAME)
