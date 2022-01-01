/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_COMPILER_LLVM_H
#define BHO_PREDEF_COMPILER_LLVM_H

/* Other compilers that emulate this one need to be detected first. */

#include <asio2/bho/predef/compiler/clang.h>

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_COMP_LLVM`

http://en.wikipedia.org/wiki/LLVM[LLVM] compiler.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__llvm__+` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_COMP_LLVM BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__llvm__)
#   define BHO_COMP_LLVM_DETECTION BHO_VERSION_NUMBER_AVAILABLE
#endif

#ifdef BHO_COMP_LLVM_DETECTION
#   if defined(BHO_PREDEF_DETAIL_COMP_DETECTED)
#       define BHO_COMP_LLVM_EMULATED BHO_COMP_LLVM_DETECTION
#   else
#       undef BHO_COMP_LLVM
#       define BHO_COMP_LLVM BHO_COMP_LLVM_DETECTION
#   endif
#   define BHO_COMP_LLVM_AVAILABLE
#   include <asio2/bho/predef/detail/comp_detected.h>
#endif

#define BHO_COMP_LLVM_NAME "LLVM"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_LLVM,BHO_COMP_LLVM_NAME)

#ifdef BHO_COMP_LLVM_EMULATED
#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_COMP_LLVM_EMULATED,BHO_COMP_LLVM_NAME)
#endif
