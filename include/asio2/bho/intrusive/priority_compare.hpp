/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2008
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef BHO_INTRUSIVE_PRIORITY_COMPARE_HPP
#define BHO_INTRUSIVE_PRIORITY_COMPARE_HPP

#include <asio2/bho/intrusive/detail/config_begin.hpp>
#include <asio2/bho/intrusive/detail/workaround.hpp>
#include <asio2/bho/intrusive/intrusive_fwd.hpp>

#include <asio2/bho/intrusive/detail/minimal_less_equal_header.hpp>

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

namespace bho {
namespace intrusive {

/// @cond

namespace adldft {

template<class T, class U>
BHO_INTRUSIVE_FORCEINLINE bool priority_order(const T &t, const U &u)
{  return t < u;  }

}  //namespace adldft {

/// @endcond

template <class T = void>
struct priority_compare
{
   //Compatibility with std::binary_function
   typedef T      first_argument_type;
   typedef T      second_argument_type;
   typedef bool   result_type;

   BHO_INTRUSIVE_FORCEINLINE bool operator()(const T &val, const T &val2) const
   {
      using adldft::priority_order;
      return priority_order(val, val2);
   }
};

template <>
struct priority_compare<void>
{
   template<class T, class U>
   BHO_INTRUSIVE_FORCEINLINE bool operator()(const T &t, const U &u) const
   {
      using adldft::priority_order;
      return priority_order(t, u);
   }
};

/// @cond

template<class PrioComp, class T>
struct get_prio_comp
{
   typedef PrioComp type;
};


template<class T>
struct get_prio_comp<void, T>
{
   typedef ::bho::intrusive::priority_compare<T> type;
};

/// @endcond

} //namespace intrusive
} //namespace bho

#include <asio2/bho/intrusive/detail/config_end.hpp>

#endif //BHO_INTRUSIVE_PRIORITY_COMPARE_HPP
