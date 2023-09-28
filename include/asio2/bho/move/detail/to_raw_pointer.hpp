/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga  2017-2017
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/move for documentation.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef BHO_MOVE_DETAIL_TO_RAW_POINTER_HPP
#define BHO_MOVE_DETAIL_TO_RAW_POINTER_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/move/detail/config_begin.hpp>
#include <asio2/bho/move/detail/workaround.hpp>
#include <asio2/bho/move/detail/pointer_element.hpp>

namespace bho {
namespace movelib {

template <class T>
BHO_MOVE_FORCEINLINE T* to_raw_pointer(T* p)
{  return p; }

template <class Pointer>
BHO_MOVE_FORCEINLINE typename bho::movelib::pointer_element<Pointer>::type*
to_raw_pointer(const Pointer &p)
{  return ::bho::movelib::to_raw_pointer(p.operator->());  }

} //namespace movelib
} //namespace bho

#include <asio2/bho/move/detail/config_end.hpp>

#endif //BHO_MOVE_DETAIL_TO_RAW_POINTER_HPP
