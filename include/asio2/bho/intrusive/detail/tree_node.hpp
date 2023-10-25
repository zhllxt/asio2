/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga  2007-2013
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef BHO_INTRUSIVE_TREE_NODE_HPP
#define BHO_INTRUSIVE_TREE_NODE_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/intrusive/detail/config_begin.hpp>
#include <asio2/bho/intrusive/detail/workaround.hpp>
#include <asio2/bho/intrusive/pointer_rebind.hpp>

namespace bho {
namespace intrusive {

template<class VoidPointer>
struct tree_node
{
   typedef typename pointer_rebind<VoidPointer, tree_node>::type  node_ptr;

   node_ptr parent_, left_, right_;
};

template<class VoidPointer>
struct tree_node_traits
{
   typedef tree_node<VoidPointer> node;

   typedef typename node::node_ptr   node_ptr;
   typedef typename pointer_rebind<VoidPointer, const node>::type const_node_ptr;

   BHO_INTRUSIVE_FORCEINLINE static node_ptr get_parent(const_node_ptr n)
   {  return n->parent_;  }

   BHO_INTRUSIVE_FORCEINLINE static node_ptr get_parent(node_ptr n)
   {  return n->parent_;  }

   BHO_INTRUSIVE_FORCEINLINE static void set_parent(node_ptr n, node_ptr p)
   {  n->parent_ = p;  }

   BHO_INTRUSIVE_FORCEINLINE static node_ptr get_left(const_node_ptr n)
   {  return n->left_;  }

   BHO_INTRUSIVE_FORCEINLINE static node_ptr get_left(node_ptr n)
   {  return n->left_;  }

   BHO_INTRUSIVE_FORCEINLINE static void set_left(node_ptr n, node_ptr l)
   {  n->left_ = l;  }

   BHO_INTRUSIVE_FORCEINLINE static node_ptr get_right(const_node_ptr n)
   {  return n->right_;  }

   BHO_INTRUSIVE_FORCEINLINE static node_ptr get_right(node_ptr n)
   {  return n->right_;  }

   BHO_INTRUSIVE_FORCEINLINE static void set_right(node_ptr n, node_ptr r)
   {  n->right_ = r;  }
};

} //namespace intrusive
} //namespace bho

#include <asio2/bho/intrusive/detail/config_end.hpp>

#endif //BHO_INTRUSIVE_TREE_NODE_HPP
