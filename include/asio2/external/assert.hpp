/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_ASSERT_HPP__
#define __ASIO2_ASSERT_HPP__

#include <asio2/config.hpp>

#if !defined(ASIO2_DISABLE_BOOST) && __has_include(<boost/assert.hpp>)
#  include <boost/assert.hpp>
#  ifndef BHO_ASSERT
#    define BHO_ASSERT BOOST_ASSERT
#  endif
#  ifndef BHO_ASSERT_MSG
#    define BHO_ASSERT_MSG BOOST_ASSERT_MSG
#  endif
#  ifndef BHO_ASSERT_IS_VOID
#    define BHO_ASSERT_IS_VOID BOOST_ASSERT_IS_VOID
#  endif
#  ifndef BHO_VERIFY
#    define BHO_VERIFY BOOST_VERIFY
#  endif
#  ifndef BHO_VERIFY_MSG
#    define BHO_VERIFY_MSG BOOST_VERIFY_MSG
#  endif
#else
#  include <asio2/bho/assert.hpp>
#endif

#endif
