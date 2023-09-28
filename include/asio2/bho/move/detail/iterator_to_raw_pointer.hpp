//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2014-2015. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BHO_MOVE_DETAIL_ITERATOR_TO_RAW_POINTER_HPP
#define BHO_MOVE_DETAIL_ITERATOR_TO_RAW_POINTER_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/move/detail/iterator_traits.hpp>
#include <asio2/bho/move/detail/to_raw_pointer.hpp>
#include <asio2/bho/move/detail/pointer_element.hpp>

namespace bho {
namespace movelib {
namespace detail {

template <class T>
BHO_MOVE_FORCEINLINE T* iterator_to_pointer(T* i)
{  return i; }

template <class Iterator>
BHO_MOVE_FORCEINLINE typename bho::movelib::iterator_traits<Iterator>::pointer
   iterator_to_pointer(const Iterator &i)
{  return i.operator->();  }

template <class Iterator>
struct iterator_to_element_ptr
{
   typedef typename bho::movelib::iterator_traits<Iterator>::pointer  pointer;
   typedef typename bho::movelib::pointer_element<pointer>::type      element_type;
   typedef element_type* type;
};

}  //namespace detail {

template <class Iterator>
BHO_MOVE_FORCEINLINE typename bho::movelib::detail::iterator_to_element_ptr<Iterator>::type
   iterator_to_raw_pointer(const Iterator &i)
{
   return ::bho::movelib::to_raw_pointer
      (  ::bho::movelib::detail::iterator_to_pointer(i)   );
}

}  //namespace movelib {
}  //namespace bho {

#endif   //#ifndef BHO_MOVE_DETAIL_ITERATOR_TO_RAW_POINTER_HPP
