/*
 * modify by zhllxt
 */

#ifndef __ASIO2_OPENSSLCONF_H__
#define __ASIO2_OPENSSLCONF_H__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/bho/predef.h>

#if   defined(BHO_OS_WINDOWS) && (BHO_ARCH_WORD_BITS == 64)
#	include <openssl/opensslconf_win64.h>
#elif defined(BHO_OS_WINDOWS) && (BHO_ARCH_WORD_BITS == 32)
#	include <openssl/opensslconf_win32.h>

#elif defined(BHO_OS_LINUX  ) && (BHO_ARCH_WORD_BITS == 64)
#	include <openssl/opensslconf_linux64.h>
#elif defined(BHO_OS_LINUX  ) && (BHO_ARCH_WORD_BITS == 32)
#	include <openssl/opensslconf_linux32.h>

#elif defined(BHO_OS_MACOS  ) && (BHO_ARCH_WORD_BITS == 64)
#	include <openssl/opensslconf_linux64.h> // May be incorrect
#elif defined(BHO_OS_MACOS  ) && (BHO_ARCH_WORD_BITS == 32)
#	include <openssl/opensslconf_linux32.h> // May be incorrect

#elif defined(BHO_OS_IOS    ) && (BHO_ARCH_WORD_BITS == 64)
#	include <openssl/opensslconf_linux64.h> // May be incorrect
#elif defined(BHO_OS_IOS    ) && (BHO_ARCH_WORD_BITS == 32)
#	include <openssl/opensslconf_linux32.h> // May be incorrect

#elif defined(BHO_OS_UNIX   ) && (BHO_ARCH_WORD_BITS == 64)
#	include <openssl/opensslconf_linux64.h> // May be incorrect
#elif defined(BHO_OS_UNIX   ) && (BHO_ARCH_WORD_BITS == 32)
#	include <openssl/opensslconf_linux32.h> // May be incorrect

#elif (BHO_ARCH_WORD_BITS == 64)
#	include <openssl/opensslconf_linux64.h> // May be incorrect
#elif (BHO_ARCH_WORD_BITS == 32)
#	include <openssl/opensslconf_linux32.h> // May be incorrect

#endif


#endif // !__ASIO2_OPENSSLCONF_H__
