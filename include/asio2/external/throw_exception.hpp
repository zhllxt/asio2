/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_EXTERNAL_THROW_EXCEPTION_HPP__
#define __ASIO2_EXTERNAL_THROW_EXCEPTION_HPP__

#include <asio2/config.hpp>

#if !defined(ASIO2_HEADER_ONLY) && __has_include(<boost/throw_exception.hpp>)
#include <boost/throw_exception.hpp>
#ifndef ASIO2_THROW_EXCEPTION
#define ASIO2_THROW_EXCEPTION BOOST_THROW_EXCEPTION
#endif
#else
#include <asio2/bho/throw_exception.hpp>
#ifndef ASIO2_THROW_EXCEPTION
#define ASIO2_THROW_EXCEPTION BHO_THROW_EXCEPTION
#endif
#endif

#endif
