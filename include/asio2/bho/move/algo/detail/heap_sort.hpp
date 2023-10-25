//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2017-2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/move for documentation.
//
//////////////////////////////////////////////////////////////////////////////

//! \file

#ifndef BHO_MOVE_DETAIL_HEAP_SORT_HPP
#define BHO_MOVE_DETAIL_HEAP_SORT_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif
#
#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/move/detail/config_begin.hpp>

#include <asio2/bho/move/detail/workaround.hpp>
#include <asio2/bho/move/detail/iterator_traits.hpp>
#include <asio2/bho/move/algo/detail/is_sorted.hpp>
#include <asio2/bho/move/utility_core.hpp>
#include <cassert>

#if defined(BHO_CLANG) || (defined(BHO_GCC) && (BHO_GCC >= 40600))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

namespace bho {  namespace movelib{

template <class RandomAccessIterator, class Compare>
class heap_sort_helper
{
   typedef typename bho::movelib::iter_size<RandomAccessIterator>::type  size_type;
   typedef typename bho::movelib::iterator_traits<RandomAccessIterator>::value_type value_type;

   static void adjust_heap(RandomAccessIterator first, size_type hole_index, size_type const len, value_type &value, Compare comp)
   {
      size_type const top_index = hole_index;
      size_type second_child = size_type(2u*(hole_index + 1u));

      while (second_child < len) {
         if (comp(*(first + second_child), *(first + size_type(second_child - 1u))))
            second_child--;
         *(first + hole_index) = bho::move(*(first + second_child));
         hole_index = second_child;
         second_child = size_type(2u * (second_child + 1u));
      }
      if (second_child == len) {
         *(first + hole_index) = bho::move(*(first + size_type(second_child - 1u)));
         hole_index = size_type(second_child - 1);
      }

      {  //push_heap-like ending
         size_type parent = size_type((hole_index - 1u) / 2u);
         while (hole_index > top_index && comp(*(first + parent), value)) {
            *(first + hole_index) = bho::move(*(first + parent));
            hole_index = parent;
            parent = size_type((hole_index - 1u) / 2u);
         }    
         *(first + hole_index) = bho::move(value);
      }
   }

   static void make_heap(RandomAccessIterator first, RandomAccessIterator last, Compare comp)
   {
      size_type const len = size_type(last - first);
      if (len > 1) {
         size_type parent = size_type(len/2u - 1u);

         do {
            value_type v(bho::move(*(first + parent)));
            adjust_heap(first, parent, len, v, comp);
         }while (parent--);
      }
   }

   static void sort_heap(RandomAccessIterator first, RandomAccessIterator last, Compare comp)
   {
      size_type len = size_type(last - first);
      while (len > 1) {
         //move biggest to the safe zone
         --last;
         value_type v(bho::move(*last));
         *last = bho::move(*first);
         adjust_heap(first, size_type(0), --len, v, comp);
      }
   }

   public:
   static void sort(RandomAccessIterator first, RandomAccessIterator last, Compare comp)
   {
      make_heap(first, last, comp);
      sort_heap(first, last, comp);
      assert(bho::movelib::is_sorted(first, last, comp));
   }
};

template <class RandomAccessIterator, class Compare>
BHO_MOVE_FORCEINLINE void heap_sort(RandomAccessIterator first, RandomAccessIterator last, Compare comp)
{
   heap_sort_helper<RandomAccessIterator, Compare>::sort(first, last, comp);
}

}} //namespace bho {  namespace movelib{

#if defined(BHO_CLANG) || (defined(BHO_GCC) && (BHO_GCC >= 40600))
#pragma GCC diagnostic pop
#endif

#include <asio2/bho/move/detail/config_end.hpp>

#endif //#ifndef BHO_MOVE_DETAIL_HEAP_SORT_HPP
