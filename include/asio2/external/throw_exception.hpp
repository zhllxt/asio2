/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
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
