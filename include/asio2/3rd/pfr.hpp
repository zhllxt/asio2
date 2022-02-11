/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_PFR_FUNCTION_HPP__
#define __ASIO2_PFR_FUNCTION_HPP__

#include <asio2/config.hpp>

#if __has_include(<boost/pfr.hpp>)
#include <boost/pfr.hpp>
namespace pfr = ::boost::pfr;
#else
#include <asio2/bho/pfr.hpp>
namespace pfr = ::bho::pfr;
#endif

#endif
