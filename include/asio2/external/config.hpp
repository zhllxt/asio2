/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
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
