#ifndef BHO_MOVE_DETAIL_IS_SORTED_HPP
#define BHO_MOVE_DETAIL_IS_SORTED_HPP
///////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2017-2018. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

namespace bho {
namespace movelib {

template<class ForwardIt, class Pred>
bool is_sorted(ForwardIt const first, ForwardIt last, Pred pred)
{
   if (first != last) {
      ForwardIt next = first, cur(first);
      while (++next != last) {
         if (pred(*next, *cur))
            return false;
         cur = next;
      }
   }
   return true;
}

template<class ForwardIt, class Pred>
bool is_sorted_and_unique(ForwardIt first, ForwardIt last, Pred pred)
{
   if (first != last) {
      ForwardIt next = first;
      while (++next != last) {
         if (!pred(*first, *next))
            return false;
         first = next;
      }
   }
   return true;
}

}  //namespace movelib {
}  //namespace bho {

#endif   //BHO_MOVE_DETAIL_IS_SORTED_HPP
