//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2015-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/move for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BHO_MOVE_ALGO_BASIC_OP
#define BHO_MOVE_ALGO_BASIC_OP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif
#
#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/move/utility_core.hpp>
#include <asio2/bho/move/adl_move_swap.hpp>
#include <asio2/bho/move/detail/iterator_traits.hpp>
#include <asio2/bho/move/algo/move.hpp>

namespace bho {
namespace movelib {

struct forward_t{};
struct backward_t{};
struct three_way_t{};
struct three_way_forward_t{};
struct four_way_t{};

struct move_op
{
   template <class SourceIt, class DestinationIt>
   BHO_MOVE_FORCEINLINE void operator()(SourceIt source, DestinationIt dest)
   {  *dest = ::bho::move(*source);  }

   template <class SourceIt, class DestinationIt>
   BHO_MOVE_FORCEINLINE DestinationIt operator()(forward_t, SourceIt first, SourceIt last, DestinationIt dest_begin)
   {  return ::bho::move(first, last, dest_begin);  }

   template <class SourceIt, class DestinationIt>
   BHO_MOVE_FORCEINLINE DestinationIt operator()(backward_t, SourceIt first, SourceIt last, DestinationIt dest_last)
   {  return ::bho::move_backward(first, last, dest_last);  }

   template <class SourceIt, class DestinationIt1, class DestinationIt2>
   BHO_MOVE_FORCEINLINE void operator()(three_way_t, SourceIt srcit, DestinationIt1 dest1it, DestinationIt2 dest2it)
   {
      *dest2it = bho::move(*dest1it);
      *dest1it = bho::move(*srcit);
   }

   template <class SourceIt, class DestinationIt1, class DestinationIt2>
   DestinationIt2 operator()(three_way_forward_t, SourceIt srcit, SourceIt srcitend, DestinationIt1 dest1it, DestinationIt2 dest2it)
   {
      //Destination2 range can overlap SourceIt range so avoid bho::move
      while(srcit != srcitend){
         this->operator()(three_way_t(), srcit++, dest1it++, dest2it++);
      }
      return dest2it;
   }

   template <class SourceIt, class DestinationIt1, class DestinationIt2, class DestinationIt3>
   BHO_MOVE_FORCEINLINE void operator()(four_way_t, SourceIt srcit, DestinationIt1 dest1it, DestinationIt2 dest2it, DestinationIt3 dest3it)
   {
      *dest3it = bho::move(*dest2it);
      *dest2it = bho::move(*dest1it);
      *dest1it = bho::move(*srcit);
   }
};

struct swap_op
{
   template <class SourceIt, class DestinationIt>
   BHO_MOVE_FORCEINLINE void operator()(SourceIt source, DestinationIt dest)
   {  bho::adl_move_swap(*dest, *source);  }

   template <class SourceIt, class DestinationIt>
   BHO_MOVE_FORCEINLINE DestinationIt operator()(forward_t, SourceIt first, SourceIt last, DestinationIt dest_begin)
   {  return bho::adl_move_swap_ranges(first, last, dest_begin);  }

   template <class SourceIt, class DestinationIt>
   BHO_MOVE_FORCEINLINE DestinationIt operator()(backward_t, SourceIt first, SourceIt last, DestinationIt dest_begin)
   {  return bho::adl_move_swap_ranges_backward(first, last, dest_begin);  }

   template <class SourceIt, class DestinationIt1, class DestinationIt2>
   BHO_MOVE_FORCEINLINE void operator()(three_way_t, SourceIt srcit, DestinationIt1 dest1it, DestinationIt2 dest2it)
   {
      typename ::bho::movelib::iterator_traits<SourceIt>::value_type tmp(bho::move(*dest2it));
      *dest2it = bho::move(*dest1it);
      *dest1it = bho::move(*srcit);
      *srcit = bho::move(tmp);
   }

   template <class SourceIt, class DestinationIt1, class DestinationIt2>
   DestinationIt2 operator()(three_way_forward_t, SourceIt srcit, SourceIt srcitend, DestinationIt1 dest1it, DestinationIt2 dest2it)
   {
      while(srcit != srcitend){
         this->operator()(three_way_t(), srcit++, dest1it++, dest2it++);
      }
      return dest2it;
   }

   template <class SourceIt, class DestinationIt1, class DestinationIt2, class DestinationIt3>
   BHO_MOVE_FORCEINLINE void operator()(four_way_t, SourceIt srcit, DestinationIt1 dest1it, DestinationIt2 dest2it, DestinationIt3 dest3it)
   {
      typename ::bho::movelib::iterator_traits<SourceIt>::value_type tmp(bho::move(*dest3it));
      *dest3it = bho::move(*dest2it);
      *dest2it = bho::move(*dest1it);
      *dest1it = bho::move(*srcit);
      *srcit = bho::move(tmp);
   }
};


}} //namespace bho::movelib

#endif   //BHO_MOVE_ALGO_BASIC_OP
