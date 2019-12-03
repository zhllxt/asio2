/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 * 
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_CONFIG_HPP__
#define __ASIO2_CONFIG_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

// If you don't want to use boost in your project,you need to define ASIO_STANDALONE.
// But you can't use http and websocket at the same time.If you want to use http and 
// websocket, you must use boost, and then you have to turn ASIO_STANDALONE off.
#define ASIO_STANDALONE

// If you want to use the ssl, you need to define ASIO2_USE_SSL.
// When use ssl,on windows need linker "libssl.lib;libcrypto.lib;Crypt32.lib;", on 
// linux need linker "ssl;crypto;"
//#define ASIO2_USE_SSL


// the tests trigger deprecation warnings when compiled with msvc in C++17 mode
#if defined(_MSVC_LANG) && _MSVC_LANG > 201402
// warning STL4009: std::allocator<void> is deprecated in C++17
#define _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
#define _SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING
//#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#endif

#ifdef ASIO_STANDALONE
#define ASIO_HEADER_ONLY
#endif

#endif // !__ASIO2_CONFIG_HPP__
