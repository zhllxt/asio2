/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 * 
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_VERSION_HPP__
#define __ASIO2_VERSION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)


/** @def ASIO2_API_VERSION

    Identifies the API version of asio2.

    This is a simple integer that is incremented by one every
    time a set of code changes is merged to the develop branch.
*/

// ASIO2_VERSION / 100 is the major version
// ASIO2_VERSION % 100 is the minor version
#define ASIO2_VERSION 204 // 2.4



#endif // !__ASIO2_VERSION_HPP__
