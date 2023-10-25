//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2014-2015. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BHO_MOVE_DETAIL_FORCE_CAST_HPP
#define BHO_MOVE_DETAIL_FORCE_CAST_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/move/detail/workaround.hpp>

namespace bho {
namespace move_detail {


template <typename T>
BHO_MOVE_FORCEINLINE T force_ptr(const volatile void *p)
{
   return static_cast<T>(const_cast<void*>(p));
}

}  //namespace move_detail {
}  //namespace bho {

#endif   //#ifndef BHO_MOVE_DETAIL_FORCE_CAST_HPP
