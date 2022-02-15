/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_LIMITS_FUNCTION_HPP__
#define __ASIO2_LIMITS_FUNCTION_HPP__

#include <asio2/config.hpp>

#if !defined(ASIO2_DISABLE_BOOST) && __has_include(<boost/limits.hpp>)
#include <boost/limits.hpp>
#else
#include <asio2/bho/limits.hpp>
#endif

#endif
