//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BEAST_VERSION_HPP
#define BEAST_VERSION_HPP

#include <beast/core/detail/config.hpp>

/*  BEAST_VERSION

    Identifies the API version of Beast.

    This is a simple integer that is incremented by one every
    time a set of code changes is merged to the develop branch.
*/
#define BEAST_VERSION 290

#define BEAST_VERSION_STRING "Boost.Beast/" BEAST_STRINGIZE(BEAST_VERSION)

#ifndef BOOST_BEAST_VERSION
#  define BOOST_BEAST_VERSION BEAST_VERSION
#endif

#ifndef BOOST_BEAST_VERSION_STRING
#  define BOOST_BEAST_VERSION_STRING BEAST_VERSION_STRING
#endif

#endif

