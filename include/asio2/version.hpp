/*
 * Copyright (c) 2017-2023 zhllxt
 * 
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
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
#define ASIO2_VERSION 208 // 2.8



#endif // !__ASIO2_VERSION_HPP__
