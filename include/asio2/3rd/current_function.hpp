/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_CURRENT_FUNCTION_HPP__
#define __ASIO2_CURRENT_FUNCTION_HPP__

#include <asio2/config.hpp>

#if __has_include(<boost/current_function.hpp>)
#include <boost/current_function.hpp>
#  ifndef BHO_CURRENT_FUNCTION
#    define BHO_CURRENT_FUNCTION BOOST_CURRENT_FUNCTION
#  endif
#else
#include <asio2/bho/current_function.hpp>
#endif

#endif
