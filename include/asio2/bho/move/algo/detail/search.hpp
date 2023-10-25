//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2022-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/move for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BHO_MOVE_DETAIL_SEARCH_HPP
#define BHO_MOVE_DETAIL_SEARCH_HPP

#include <asio2/bho/move/detail/iterator_traits.hpp>

#if defined(BHO_CLANG) || (defined(BHO_GCC) && (BHO_GCC >= 40600))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

namespace bho {
namespace movelib {

template <class RandIt, class T, class Compare>
RandIt lower_bound
   (RandIt first, const RandIt last, const T& key, Compare comp)
{
   typedef typename iter_size<RandIt>::type size_type;
   size_type len = size_type(last - first);
   RandIt middle;

   while (len) {
      size_type step = size_type(len >> 1);
      middle = first;
      middle += step;

      if (comp(*middle, key)) {
         first = ++middle;
         len = size_type(len - (step + 1));
      }
      else{
         len = step;
      }
   }
   return first;
}

template <class RandIt, class T, class Compare>
RandIt upper_bound
   (RandIt first, const RandIt last, const T& key, Compare comp)
{
   typedef typename iter_size<RandIt>::type size_type;
   size_type len = size_type(last - first);
   RandIt middle;

   while (len) {
      size_type step = size_type(len >> 1);
      middle = first;
      middle += step;

      if (!comp(key, *middle)) {
         first = ++middle;
         len = size_type(len - (step + 1));
      }
      else{
         len = step;
      }
   }
   return first;
}

}  //namespace movelib {
}  //namespace bho {

#if defined(BHO_CLANG) || (defined(BHO_GCC) && (BHO_GCC >= 40600))
#pragma GCC diagnostic pop
#endif

#endif   //#define BHO_MOVE_DETAIL_SEARCH_HPP
