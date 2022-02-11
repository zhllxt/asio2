/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_STATIC_ASSERT_HPP__
#define __ASIO2_STATIC_ASSERT_HPP__

#include <asio2/config.hpp>

#if __has_include(<boost/static_assert.hpp>)
#include <boost/static_assert.hpp>
#  ifndef BHO_STATIC_ASSERT
#    define BHO_STATIC_ASSERT BOOST_STATIC_ASSERT
#  endif
#  ifndef BHO_STATIC_ASSERT_MSG
#    define BHO_STATIC_ASSERT_MSG BOOST_STATIC_ASSERT_MSG
#  endif
#else
#include <asio2/bho/static_assert.hpp>
#endif

#endif
