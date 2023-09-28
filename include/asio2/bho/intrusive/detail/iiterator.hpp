/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga  2006-2014
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef BHO_INTRUSIVE_DETAIL_IITERATOR_HPP
#define BHO_INTRUSIVE_DETAIL_IITERATOR_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/intrusive/detail/workaround.hpp>
#include <asio2/bho/intrusive/detail/iterator.hpp>
#include <asio2/bho/intrusive/pointer_traits.hpp>
#include <asio2/bho/intrusive/detail/mpl.hpp>
#include <asio2/bho/intrusive/detail/is_stateful_value_traits.hpp>

namespace bho {
namespace intrusive {

template<class ValueTraits>
struct value_traits_pointers
{
   typedef BHO_INTRUSIVE_OBTAIN_TYPE_WITH_DEFAULT
      (bho::intrusive::detail::
      , ValueTraits, value_traits_ptr
      , typename bho::intrusive::pointer_traits<typename ValueTraits::node_traits::node_ptr>::template
         rebind_pointer<ValueTraits>::type)   value_traits_ptr;

   typedef typename bho::intrusive::pointer_traits<value_traits_ptr>::template
      rebind_pointer<ValueTraits const>::type const_value_traits_ptr;
};

template<class ValueTraits, bool IsConst, class Category>
struct iiterator
{
   typedef ValueTraits                                         value_traits;
   typedef typename value_traits::node_traits                  node_traits;
   typedef typename node_traits::node                          node;
   typedef typename node_traits::node_ptr                      node_ptr;
   typedef ::bho::intrusive::pointer_traits<node_ptr>        nodepointer_traits_t;
   typedef typename nodepointer_traits_t::template
      rebind_pointer<void>::type                               void_pointer;
   typedef typename ValueTraits::value_type                    value_type;
   typedef typename ValueTraits::pointer                       nonconst_pointer;
   typedef typename ValueTraits::const_pointer                 yesconst_pointer;
   typedef typename ::bho::intrusive::pointer_traits
      <nonconst_pointer>::reference                            nonconst_reference;
   typedef typename ::bho::intrusive::pointer_traits
      <yesconst_pointer>::reference                            yesconst_reference;
   typedef typename nodepointer_traits_t::difference_type      difference_type;
   typedef typename detail::if_c
      <IsConst, yesconst_pointer, nonconst_pointer>::type      pointer;
   typedef typename detail::if_c
      <IsConst, yesconst_reference, nonconst_reference>::type  reference;
   typedef iterator
         < Category
         , value_type
         , difference_type
         , pointer
         , reference
         > iterator_type;
   typedef typename value_traits_pointers
      <ValueTraits>::value_traits_ptr                          value_traits_ptr;
   typedef typename value_traits_pointers
      <ValueTraits>::const_value_traits_ptr                    const_value_traits_ptr;
   static const bool stateful_value_traits =
      detail::is_stateful_value_traits<value_traits>::value;
};

template<class NodePtr, class StoredPointer, bool StatefulValueTraits = true>
struct iiterator_members
{

   BHO_INTRUSIVE_FORCEINLINE iiterator_members()
      : nodeptr_()//Value initialization to achieve "null iterators" (N3644)
   {}

   BHO_INTRUSIVE_FORCEINLINE iiterator_members(const NodePtr &n_ptr, const StoredPointer &data)
      :  nodeptr_(n_ptr), ptr_(data)
   {}

   BHO_INTRUSIVE_FORCEINLINE StoredPointer get_ptr() const
   {  return ptr_;  }

   NodePtr nodeptr_;
   StoredPointer ptr_;
};

template<class NodePtr, class StoredPointer>
struct iiterator_members<NodePtr, StoredPointer, false>
{
   BHO_INTRUSIVE_FORCEINLINE iiterator_members()
      : nodeptr_()//Value initialization to achieve "null iterators" (N3644)
   {}

   BHO_INTRUSIVE_FORCEINLINE iiterator_members(const NodePtr &n_ptr, const StoredPointer &)
      : nodeptr_(n_ptr)
   {}

   BHO_INTRUSIVE_FORCEINLINE StoredPointer get_ptr() const
   {  return StoredPointer();  }

   NodePtr nodeptr_;
};

} //namespace intrusive
} //namespace bho

#endif //BHO_INTRUSIVE_DETAIL_IITERATOR_HPP
