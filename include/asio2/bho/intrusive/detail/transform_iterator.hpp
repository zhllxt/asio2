/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2007-2013
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef BHO_INTRUSIVE_DETAIL_TRANSFORM_ITERATOR_HPP
#define BHO_INTRUSIVE_DETAIL_TRANSFORM_ITERATOR_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/intrusive/detail/config_begin.hpp>
#include <asio2/bho/intrusive/detail/workaround.hpp>
#include <asio2/bho/intrusive/detail/mpl.hpp>
#include <asio2/bho/intrusive/detail/iterator.hpp>

namespace bho {
namespace intrusive {
namespace detail {

template <class PseudoReference>
struct operator_arrow_proxy
{
   BHO_INTRUSIVE_FORCEINLINE operator_arrow_proxy(const PseudoReference &px)
      :  m_value(px)
   {}

   BHO_INTRUSIVE_FORCEINLINE PseudoReference* operator->() const { return &m_value; }
   // This function is needed for MWCW and BCC, which won't call operator->
   // again automatically per 13.3.1.2 para 8
//   operator T*() const { return &m_value; }
   mutable PseudoReference m_value;
};

template <class T>
struct operator_arrow_proxy<T&>
{
   BHO_INTRUSIVE_FORCEINLINE operator_arrow_proxy(T &px)
      :  m_value(px)
   {}

   BHO_INTRUSIVE_FORCEINLINE T* operator->() const { return &m_value; }
   // This function is needed for MWCW and BCC, which won't call operator->
   // again automatically per 13.3.1.2 para 8
//   operator T*() const { return &m_value; }
   T &m_value;
};

template <class Iterator, class UnaryFunction>
class transform_iterator
{
   public:
   typedef typename Iterator::iterator_category                                           iterator_category;
   typedef typename detail::remove_reference<typename UnaryFunction::result_type>::type   value_type;
   typedef typename Iterator::difference_type                                             difference_type;
   typedef operator_arrow_proxy<typename UnaryFunction::result_type>                      pointer;
   typedef typename UnaryFunction::result_type                                            reference;
   
   explicit transform_iterator(const Iterator &it, const UnaryFunction &f = UnaryFunction())
      :  members_(it, f)
   {}

   explicit transform_iterator()
      :  members_()
   {}

   BHO_INTRUSIVE_FORCEINLINE Iterator get_it() const
   {  return members_.m_it;   }

   //Constructors
   BHO_INTRUSIVE_FORCEINLINE transform_iterator& operator++()
   { increment();   return *this;   }

   BHO_INTRUSIVE_FORCEINLINE transform_iterator operator++(int)
   {
      transform_iterator result (*this);
      increment();
      return result;
   }

   BHO_INTRUSIVE_FORCEINLINE friend bool operator== (const transform_iterator& i, const transform_iterator& i2)
   { return i.equal(i2); }

   BHO_INTRUSIVE_FORCEINLINE friend bool operator!= (const transform_iterator& i, const transform_iterator& i2)
   { return !(i == i2); }

   BHO_INTRUSIVE_FORCEINLINE friend typename Iterator::difference_type operator- (const transform_iterator& i, const transform_iterator& i2)
   { return i2.distance_to(i); }

   //Arithmetic
   transform_iterator& operator+=(typename Iterator::difference_type off)
   {  this->advance(off); return *this;   }

   BHO_INTRUSIVE_FORCEINLINE transform_iterator operator+(typename Iterator::difference_type off) const
   {
      transform_iterator other(*this);
      other.advance(off);
      return other;
   }

   BHO_INTRUSIVE_FORCEINLINE friend transform_iterator operator+(typename Iterator::difference_type off, const transform_iterator& right)
   {  return right + off; }

   BHO_INTRUSIVE_FORCEINLINE transform_iterator& operator-=(typename Iterator::difference_type off)
   {  this->advance(-off); return *this;   }

   BHO_INTRUSIVE_FORCEINLINE transform_iterator operator-(typename Iterator::difference_type off) const
   {  return *this + (-off);  }

   BHO_INTRUSIVE_FORCEINLINE typename UnaryFunction::result_type operator*() const
   { return dereference(); }

   BHO_INTRUSIVE_FORCEINLINE operator_arrow_proxy<typename UnaryFunction::result_type>
      operator->() const
   { return operator_arrow_proxy<typename UnaryFunction::result_type>(dereference());  }

   private:
   struct members
      :  UnaryFunction
   {
      BHO_INTRUSIVE_FORCEINLINE members(const Iterator &it, const UnaryFunction &f)
         :  UnaryFunction(f), m_it(it)
      {}

      BHO_INTRUSIVE_FORCEINLINE members()
      {}

      Iterator m_it;
   } members_;


   BHO_INTRUSIVE_FORCEINLINE void increment()
   { ++members_.m_it; }

   BHO_INTRUSIVE_FORCEINLINE void decrement()
   { --members_.m_it; }

   BHO_INTRUSIVE_FORCEINLINE bool equal(const transform_iterator &other) const
   {  return members_.m_it == other.members_.m_it;   }

   BHO_INTRUSIVE_FORCEINLINE bool less(const transform_iterator &other) const
   {  return other.members_.m_it < members_.m_it;   }

   typename UnaryFunction::result_type dereference() const
   { return members_(*members_.m_it); }

   void advance(typename Iterator::difference_type n)
   {  bho::intrusive::iterator_advance(members_.m_it, n); }

   typename Iterator::difference_type distance_to(const transform_iterator &other)const
   {  return bho::intrusive::iterator_distance(other.members_.m_it, members_.m_it); }
};

} //namespace detail
} //namespace intrusive
} //namespace bho

#include <asio2/bho/intrusive/detail/config_end.hpp>

#endif //BHO_INTRUSIVE_DETAIL_TRANSFORM_ITERATOR_HPP
