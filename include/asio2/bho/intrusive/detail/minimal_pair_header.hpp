/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga  2014-2015
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////
#ifndef BHO_INTRUSIVE_DETAIL_MINIMAL_PAIR_HEADER_HPP
#define BHO_INTRUSIVE_DETAIL_MINIMAL_PAIR_HEADER_HPP
#
#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif
#
#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif
#
#//Try to avoid including <utility>, as it's quite big in C++11
#if defined(BHO_GNU_STDLIB)
#  include <bits/stl_pair.h>
#else
#  include <utility>    //Fallback
#endif
#
#endif //BHO_INTRUSIVE_DETAIL_MINIMAL_PAIR_HEADER_HPP
