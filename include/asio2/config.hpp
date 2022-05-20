/*
 * COPYRIGHT (C) 2017-2021, zhllxt
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

// Note : Version 2.6 and 2.7 only supports asio standalone, does not support boost::asio, because
// boost::optional and other boost classes are used in boost::beast, while std::optional is
// used in beast standalone. to make the two compatible, it requires too much work. so,
// boost::asio is not supported for version 2.6 and 2.7.
// But at version 2.8, it supports both asio standalone and boost::asio.

// define ASIO_STANDALONE to use asio standalone, otherwise use boost::asio.
#define ASIO_STANDALONE

// define BEAST_HEADER_ONLY to use beast standalone, otherwise use boost::beast.
#define BEAST_HEADER_ONLY 1

// If you want to use the ssl, you need to define ASIO2_USE_SSL.
// When use ssl,on windows need linker "libssl.lib;libcrypto.lib;Crypt32.lib;", on 
// linux need linker "ssl;crypto;", if link failed under gcc, try ":libssl.a;:libcrypto.a;"
// ssl must be before crypto.
//#define ASIO2_USE_SSL

// RPC component is using tcp dgram mode as the underlying communication support by default,
// If you want to using websocket as the underlying communication support, turn on this macro.
//#define ASIO2_USE_WEBSOCKET_RPC

// Whether to detect the validity of UTF8 string of mqtt
#define ASIO2_CHECK_UTF8

// Whether called the timer callback when the timer is awaked with some error.
// eg : stop_timer will awake the timer with error of asio::error::operation_aborted.
//#define ASIO2_ENABLE_TIMER_CALLBACK_WHEN_ERROR

#endif // !__ASIO2_CONFIG_HPP__
