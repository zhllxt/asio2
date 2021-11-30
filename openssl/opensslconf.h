/*
 * modify by zhllxt
 */

#ifndef __ASIO2_OPENSSLCONF_H__
#define __ASIO2_OPENSSLCONF_H__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)


#if   defined(_WIN64) || defined(__WIN64__) || defined(WIN64)
#	include <openssl/opensslconf_win64.h>
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#	include <openssl/opensslconf_win32.h>
#elif (defined(linux) || defined(__linux) || defined(__linux__) || defined(__GNU__) || defined(__GLIBC__)) && !defined(_CRAYC) && defined(__x86_64__)
#	include <openssl/opensslconf_linux64.h>
#elif (defined(linux) || defined(__linux) || defined(__linux__) || defined(__GNU__) || defined(__GLIBC__)) && !defined(_CRAYC) && defined(__i386__)
#	include <openssl/opensslconf_linux32.h>
#elif (defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)) && defined(__arm__)
//#    define EMBTC_IOSARM 1
//#    define EMBTC_IOS 1
#elif (defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)) && defined(__aarch64__)
//#    define EMBTC_IOSARM64 1
//#    define EMBTC_IOS 1
#elif defined(__ANDROID__) && defined(__arm__)
//#    define EMBTC_AARM 1
//#    define EMBTC_ANDROID 1
#elif defined(__arm__)
//#    define EMBTC_AARM 1
//#    define EMBTC_ANDROID 1
#endif


#endif // !__ASIO2_OPENSSLCONF_H__
