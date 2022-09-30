/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_EXTERNAL_PREDEF_H__
#define __ASIO2_EXTERNAL_PREDEF_H__

#include <asio2/config.hpp>

#if !defined(ASIO2_HEADER_ONLY) && __has_include(<boost/predef.h>)
#include <boost/predef.h>

#ifndef ASIO2_OS_IOS
#define ASIO2_OS_IOS   BOOST_OS_IOS
#endif
#ifndef ASIO2_OS_LINUX
#define ASIO2_OS_LINUX BOOST_OS_LINUX
#endif
#ifndef ASIO2_OS_MACOS
#define ASIO2_OS_MACOS BOOST_OS_MACOS
#endif
#ifndef ASIO2_OS_UNIX
#define ASIO2_OS_UNIX  BOOST_OS_UNIX
#endif
#ifndef ASIO2_OS_WINDOWS
#define ASIO2_OS_WINDOWS BOOST_OS_WINDOWS
#endif

#ifndef ASIO2_ENDIAN_BIG_BYTE
#define ASIO2_ENDIAN_BIG_BYTE BOOST_ENDIAN_BIG_BYTE
#endif
#ifndef ASIO2_ENDIAN_BIG_WORD
#define ASIO2_ENDIAN_BIG_WORD BOOST_ENDIAN_BIG_WORD
#endif
#ifndef ASIO2_ENDIAN_LITTLE_BYTE
#define ASIO2_ENDIAN_LITTLE_BYTE BOOST_ENDIAN_LITTLE_BYTE
#endif
#ifndef ASIO2_ENDIAN_LITTLE_WORD
#define ASIO2_ENDIAN_LITTLE_WORD BOOST_ENDIAN_LITTLE_WORD
#endif
#else
#include <asio2/bho/predef.h>

#ifndef ASIO2_OS_IOS
#define ASIO2_OS_IOS   BHO_OS_IOS
#endif
#ifndef ASIO2_OS_LINUX
#define ASIO2_OS_LINUX BHO_OS_LINUX
#endif
#ifndef ASIO2_OS_MACOS
#define ASIO2_OS_MACOS BHO_OS_MACOS
#endif
#ifndef ASIO2_OS_UNIX
#define ASIO2_OS_UNIX  BHO_OS_UNIX
#endif
#ifndef ASIO2_OS_WINDOWS
#define ASIO2_OS_WINDOWS BHO_OS_WINDOWS
#endif

#ifndef ASIO2_ENDIAN_BIG_BYTE
#define ASIO2_ENDIAN_BIG_BYTE BHO_ENDIAN_BIG_BYTE
#endif
#ifndef ASIO2_ENDIAN_BIG_WORD
#define ASIO2_ENDIAN_BIG_WORD BHO_ENDIAN_BIG_WORD
#endif
#ifndef ASIO2_ENDIAN_LITTLE_BYTE
#define ASIO2_ENDIAN_LITTLE_BYTE BHO_ENDIAN_LITTLE_BYTE
#endif
#ifndef ASIO2_ENDIAN_LITTLE_WORD
#define ASIO2_ENDIAN_LITTLE_WORD BHO_ENDIAN_LITTLE_WORD
#endif
#endif

#endif
