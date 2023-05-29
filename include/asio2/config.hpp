/*
 * Copyright (c) 2017-2023 zhllxt
 * 
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
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

// define this to use asio standalone and beast standalone, otherwise use boost::asio and boost::beast.
#define ASIO2_HEADER_ONLY

// If you want to use the ssl, you need to define ASIO2_ENABLE_SSL.
// When use ssl,on windows need linker "libssl.lib;libcrypto.lib;Crypt32.lib;", on 
// linux need linker "ssl;crypto;", if link failed under gcc, try ":libssl.a;:libcrypto.a;"
// ssl must be before crypto.
//#define ASIO2_ENABLE_SSL

// RPC component is using tcp dgram mode as the underlying communication support by default,
// If you want to using websocket as the underlying communication support, turn on this macro.
//#define ASIO2_USE_WEBSOCKET_RPC

// Whether to detect the validity of UTF8 string of mqtt
#define ASIO2_CHECK_UTF8

// Whether called the timer callback when the timer is awaked with some error.
// eg : stop_timer will awake the timer with error of asio::error::operation_aborted.
//#define ASIO2_ENABLE_TIMER_CALLBACK_WHEN_ERROR

// Define ASIO_NO_EXCEPTIONS to disable the exception. so when the exception occurs, you can
// check the stack trace.
// If the ASIO_NO_EXCEPTIONS is defined, you can impl the throw_exception function by youself,
// see: /asio2/external/asio.hpp
//#define ASIO_NO_EXCEPTIONS

#endif // !__ASIO2_CONFIG_HPP__
