/*
Copyright Rene Rivera 2008-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#include <asio2/bho/predef/architecture/x86/32.h>
#include <asio2/bho/predef/architecture/x86/64.h>

#ifndef BHO_PREDEF_ARCHITECTURE_X86_H
#define BHO_PREDEF_ARCHITECTURE_X86_H

/* tag::reference[]
= `BHO_ARCH_X86`

http://en.wikipedia.org/wiki/X86[Intel x86] architecture. This is
a category to indicate that either `BHO_ARCH_X86_32` or
`BHO_ARCH_X86_64` is detected.
*/ // end::reference[]

#define BHO_ARCH_X86 BHO_VERSION_NUMBER_NOT_AVAILABLE

#if BHO_ARCH_X86_32 || BHO_ARCH_X86_64
#   undef BHO_ARCH_X86
#   define BHO_ARCH_X86 BHO_VERSION_NUMBER_AVAILABLE
#endif

#if BHO_ARCH_X86
#   define BHO_ARCH_X86_AVAILABLE
#endif

#define BHO_ARCH_X86_NAME "Intel x86"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_ARCH_X86,BHO_ARCH_X86_NAME)
