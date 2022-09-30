/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
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
