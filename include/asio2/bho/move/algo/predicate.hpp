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
#ifndef BHO_MOVE_ALGO_PREDICATE_HPP
#define BHO_MOVE_ALGO_PREDICATE_HPP

#include <asio2/bho/move/algo/move.hpp>
#include <asio2/bho/move/adl_move_swap.hpp>
#include <asio2/bho/move/algo/detail/basic_op.hpp>
#include <asio2/bho/move/detail/iterator_traits.hpp>
#include <asio2/bho/move/detail/destruct_n.hpp>
#include <cassert>

namespace bho {
namespace movelib {

template<class Comp>
struct antistable
{
   BHO_MOVE_FORCEINLINE explicit antistable(Comp &comp)
      : m_comp(comp)
   {}

   BHO_MOVE_FORCEINLINE antistable(const antistable & other)
      : m_comp(other.m_comp)
   {}

   template<class U, class V>
   BHO_MOVE_FORCEINLINE bool operator()(const U &u, const V & v)
   {  return !m_comp(v, u);  }

   BHO_MOVE_FORCEINLINE const Comp &get() const
   {  return m_comp; }

   private:
   antistable & operator=(const antistable &);
   Comp &m_comp;
};

template<class Comp>
Comp unantistable(Comp comp)
{   return comp;  }

template<class Comp>
Comp unantistable(antistable<Comp> comp)
{   return comp.get();  }

template <class Comp>
class negate
{
   public:
   BHO_MOVE_FORCEINLINE negate()
   {}

   BHO_MOVE_FORCEINLINE explicit negate(Comp comp)
      : m_comp(comp)
   {}

   template <class T1, class T2>
   BHO_MOVE_FORCEINLINE bool operator()(const T1& l, const T2& r)
   {
      return !m_comp(l, r);
   }

   private:
   Comp m_comp;
};


template <class Comp>
class inverse
{
   public:
   BHO_MOVE_FORCEINLINE inverse()
   {}

   BHO_MOVE_FORCEINLINE explicit inverse(Comp comp)
      : m_comp(comp)
   {}

   template <class T1, class T2>
   BHO_MOVE_FORCEINLINE bool operator()(const T1& l, const T2& r)
   {
      return m_comp(r, l);
   }

   private:
   Comp m_comp;
};

}  //namespace movelib {
}  //namespace bho {

#endif   //#define BHO_MOVE_ALGO_PREDICATE_HPP
