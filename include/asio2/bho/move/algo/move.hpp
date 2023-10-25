//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2012-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/move for documentation.
//
//////////////////////////////////////////////////////////////////////////////

//! \file

#ifndef BHO_MOVE_ALGO_MOVE_HPP
#define BHO_MOVE_ALGO_MOVE_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif
#
#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/move/detail/config_begin.hpp>

#include <asio2/bho/move/utility_core.hpp>
#include <asio2/bho/move/detail/iterator_traits.hpp>
#include <asio2/bho/move/detail/iterator_to_raw_pointer.hpp>
#include <asio2/bho/move/detail/addressof.hpp>
#if defined(BHO_MOVE_USE_STANDARD_LIBRARY_MOVE)
#include <algorithm>
#endif

namespace bho {

//////////////////////////////////////////////////////////////////////////////
//
//                               move
//
//////////////////////////////////////////////////////////////////////////////

#if !defined(BHO_MOVE_USE_STANDARD_LIBRARY_MOVE)

   //! <b>Effects</b>: Moves elements in the range [first,last) into the range [result,result + (last -
   //!   first)) starting from first and proceeding to last. For each non-negative integer n < (last-first),
   //!   performs *(result + n) = ::bho::move (*(first + n)).
   //!
   //! <b>Effects</b>: result + (last - first).
   //!
   //! <b>Requires</b>: result shall not be in the range [first,last).
   //!
   //! <b>Complexity</b>: Exactly last - first move assignments.
   template <typename I, // I models InputIterator
            typename O> // O models OutputIterator
   O move(I f, I l, O result)
   {
      while (f != l) {
         *result = ::bho::move(*f);
         ++f; ++result;
      }
      return result;
   }

   //////////////////////////////////////////////////////////////////////////////
   //
   //                               move_backward
   //
   //////////////////////////////////////////////////////////////////////////////

   //! <b>Effects</b>: Moves elements in the range [first,last) into the range
   //!   [result - (last-first),result) starting from last - 1 and proceeding to
   //!   first. For each positive integer n <= (last - first),
   //!   performs *(result - n) = ::bho::move(*(last - n)).
   //!
   //! <b>Requires</b>: result shall not be in the range [first,last).
   //!
   //! <b>Returns</b>: result - (last - first).
   //!
   //! <b>Complexity</b>: Exactly last - first assignments.
   template <typename I, // I models BidirectionalIterator
   typename O> // O models BidirectionalIterator
   O move_backward(I f, I l, O result)
   {
      while (f != l) {
         --l; --result;
         *result = ::bho::move(*l);
      }
      return result;
   }

#else

   using ::std::move_backward;

#endif   //!defined(BHO_MOVE_USE_STANDARD_LIBRARY_MOVE)

//////////////////////////////////////////////////////////////////////////////
//
//                               uninitialized_move
//
//////////////////////////////////////////////////////////////////////////////

//! <b>Effects</b>:
//!   \code
//!   for (; first != last; ++result, ++first)
//!      new (static_cast<void*>(&*result))
//!         typename iterator_traits<ForwardIterator>::value_type(bho::move(*first));
//!   \endcode
//!
//! <b>Returns</b>: result
template
   <typename I, // I models InputIterator
    typename F> // F models ForwardIterator
F uninitialized_move(I f, I l, F r
   /// @cond
//   ,typename ::bho::move_detail::enable_if<has_move_emulation_enabled<typename bho::movelib::iterator_traits<I>::value_type> >::type* = 0
   /// @endcond
   )
{
   typedef typename bho::movelib::iterator_traits<I>::value_type input_value_type;

   F back = r;
   BHO_MOVE_TRY{
      while (f != l) {
         void * const addr = static_cast<void*>(::bho::move_detail::addressof(*r));
         ::new(addr) input_value_type(::bho::move(*f));
         ++f; ++r;
      }
   }
   BHO_MOVE_CATCH(...){
      for (; back != r; ++back){
         bho::movelib::iterator_to_raw_pointer(back)->~input_value_type();
      }
      BHO_MOVE_RETHROW;
   }
   BHO_MOVE_CATCH_END
   return r;
}

/// @cond
/*
template
   <typename I,   // I models InputIterator
    typename F>   // F models ForwardIterator
F uninitialized_move(I f, I l, F r,
   typename ::bho::move_detail::disable_if<has_move_emulation_enabled<typename bho::movelib::iterator_traits<I>::value_type> >::type* = 0)
{
   return std::uninitialized_copy(f, l, r);
}
*/

/// @endcond

}  //namespace bho {

#include <asio2/bho/move/detail/config_end.hpp>

#endif //#ifndef BHO_MOVE_ALGO_MOVE_HPP
