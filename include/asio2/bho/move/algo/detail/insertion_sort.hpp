//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2014-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/move for documentation.
//
//////////////////////////////////////////////////////////////////////////////

//! \file

#ifndef BHO_MOVE_DETAIL_INSERT_SORT_HPP
#define BHO_MOVE_DETAIL_INSERT_SORT_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif
#
#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/move/utility_core.hpp>
#include <asio2/bho/move/algo/move.hpp>
#include <asio2/bho/move/detail/iterator_traits.hpp>
#include <asio2/bho/move/adl_move_swap.hpp>
#include <asio2/bho/move/utility_core.hpp>
#include <asio2/bho/move/detail/placement_new.hpp>
#include <asio2/bho/move/detail/destruct_n.hpp>
#include <asio2/bho/move/algo/detail/basic_op.hpp>
#include <asio2/bho/move/detail/placement_new.hpp>
#include <asio2/bho/move/detail/iterator_to_raw_pointer.hpp>

#if defined(BHO_CLANG) || (defined(BHO_GCC) && (BHO_GCC >= 40600))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

namespace bho {  namespace movelib{

// @cond

template <class Compare, class ForwardIterator, class BirdirectionalIterator, class Op>
void insertion_sort_op(ForwardIterator first1, ForwardIterator last1, BirdirectionalIterator first2, Compare comp, Op op)
{
   if (first1 != last1){
      BirdirectionalIterator last2 = first2;
      op(first1, last2);
      for (++last2; ++first1 != last1; ++last2){
         BirdirectionalIterator j2 = last2;
         BirdirectionalIterator i2 = j2;
         if (comp(*first1, *--i2)){
            op(i2, j2);
            for (--j2; i2 != first2 && comp(*first1, *--i2); --j2) {
               op(i2, j2);
            }
         }
         op(first1, j2);
      }
   }
}

template <class Compare, class ForwardIterator, class BirdirectionalIterator>
void insertion_sort_swap(ForwardIterator first1, ForwardIterator last1, BirdirectionalIterator first2, Compare comp)
{
   insertion_sort_op(first1, last1, first2, comp, swap_op());
}


template <class Compare, class ForwardIterator, class BirdirectionalIterator>
void insertion_sort_copy(ForwardIterator first1, ForwardIterator last1, BirdirectionalIterator first2, Compare comp)
{
   insertion_sort_op(first1, last1, first2, comp, move_op());
}

// @endcond

template <class Compare, class BirdirectionalIterator>
void insertion_sort(BirdirectionalIterator first, BirdirectionalIterator last, Compare comp)
{
   typedef typename bho::movelib::iterator_traits<BirdirectionalIterator>::value_type value_type;
   if (first != last){
      BirdirectionalIterator i = first;
      for (++i; i != last; ++i){
         BirdirectionalIterator j = i;
         if (comp(*i,  *--j)) {
            value_type tmp(::bho::move(*i));
            *i = ::bho::move(*j);
            for (BirdirectionalIterator k = j; k != first && comp(tmp,  *--k); --j) {
               *j = ::bho::move(*k);
            }
            *j = ::bho::move(tmp);
         }
      }
   }
}

template <class Compare, class BirdirectionalIterator, class BirdirectionalRawIterator>
void insertion_sort_uninitialized_copy
   (BirdirectionalIterator first1, BirdirectionalIterator const last1
   , BirdirectionalRawIterator const first2
   , Compare comp)
{
   typedef typename iterator_traits<BirdirectionalIterator>::value_type value_type;
   if (first1 != last1){
      BirdirectionalRawIterator last2 = first2;
      ::new((iterator_to_raw_pointer)(last2), bho_move_new_t()) value_type(::bho::move(*first1));
      destruct_n<value_type, BirdirectionalRawIterator> d(first2);
      d.incr();
      for (++last2; ++first1 != last1; ++last2){
         BirdirectionalRawIterator j2 = last2;
         BirdirectionalRawIterator k2 = j2;
         if (comp(*first1, *--k2)){
            ::new((iterator_to_raw_pointer)(j2), bho_move_new_t()) value_type(::bho::move(*k2));
            d.incr();
            for (--j2; k2 != first2 && comp(*first1, *--k2); --j2)
               *j2 = ::bho::move(*k2);
            *j2 = ::bho::move(*first1);
         }
         else{
            ::new((iterator_to_raw_pointer)(j2), bho_move_new_t()) value_type(::bho::move(*first1));
            d.incr();
         }
      }
      d.release();
   }
}

}} //namespace bho {  namespace movelib{

#if defined(BHO_CLANG) || (defined(BHO_GCC) && (BHO_GCC >= 40600))
#pragma GCC diagnostic pop
#endif

#endif //#ifndef BHO_MOVE_DETAIL_INSERT_SORT_HPP
