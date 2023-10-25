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

#ifndef BHO_INTRUSIVE_LIST_ITERATOR_HPP
#define BHO_INTRUSIVE_LIST_ITERATOR_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/intrusive/detail/workaround.hpp>
#include <asio2/bho/intrusive/detail/std_fwd.hpp>
#include <asio2/bho/intrusive/detail/iiterator.hpp>
#include <asio2/bho/intrusive/detail/mpl.hpp>

namespace bho {
namespace intrusive {

// list_iterator provides some basic functions for a
// node oriented bidirectional iterator:
template<class ValueTraits, bool IsConst>
class list_iterator
{
   private:
   typedef iiterator
      <ValueTraits, IsConst, std::bidirectional_iterator_tag> types_t;

   static const bool stateful_value_traits =                types_t::stateful_value_traits;

   typedef ValueTraits                                      value_traits;
   typedef typename types_t::node_traits                    node_traits;

   typedef typename types_t::node                           node;
   typedef typename types_t::node_ptr                       node_ptr;
   typedef typename types_t::const_value_traits_ptr         const_value_traits_ptr;
   class nat;
   typedef typename
      detail::if_c< IsConst
                  , list_iterator<value_traits, false>
                  , nat>::type                              nonconst_iterator;

   public:
   typedef typename types_t::iterator_type::difference_type    difference_type;
   typedef typename types_t::iterator_type::value_type         value_type;
   typedef typename types_t::iterator_type::pointer            pointer;
   typedef typename types_t::iterator_type::reference          reference;
   typedef typename types_t::iterator_type::iterator_category  iterator_category;

   BHO_INTRUSIVE_FORCEINLINE list_iterator()
   {}

   BHO_INTRUSIVE_FORCEINLINE explicit list_iterator(node_ptr nodeptr, const_value_traits_ptr traits_ptr)
      : members_(nodeptr, traits_ptr)
   {}

   BHO_INTRUSIVE_FORCEINLINE list_iterator(const list_iterator &other)
      :  members_(other.pointed_node(), other.get_value_traits())
   {}

   BHO_INTRUSIVE_FORCEINLINE list_iterator(const nonconst_iterator &other)
      :  members_(other.pointed_node(), other.get_value_traits())
   {}

   BHO_INTRUSIVE_FORCEINLINE list_iterator &operator=(const list_iterator &other)
   {  members_.nodeptr_ = other.members_.nodeptr_;  return *this;  }

   BHO_INTRUSIVE_FORCEINLINE node_ptr pointed_node() const
   { return members_.nodeptr_; }

   BHO_INTRUSIVE_FORCEINLINE list_iterator &operator=(node_ptr nodeptr)
   {  members_.nodeptr_ = nodeptr;  return *this;  }

   BHO_INTRUSIVE_FORCEINLINE const_value_traits_ptr get_value_traits() const
   {  return members_.get_ptr(); }

   public:
   BHO_INTRUSIVE_FORCEINLINE list_iterator& operator++()
   {
      node_ptr p = node_traits::get_next(members_.nodeptr_);
      members_.nodeptr_ = p;
      return static_cast<list_iterator&> (*this);
   }

   BHO_INTRUSIVE_FORCEINLINE list_iterator operator++(int)
   {
      list_iterator result (*this);
      members_.nodeptr_ = node_traits::get_next(members_.nodeptr_);
      return result;
   }

   BHO_INTRUSIVE_FORCEINLINE list_iterator& operator--()
   {
      members_.nodeptr_ = node_traits::get_previous(members_.nodeptr_);
      return static_cast<list_iterator&> (*this);
   }

   BHO_INTRUSIVE_FORCEINLINE list_iterator operator--(int)
   {
      list_iterator result (*this);
      members_.nodeptr_ = node_traits::get_previous(members_.nodeptr_);
      return result;
   }

   BHO_INTRUSIVE_FORCEINLINE friend bool operator== (const list_iterator& l, const list_iterator& r)
   {  return l.pointed_node() == r.pointed_node();   }

   BHO_INTRUSIVE_FORCEINLINE friend bool operator!= (const list_iterator& l, const list_iterator& r)
   {  return !(l == r); }

   BHO_INTRUSIVE_FORCEINLINE reference operator*() const
   {  return *operator->();   }

   BHO_INTRUSIVE_FORCEINLINE pointer operator->() const
   { return this->operator_arrow(detail::bool_<stateful_value_traits>()); }

   BHO_INTRUSIVE_FORCEINLINE list_iterator<ValueTraits, false> unconst() const
   {  return list_iterator<ValueTraits, false>(this->pointed_node(), this->get_value_traits());   }

   private:
   BHO_INTRUSIVE_FORCEINLINE pointer operator_arrow(detail::false_) const
   { return ValueTraits::to_value_ptr(members_.nodeptr_); }

   BHO_INTRUSIVE_FORCEINLINE pointer operator_arrow(detail::true_) const
   { return this->get_value_traits()->to_value_ptr(members_.nodeptr_); }

   iiterator_members<node_ptr, const_value_traits_ptr, stateful_value_traits> members_;
};

} //namespace intrusive
} //namespace bho

#endif //BHO_INTRUSIVE_LIST_ITERATOR_HPP
