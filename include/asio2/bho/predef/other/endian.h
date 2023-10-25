/*
Copyright Rene Rivera 2013-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_ENDIAN_H
#define BHO_PREDEF_ENDIAN_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>
#include <asio2/bho/predef/library/c/gnu.h>
#include <asio2/bho/predef/os/macos.h>
#include <asio2/bho/predef/os/bsd.h>
#include <asio2/bho/predef/platform/android.h>

/* tag::reference[]
= `BHO_ENDIAN_*`

Detection of endian memory ordering. There are four defined macros
in this header that define the various generally possible endian
memory orderings:

* `BHO_ENDIAN_BIG_BYTE`, byte-swapped big-endian.
* `BHO_ENDIAN_BIG_WORD`, word-swapped big-endian.
* `BHO_ENDIAN_LITTLE_BYTE`, byte-swapped little-endian.
* `BHO_ENDIAN_LITTLE_WORD`, word-swapped little-endian.

The detection is conservative in that it only identifies endianness
that it knows for certain. In particular bi-endianness is not
indicated as is it not practically possible to determine the
endianness from anything but an operating system provided
header. And the currently known headers do not define that
programatic bi-endianness is available.

This implementation is a compilation of various publicly available
information and acquired knowledge:

. The indispensable documentation of "Pre-defined Compiler Macros"
  http://sourceforge.net/p/predef/wiki/Endianness[Endianness].
. The various endian specifications available in the
  http://wikipedia.org/[Wikipedia] computer architecture pages.
. Generally available searches for headers that define endianness.
*/ // end::reference[]

#define BHO_ENDIAN_BIG_BYTE BHO_VERSION_NUMBER_NOT_AVAILABLE
#define BHO_ENDIAN_BIG_WORD BHO_VERSION_NUMBER_NOT_AVAILABLE
#define BHO_ENDIAN_LITTLE_BYTE BHO_VERSION_NUMBER_NOT_AVAILABLE
#define BHO_ENDIAN_LITTLE_WORD BHO_VERSION_NUMBER_NOT_AVAILABLE

/* GNU libc provides a header defining __BYTE_ORDER, or _BYTE_ORDER.
 * And some OSs provide some for of endian header also.
 */
#if !BHO_ENDIAN_BIG_BYTE && !BHO_ENDIAN_BIG_WORD && \
    !BHO_ENDIAN_LITTLE_BYTE && !BHO_ENDIAN_LITTLE_WORD
#   if BHO_LIB_C_GNU || BHO_PLAT_ANDROID || BHO_OS_BSD_OPEN
#       include <endian.h>
#   else
#       if BHO_OS_MACOS
#           include <machine/endian.h>
#       else
#           if BHO_OS_BSD
#               include <sys/endian.h>
#           endif
#       endif
#   endif
#   if defined(__BYTE_ORDER)
#       if defined(__BIG_ENDIAN) && (__BYTE_ORDER == __BIG_ENDIAN)
#           undef BHO_ENDIAN_BIG_BYTE
#           define BHO_ENDIAN_BIG_BYTE BHO_VERSION_NUMBER_AVAILABLE
#       endif
#       if defined(__LITTLE_ENDIAN) && (__BYTE_ORDER == __LITTLE_ENDIAN)
#           undef BHO_ENDIAN_LITTLE_BYTE
#           define BHO_ENDIAN_LITTLE_BYTE BHO_VERSION_NUMBER_AVAILABLE
#       endif
#       if defined(__PDP_ENDIAN) && (__BYTE_ORDER == __PDP_ENDIAN)
#           undef BHO_ENDIAN_LITTLE_WORD
#           define BHO_ENDIAN_LITTLE_WORD BHO_VERSION_NUMBER_AVAILABLE
#       endif
#   endif
#   if !defined(__BYTE_ORDER) && defined(_BYTE_ORDER)
#       if defined(_BIG_ENDIAN) && (_BYTE_ORDER == _BIG_ENDIAN)
#           undef BHO_ENDIAN_BIG_BYTE
#           define BHO_ENDIAN_BIG_BYTE BHO_VERSION_NUMBER_AVAILABLE
#       endif
#       if defined(_LITTLE_ENDIAN) && (_BYTE_ORDER == _LITTLE_ENDIAN)
#           undef BHO_ENDIAN_LITTLE_BYTE
#           define BHO_ENDIAN_LITTLE_BYTE BHO_VERSION_NUMBER_AVAILABLE
#       endif
#       if defined(_PDP_ENDIAN) && (_BYTE_ORDER == _PDP_ENDIAN)
#           undef BHO_ENDIAN_LITTLE_WORD
#           define BHO_ENDIAN_LITTLE_WORD BHO_VERSION_NUMBER_AVAILABLE
#       endif
#   endif
#endif

/* Built-in byte-swapped big-endian macros.
 */
#if !BHO_ENDIAN_BIG_BYTE && !BHO_ENDIAN_BIG_WORD && \
    !BHO_ENDIAN_LITTLE_BYTE && !BHO_ENDIAN_LITTLE_WORD
#   if (defined(__BIG_ENDIAN__) && !defined(__LITTLE_ENDIAN__)) || \
       (defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)) || \
        defined(__ARMEB__) || \
        defined(__THUMBEB__) || \
        defined(__AARCH64EB__) || \
        defined(_MIPSEB) || \
        defined(__MIPSEB) || \
        defined(__MIPSEB__)
#       undef BHO_ENDIAN_BIG_BYTE
#       define BHO_ENDIAN_BIG_BYTE BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

/* Built-in byte-swapped little-endian macros.
 */
#if !BHO_ENDIAN_BIG_BYTE && !BHO_ENDIAN_BIG_WORD && \
    !BHO_ENDIAN_LITTLE_BYTE && !BHO_ENDIAN_LITTLE_WORD
#   if (defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)) || \
       (defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)) || \
        defined(__ARMEL__) || \
        defined(__THUMBEL__) || \
        defined(__AARCH64EL__) || \
        defined(__loongarch__) || \
        defined(_MIPSEL) || \
        defined(__MIPSEL) || \
        defined(__MIPSEL__) || \
        defined(__riscv) || \
        defined(__e2k__)
#       undef BHO_ENDIAN_LITTLE_BYTE
#       define BHO_ENDIAN_LITTLE_BYTE BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

/* Some architectures are strictly one endianess (as opposed
 * the current common bi-endianess).
 */
#if !BHO_ENDIAN_BIG_BYTE && !BHO_ENDIAN_BIG_WORD && \
    !BHO_ENDIAN_LITTLE_BYTE && !BHO_ENDIAN_LITTLE_WORD
#   include <asio2/bho/predef/architecture.h>
#   if BHO_ARCH_M68K || \
        BHO_ARCH_PARISC || \
        BHO_ARCH_SPARC || \
        BHO_ARCH_SYS370 || \
        BHO_ARCH_SYS390 || \
        BHO_ARCH_Z
#       undef BHO_ENDIAN_BIG_BYTE
#       define BHO_ENDIAN_BIG_BYTE BHO_VERSION_NUMBER_AVAILABLE
#   endif
#   if BHO_ARCH_IA64 || \
        BHO_ARCH_X86 || \
        BHO_ARCH_BLACKFIN
#       undef BHO_ENDIAN_LITTLE_BYTE
#       define BHO_ENDIAN_LITTLE_BYTE BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

/* Windows on ARM, if not otherwise detected/specified, is always
 * byte-swapped little-endian.
 */
#if !BHO_ENDIAN_BIG_BYTE && !BHO_ENDIAN_BIG_WORD && \
    !BHO_ENDIAN_LITTLE_BYTE && !BHO_ENDIAN_LITTLE_WORD
#   if BHO_ARCH_ARM
#       include <asio2/bho/predef/os/windows.h>
#       if BHO_OS_WINDOWS
#           undef BHO_ENDIAN_LITTLE_BYTE
#           define BHO_ENDIAN_LITTLE_BYTE BHO_VERSION_NUMBER_AVAILABLE
#       endif
#   endif
#endif

#if BHO_ENDIAN_BIG_BYTE
#   define BHO_ENDIAN_BIG_BYTE_AVAILABLE
#endif
#if BHO_ENDIAN_BIG_WORD
#   define BHO_ENDIAN_BIG_WORD_BYTE_AVAILABLE
#endif
#if BHO_ENDIAN_LITTLE_BYTE
#   define BHO_ENDIAN_LITTLE_BYTE_AVAILABLE
#endif
#if BHO_ENDIAN_LITTLE_WORD
#   define BHO_ENDIAN_LITTLE_WORD_BYTE_AVAILABLE
#endif

#define BHO_ENDIAN_BIG_BYTE_NAME "Byte-Swapped Big-Endian"
#define BHO_ENDIAN_BIG_WORD_NAME "Word-Swapped Big-Endian"
#define BHO_ENDIAN_LITTLE_BYTE_NAME "Byte-Swapped Little-Endian"
#define BHO_ENDIAN_LITTLE_WORD_NAME "Word-Swapped Little-Endian"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_ENDIAN_BIG_BYTE,BHO_ENDIAN_BIG_BYTE_NAME)

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_ENDIAN_BIG_WORD,BHO_ENDIAN_BIG_WORD_NAME)

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_ENDIAN_LITTLE_BYTE,BHO_ENDIAN_LITTLE_BYTE_NAME)

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_ENDIAN_LITTLE_WORD,BHO_ENDIAN_LITTLE_WORD_NAME)
