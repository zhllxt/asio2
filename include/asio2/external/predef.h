/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_PREDEF_HPP__
#define __ASIO2_PREDEF_HPP__

#include <asio2/config.hpp>

#if !defined(ASIO2_DISABLE_BOOST) && __has_include(<boost/predef.h>)
#include <boost/predef.h>

#  ifndef BHO_ENDIAN_BIG_BYTE
#    define BHO_ENDIAN_BIG_BYTE BOOST_ENDIAN_BIG_BYTE
#  endif
#  ifndef BHO_ENDIAN_BIG_WORD
#    define BHO_ENDIAN_BIG_WORD BOOST_ENDIAN_BIG_WORD
#  endif
#  ifndef BHO_ENDIAN_LITTLE_BYTE
#    define BHO_ENDIAN_LITTLE_BYTE BOOST_ENDIAN_LITTLE_BYTE
#  endif
#  ifndef BHO_ENDIAN_LITTLE_WORD
#    define BHO_ENDIAN_LITTLE_WORD BOOST_ENDIAN_LITTLE_WORD
#  endif

#  ifndef BHO_OS_WINDOWS
#    define BHO_OS_WINDOWS BOOST_OS_WINDOWS
#  endif
#  ifndef BHO_OS_VMS
#    define BHO_OS_VMS BOOST_OS_VMS
#  endif
#  ifndef BHO_OS_UNIX
#    define BHO_OS_UNIX BOOST_OS_UNIX
#  endif
#  ifndef BHO_OS_SOLARIS
#    define BHO_OS_SOLARIS BOOST_OS_SOLARIS
#  endif
#  ifndef BHO_OS_QNX
#    define BHO_OS_QNX BOOST_OS_QNX
#  endif
#  ifndef BHO_OS_OS400
#    define BHO_OS_OS400 BOOST_OS_OS400
#  endif
#  ifndef BHO_OS_MACOS
#    define BHO_OS_MACOS BOOST_OS_MACOS
#  endif
#  ifndef BHO_OS_LINUX
#    define BHO_OS_LINUX BOOST_OS_LINUX
#  endif
#  ifndef BHO_OS_IOS
#    define BHO_OS_IOS BOOST_OS_IOS
#  endif
#  ifndef BHO_OS_IRIX
#    define BHO_OS_IRIX BOOST_OS_IRIX
#  endif
#  ifndef BHO_OS_HPUX
#    define BHO_OS_HPUX BOOST_OS_HPUX
#  endif
#  ifndef BHO_OS_HAIKU
#    define BHO_OS_HAIKU BOOST_OS_HAIKU
#  endif
#  ifndef BHO_OS_CYGWIN
#    define BHO_OS_CYGWIN BOOST_OS_CYGWIN
#  endif
#  ifndef BHO_OS_BSD
#    define BHO_OS_BSD BOOST_OS_BSD
#  endif
#  ifndef BHO_OS_BEOS
#    define BHO_OS_BEOS BOOST_OS_BEOS
#  endif
#  ifndef BHO_OS_AMIGAOS
#    define BHO_OS_AMIGAOS BOOST_OS_AMIGAOS
#  endif
#  ifndef BHO_OS_AIX
#    define BHO_OS_AIX BOOST_OS_AIX
#  endif
#else
#include <asio2/bho/predef.h>
#endif

#endif
