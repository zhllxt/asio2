/*
Copyright Benjamin Worpitz 2018
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_ARCHITECTURE_PTX_H
#define BHO_PREDEF_ARCHITECTURE_PTX_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_ARCH_PTX`

https://en.wikipedia.org/wiki/Parallel_Thread_Execution[PTX] architecture.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__CUDA_ARCH__+` | {predef_detection}

| `+__CUDA_ARCH__+` | V.R.0
|===
*/ // end::reference[]

#define BHO_ARCH_PTX BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__CUDA_ARCH__)
#   undef BHO_ARCH_PTX
#   define BHO_ARCH_PTX BHO_PREDEF_MAKE_10_VR0(__CUDA_ARCH__)
#endif

#if BHO_ARCH_PTX
#   define BHO_ARCH_PTX_AVAILABLE
#endif

#if BHO_ARCH_PTX
#   undef BHO_ARCH_WORD_BITS_64
#   define BHO_ARCH_WORD_BITS_64 BHO_VERSION_NUMBER_AVAILABLE
#endif

#define BHO_ARCH_PTX_NAME "PTX"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_ARCH_PTX,BHO_ARCH_PTX_NAME)
