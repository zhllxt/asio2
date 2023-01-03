/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_EXTERNAL_CONFIG_HPP__
#define __ASIO2_EXTERNAL_CONFIG_HPP__

#include <asio2/config.hpp>

#if !defined(ASIO2_HEADER_ONLY) && __has_include(<boost/config.hpp>)
#include <boost/config.hpp>
#ifndef ASIO2_JOIN
#define ASIO2_JOIN BOOST_JOIN
#endif
#ifndef ASIO2_STRINGIZE
#define ASIO2_STRINGIZE BOOST_STRINGIZE
#endif
#else
#include <asio2/bho/config.hpp>
#ifndef ASIO2_JOIN
#define ASIO2_JOIN BHO_JOIN
#endif
#ifndef ASIO2_STRINGIZE
#define ASIO2_STRINGIZE BHO_STRINGIZE
#endif
#endif

#endif
