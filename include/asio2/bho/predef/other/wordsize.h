/*
Copyright Rene Ferdinand Rivera Morell 2020-2021
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_OTHER_WORD_SIZE_H
#define BHO_PREDEF_OTHER_WORD_SIZE_H

#include <asio2/bho/predef/architecture.h>
#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_ARCH_WORD_BITS`

Detects the native word size, in bits, for the current architecture. There are
two types of macros for this detection:

* `BHO_ARCH_WORD_BITS`, gives the number of word size bits
  (16, 32, 64).
* `BHO_ARCH_WORD_BITS_16`, `BHO_ARCH_WORD_BITS_32`, and
  `BHO_ARCH_WORD_BITS_64`, indicate when the given word size is
  detected.

They allow for both single checks and direct use of the size in code.

NOTE: The word size is determined manually on each architecture. Hence use of
the `wordsize.h` header will also include all the architecture headers.

*/ // end::reference[]

#if !defined(BHO_ARCH_WORD_BITS_64)
#   define BHO_ARCH_WORD_BITS_64 BHO_VERSION_NUMBER_NOT_AVAILABLE
#elif !defined(BHO_ARCH_WORD_BITS)
#   define BHO_ARCH_WORD_BITS 64
#endif

#if !defined(BHO_ARCH_WORD_BITS_32)
#   define BHO_ARCH_WORD_BITS_32 BHO_VERSION_NUMBER_NOT_AVAILABLE
#elif !defined(BHO_ARCH_WORD_BITS)
#	  define BHO_ARCH_WORD_BITS 32
#endif

#if !defined(BHO_ARCH_WORD_BITS_16)
#   define BHO_ARCH_WORD_BITS_16 BHO_VERSION_NUMBER_NOT_AVAILABLE
#elif !defined(BHO_ARCH_WORD_BITS)
#   define BHO_ARCH_WORD_BITS 16
#endif

#if !defined(BHO_ARCH_WORD_BITS)
#   define BHO_ARCH_WORD_BITS 0
#endif

#define BHO_ARCH_WORD_BITS_NAME "Word Bits"
#define BHO_ARCH_WORD_BITS_16_NAME "16-bit Word Size"
#define BHO_ARCH_WORD_BITS_32_NAME "32-bit Word Size"
#define BHO_ARCH_WORD_BITS_64_NAME "64-bit Word Size"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_ARCH_WORD_BITS,BHO_ARCH_WORD_BITS_NAME)

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_ARCH_WORD_BITS_16,BHO_ARCH_WORD_BITS_16_NAME)

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_ARCH_WORD_BITS_32,BHO_ARCH_WORD_BITS_32_NAME)

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_ARCH_WORD_BITS_64,BHO_ARCH_WORD_BITS_64_NAME)
