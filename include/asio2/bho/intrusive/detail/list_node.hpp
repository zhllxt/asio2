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

#ifndef BHO_INTRUSIVE_LIST_NODE_HPP
#define BHO_INTRUSIVE_LIST_NODE_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/intrusive/detail/workaround.hpp>
#include <asio2/bho/intrusive/pointer_rebind.hpp>

namespace bho {
namespace intrusive {

// list_node_traits can be used with circular_list_algorithms and supplies
// a list_node holding the pointers needed for a double-linked list
// it is used by list_derived_node and list_member_node

template<class VoidPointer>
struct list_node
{
   typedef typename pointer_rebind<VoidPointer, list_node>::type  node_ptr;
   node_ptr next_;
   node_ptr prev_;
};

template<class VoidPointer>
struct list_node_traits
{
   typedef list_node<VoidPointer>      node;
   typedef typename node::node_ptr     node_ptr;
   typedef typename pointer_rebind<VoidPointer, const node>::type   const_node_ptr;

   BHO_INTRUSIVE_FORCEINLINE static node_ptr get_previous(const_node_ptr n)
   {  return n->prev_;  }

   BHO_INTRUSIVE_FORCEINLINE static node_ptr get_previous(node_ptr n)
   {  return n->prev_;  }

   BHO_INTRUSIVE_FORCEINLINE static void set_previous(node_ptr n, node_ptr prev)
   {  n->prev_ = prev;  }

   BHO_INTRUSIVE_FORCEINLINE static node_ptr get_next(const_node_ptr n)
   {  return n->next_;  }

   BHO_INTRUSIVE_FORCEINLINE static node_ptr get_next(node_ptr n)
   {  return n->next_;  }

   BHO_INTRUSIVE_FORCEINLINE static void set_next(node_ptr n, node_ptr next)
   {  n->next_ = next;  }
};

} //namespace intrusive
} //namespace bho

#endif //BHO_INTRUSIVE_LIST_NODE_HPP
