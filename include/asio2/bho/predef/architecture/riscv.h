/*
Copyright Andreas Schwab 2019
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_ARCHITECTURE_RISCV_H
#define BHO_PREDEF_ARCHITECTURE_RISCV_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_ARCH_RISCV`

http://en.wikipedia.org/wiki/RISC-V[RISC-V] architecture.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__riscv+` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_ARCH_RISCV BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__riscv)
#   undef BHO_ARCH_RISCV
#   define BHO_ARCH_RISCV BHO_VERSION_NUMBER_AVAILABLE
#endif

#if BHO_ARCH_RISCV
#   define BHO_ARCH_RISCV_AVAILABLE
#endif

#if BHO_ARCH_RISCV
#   undef BHO_ARCH_WORD_BITS_32
#   define BHO_ARCH_WORD_BITS_32 BHO_VERSION_NUMBER_AVAILABLE
#endif

#define BHO_ARCH_RISCV_NAME "RISC-V"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_ARCH_RISCV,BHO_ARCH_RISCV_NAME)
