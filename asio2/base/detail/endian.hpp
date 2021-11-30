/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * refrenced from : boost/predef/other/endian.h
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_ENDIAN_HPP__
#define __ASIO2_ENDIAN_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)


#ifdef ASIO2_BIG_ENDIAN
	static_assert(false, "Unknown ASIO2_BIG_ENDIAN definition will affect the relevant functions of this program.");
#else
	#define ASIO2_BIG_ENDIAN 0
#endif


#if !ASIO2_BIG_ENDIAN
#   if (defined(__GLIBC__) || defined(__GNU_LIBRARY__)) || defined(__ANDROID__)
#        if __has_include(<endian.h>)
#            include <endian.h>
#        endif
#   else
#       if (defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__)))
#           if __has_include(<machine/endian.h>)
#               include <machine/endian.h>
#           endif
#       else
#           if (defined(BSD) || defined(_SYSTYPE_BSD))
#               if defined(__OpenBSD__)
#                   if __has_include(<machine/endian.h>)
#                       include <machine/endian.h>
#                   endif
#               else
#                   if __has_include(<sys/endian.h>)
#                       include <sys/endian.h>
#                   endif
#               endif
#           endif
#       endif
#   endif
#   if defined(__BYTE_ORDER)
#       if defined(__BIG_ENDIAN) && (__BYTE_ORDER == __BIG_ENDIAN)
#           undef  ASIO2_BIG_ENDIAN
#           define ASIO2_BIG_ENDIAN 1
#       endif
#   endif
#   if !defined(__BYTE_ORDER) && defined(_BYTE_ORDER)
#       if defined(_BIG_ENDIAN) && (_BYTE_ORDER == _BIG_ENDIAN)
#           undef  ASIO2_BIG_ENDIAN
#           define ASIO2_BIG_ENDIAN 1
#       endif
#   endif
#endif

/* Built-in byte-swpped big-endian macros.
 */
#if !ASIO2_BIG_ENDIAN
#   if (defined(__BIG_ENDIAN__) && !defined(__LITTLE_ENDIAN__)) || \
       (defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)) || \
        defined(__ARMEB__) || \
        defined(__THUMBEB__) || \
        defined(__AARCH64EB__) || \
        defined(_MIPSEB) || \
        defined(__MIPSEB) || \
        defined(__MIPSEB__)
#       undef  ASIO2_BIG_ENDIAN
#       define ASIO2_BIG_ENDIAN 1
#   endif
#endif

/* Some architectures are strictly one endianess (as opposed
 * the current common bi-endianess).
 */
#if !ASIO2_BIG_ENDIAN
#   if  (defined(__m68k__) || defined(M68000)) || \
        (defined(__hppa__) || defined(__hppa) || defined(__HPPA__)) || \
        (defined(__sparc__) || defined(__sparc)) || \
        (defined(__370__) || defined(__THW_370__)) || \
        (defined(__s390__) || defined(__s390x__)) || \
        defined(__SYSC_ZARCH__)
#       undef  ASIO2_BIG_ENDIAN
#       define ASIO2_BIG_ENDIAN 1
#   endif
#endif

#if !ASIO2_BIG_ENDIAN
#    ifdef _BIG_ENDIAN_
#        if _BIG_ENDIAN_
#            undef  ASIO2_BIG_ENDIAN
#            define ASIO2_BIG_ENDIAN 1
#        endif
#    endif
#endif

#if !ASIO2_BIG_ENDIAN
#    if  defined(__hppa__) || \
         defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
         (defined(__MIPS__) && defined(__MIPSEB__)) || \
         defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
         defined(__sparc__) || defined(__powerpc__) || \
         defined(__mc68000__) || defined(__s390x__) || defined(__s390__)
#        undef  ASIO2_BIG_ENDIAN
#        define ASIO2_BIG_ENDIAN 1
#    endif
#endif

#ifdef ASIO2_LITTLE_ENDIAN
	static_assert(false, "Unknown ASIO2_LITTLE_ENDIAN definition will affect the relevant functions of this program.");
#else
	#define ASIO2_LITTLE_ENDIAN (!ASIO2_BIG_ENDIAN)
#endif


#endif // !__ASIO2_ENDIAN_HPP__
