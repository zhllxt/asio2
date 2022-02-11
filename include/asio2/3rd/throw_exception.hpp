/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_THROW_EXCEPTION_HPP__
#define __ASIO2_THROW_EXCEPTION_HPP__

#include <asio2/config.hpp>

#if __has_include(<boost/throw_exception.hpp>)
#include <boost/throw_exception.hpp>
#  ifndef BHO_THROW_EXCEPTION
#    define BHO_THROW_EXCEPTION BOOST_THROW_EXCEPTION
#  endif
#else
#include <asio2/bho/throw_exception.hpp>
#endif

#endif
