/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_EXTERNAL_ASSERT_HPP__
#define __ASIO2_EXTERNAL_ASSERT_HPP__

#include <asio2/config.hpp>

#if !defined(ASIO2_HEADER_ONLY) && __has_include(<boost/assert.hpp>)
#include <boost/assert.hpp>
#ifndef ASIO2_ASSERT
#define ASIO2_ASSERT BOOST_ASSERT
#endif
#else
#include <asio2/bho/assert.hpp>
#ifndef ASIO2_ASSERT
#define ASIO2_ASSERT BHO_ASSERT
#endif
#endif

#endif
