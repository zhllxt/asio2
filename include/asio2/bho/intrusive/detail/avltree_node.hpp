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

#ifndef BHO_INTRUSIVE_AVLTREE_NODE_HPP
#define BHO_INTRUSIVE_AVLTREE_NODE_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/intrusive/detail/config_begin.hpp>
#include <asio2/bho/intrusive/detail/workaround.hpp>
#include <asio2/bho/intrusive/pointer_rebind.hpp>
#include <asio2/bho/intrusive/avltree_algorithms.hpp>
#include <asio2/bho/intrusive/pointer_plus_bits.hpp>
#include <asio2/bho/intrusive/detail/mpl.hpp>

namespace bho {
namespace intrusive {

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                Generic node_traits for any pointer type                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

//This is the compact representation: 3 pointers
template<class VoidPointer>
struct compact_avltree_node
{
   typedef typename pointer_rebind<VoidPointer, compact_avltree_node<VoidPointer> >::type       node_ptr;
   typedef typename pointer_rebind<VoidPointer, const compact_avltree_node<VoidPointer> >::type const_node_ptr;
   enum balance { negative_t, zero_t, positive_t };
   node_ptr parent_, left_, right_;
};

//This is the normal representation: 3 pointers + enum
template<class VoidPointer>
struct avltree_node
{
   typedef typename pointer_rebind<VoidPointer, avltree_node<VoidPointer> >::type         node_ptr;
   typedef typename pointer_rebind<VoidPointer, const avltree_node<VoidPointer> >::type   const_node_ptr;
   enum balance { negative_t, zero_t, positive_t };
   node_ptr parent_, left_, right_;
   balance balance_;
};

//This is the default node traits implementation
//using a node with 3 generic pointers plus an enum
template<class VoidPointer>
struct default_avltree_node_traits_impl
{
   typedef avltree_node<VoidPointer>      node;
   typedef typename node::node_ptr        node_ptr;
   typedef typename node::const_node_ptr  const_node_ptr;

   typedef typename node::balance balance;

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

   BHO_INTRUSIVE_FORCEINLINE static balance get_balance(const_node_ptr n)
   {  return n->balance_;  }

   BHO_INTRUSIVE_FORCEINLINE static balance get_balance(node_ptr n)
   {  return n->balance_;  }

   BHO_INTRUSIVE_FORCEINLINE static void set_balance(node_ptr n, balance b)
   {  n->balance_ = b;  }

   BHO_INTRUSIVE_FORCEINLINE static balance negative()
   {  return node::negative_t;  }

   BHO_INTRUSIVE_FORCEINLINE static balance zero()
   {  return node::zero_t;  }

   BHO_INTRUSIVE_FORCEINLINE static balance positive()
   {  return node::positive_t;  }
};

//This is the compact node traits implementation
//using a node with 3 generic pointers
template<class VoidPointer>
struct compact_avltree_node_traits_impl
{
   typedef compact_avltree_node<VoidPointer> node;
   typedef typename node::node_ptr           node_ptr;
   typedef typename node::const_node_ptr     const_node_ptr;
   typedef typename node::balance balance;

   typedef pointer_plus_bits<node_ptr, 2> ptr_bit;

   BHO_INTRUSIVE_FORCEINLINE static node_ptr get_parent(const_node_ptr n)
   {  return ptr_bit::get_pointer(n->parent_);  }

   BHO_INTRUSIVE_FORCEINLINE static void set_parent(node_ptr n, node_ptr p)
   {  ptr_bit::set_pointer(n->parent_, p);  }

   BHO_INTRUSIVE_FORCEINLINE static node_ptr get_left(const_node_ptr n)
   {  return n->left_;  }

   BHO_INTRUSIVE_FORCEINLINE static void set_left(node_ptr n, node_ptr l)
   {  n->left_ = l;  }

   BHO_INTRUSIVE_FORCEINLINE static node_ptr get_right(const_node_ptr n)
   {  return n->right_;  }

   BHO_INTRUSIVE_FORCEINLINE static void set_right(node_ptr n, node_ptr r)
   {  n->right_ = r;  }

   BHO_INTRUSIVE_FORCEINLINE static balance get_balance(const_node_ptr n)
   {  return (balance)ptr_bit::get_bits(n->parent_);  }

   BHO_INTRUSIVE_FORCEINLINE static void set_balance(node_ptr n, balance b)
   {  ptr_bit::set_bits(n->parent_, (std::size_t)b);  }

   BHO_INTRUSIVE_FORCEINLINE static balance negative()
   {  return node::negative_t;  }

   BHO_INTRUSIVE_FORCEINLINE static balance zero()
   {  return node::zero_t;  }

   BHO_INTRUSIVE_FORCEINLINE static balance positive()
   {  return node::positive_t;  }
};

//Dispatches the implementation based on the boolean
template<class VoidPointer, bool Compact>
struct avltree_node_traits_dispatch
   :  public default_avltree_node_traits_impl<VoidPointer>
{};

template<class VoidPointer>
struct avltree_node_traits_dispatch<VoidPointer, true>
   :  public compact_avltree_node_traits_impl<VoidPointer>
{};

//Inherit from rbtree_node_traits_dispatch depending on the embedding capabilities
template<class VoidPointer, bool OptimizeSize = false>
struct avltree_node_traits
   :  public avltree_node_traits_dispatch
         < VoidPointer
         , OptimizeSize &&
            max_pointer_plus_bits
            < VoidPointer
            , detail::alignment_of<compact_avltree_node<VoidPointer> >::value
            >::value >= 2u
         >
{};

} //namespace intrusive
} //namespace bho

#include <asio2/bho/intrusive/detail/config_end.hpp>

#endif //BHO_INTRUSIVE_AVLTREE_NODE_HPP
