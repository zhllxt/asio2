/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Olaf Krzikalla 2004-2006.
// (C) Copyright Ion Gaztanaga  2006-2013
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef BHO_INTRUSIVE_SLIST_ITERATOR_HPP
#define BHO_INTRUSIVE_SLIST_ITERATOR_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/intrusive/detail/config_begin.hpp>
#include <asio2/bho/intrusive/detail/workaround.hpp>
#include <asio2/bho/intrusive/detail/std_fwd.hpp>
#include <asio2/bho/intrusive/detail/iiterator.hpp>
#include <asio2/bho/intrusive/detail/mpl.hpp>
#include <asio2/bho/static_assert.hpp>

namespace bho {
namespace intrusive {


// slist_iterator provides some basic functions for a
// node oriented bidirectional iterator:
template<class ValueTraits, bool IsConst>
class slist_iterator
{
   private:
   typedef iiterator
      <ValueTraits, IsConst, std::forward_iterator_tag> types_t;

   static const bool stateful_value_traits =                types_t::stateful_value_traits;

   typedef ValueTraits                                      value_traits;
   typedef typename types_t::node_traits                    node_traits;

   typedef typename types_t::node                           node;
   typedef typename types_t::node_ptr                       node_ptr;
   typedef typename types_t::const_value_traits_ptr         const_value_traits_ptr;
   class nat;
   typedef typename
      detail::if_c< IsConst
                  , slist_iterator<value_traits, false>
                  , nat>::type                              nonconst_iterator;

   public:
   typedef typename types_t::iterator_type::difference_type    difference_type;
   typedef typename types_t::iterator_type::value_type         value_type;
   typedef typename types_t::iterator_type::pointer            pointer;
   typedef typename types_t::iterator_type::reference          reference;
   typedef typename types_t::iterator_type::iterator_category  iterator_category;

   BHO_INTRUSIVE_FORCEINLINE slist_iterator()
   {}

   BHO_INTRUSIVE_FORCEINLINE slist_iterator(node_ptr nodeptr, const_value_traits_ptr traits_ptr)
      : members_(nodeptr, traits_ptr)
   {}

   BHO_INTRUSIVE_FORCEINLINE explicit slist_iterator(node_ptr nodeptr)
      : members_(nodeptr, const_value_traits_ptr())
   {  BHO_STATIC_ASSERT((stateful_value_traits == false));  }

   BHO_INTRUSIVE_FORCEINLINE slist_iterator(const slist_iterator &other)
      :  members_(other.pointed_node(), other.get_value_traits())
   {}

   BHO_INTRUSIVE_FORCEINLINE slist_iterator(const nonconst_iterator &other)
      :  members_(other.pointed_node(), other.get_value_traits())
   {}

   BHO_INTRUSIVE_FORCEINLINE slist_iterator &operator=(const slist_iterator &other)
   {  members_.nodeptr_ = other.members_.nodeptr_;  return *this;  }

   BHO_INTRUSIVE_FORCEINLINE node_ptr pointed_node() const
   { return members_.nodeptr_; }

   BHO_INTRUSIVE_FORCEINLINE slist_iterator &operator=(node_ptr n)
   {  members_.nodeptr_ = n;  return static_cast<slist_iterator&>(*this);  }

   BHO_INTRUSIVE_FORCEINLINE const_value_traits_ptr get_value_traits() const
   {  return members_.get_ptr(); }

   BHO_INTRUSIVE_FORCEINLINE bool operator!() const
   {  return !members_.nodeptr_; }

   public:
   BHO_INTRUSIVE_FORCEINLINE slist_iterator& operator++()
   {
      members_.nodeptr_ = node_traits::get_next(members_.nodeptr_);
      return static_cast<slist_iterator&> (*this);
   }

   BHO_INTRUSIVE_FORCEINLINE slist_iterator operator++(int)
   {
      slist_iterator result (*this);
      members_.nodeptr_ = node_traits::get_next(members_.nodeptr_);
      return result;
   }

   BHO_INTRUSIVE_FORCEINLINE friend bool operator== (const slist_iterator& l, const slist_iterator& r)
   {  return l.pointed_node() == r.pointed_node();   }

   BHO_INTRUSIVE_FORCEINLINE friend bool operator!= (const slist_iterator& l, const slist_iterator& r)
   {  return l.pointed_node() != r.pointed_node();   }

   BHO_INTRUSIVE_FORCEINLINE reference operator*() const
   {  return *operator->();   }

   BHO_INTRUSIVE_FORCEINLINE pointer operator->() const
   { return this->operator_arrow(detail::bool_<stateful_value_traits>()); }

   BHO_INTRUSIVE_FORCEINLINE slist_iterator<ValueTraits, false> unconst() const
   {  return slist_iterator<ValueTraits, false>(this->pointed_node(), this->get_value_traits());   }

   private:

   BHO_INTRUSIVE_FORCEINLINE pointer operator_arrow(detail::false_) const
   { return ValueTraits::to_value_ptr(members_.nodeptr_); }

   BHO_INTRUSIVE_FORCEINLINE pointer operator_arrow(detail::true_) const
   { return this->get_value_traits()->to_value_ptr(members_.nodeptr_); }

   iiterator_members<node_ptr, const_value_traits_ptr, stateful_value_traits> members_;
};

} //namespace intrusive
} //namespace bho

#include <asio2/bho/intrusive/detail/config_end.hpp>

#endif //BHO_INTRUSIVE_SLIST_ITERATOR_HPP
