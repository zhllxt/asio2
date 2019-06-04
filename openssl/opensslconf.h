/*
 * modify by zhllxt
 */

#ifndef __ASIO2_OPENSSLCONF_H__
#define __ASIO2_OPENSSLCONF_H__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)


#if   defined(_WIN64)
#	include <openssl/opensslconf_win64.h>
#elif defined(_WIN32)
#	include <openssl/opensslconf_win32.h>
#elif defined(__x86_64__)
#	include <openssl/opensslconf_linux64.h>
#elif defined(__i386__)
#	include <openssl/opensslconf_linux32.h>
#endif


#endif // !__ASIO2_OPENSSLCONF_H__
