//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BHO_BEAST_VERSION_HPP
#define BHO_BEAST_VERSION_HPP

#include <asio2/bho/beast/core/detail/config.hpp>
#include <asio2/bho/config.hpp>

//[version

/* Identifies the API version of Beast.

   This is a simple integer that is incremented by one every
   time a set of code changes is merged to the develop branch.
*/
#define BHO_BEAST_VERSION 351

// A string describing BHO_BEAST_VERSION, that can be used in http headers.
#define BHO_BEAST_VERSION_STRING "BHO.Beast/" BHO_STRINGIZE(BHO_BEAST_VERSION)

//]

#endif

