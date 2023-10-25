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

#ifndef BHO_INTRUSIVE_FWD_HPP
#define BHO_INTRUSIVE_FWD_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif
#
#ifndef BHO_CSTDINT_HPP
#  include <asio2/bho/cstdint.hpp>
#endif
#
#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

//! \file
//! This header file forward declares most Intrusive classes.
//!
//! It forward declares the following containers and hooks:
//!   - bho::intrusive::slist / bho::intrusive::slist_base_hook / bho::intrusive::slist_member_hook
//!   - bho::intrusive::list / bho::intrusive::list_base_hook / bho::intrusive::list_member_hook
//!   - bho::intrusive::bstree / bho::intrusive::bs_set / bho::intrusive::bs_multiset /
//!      bho::intrusive::bs_set_base_hook / bho::intrusive::bs_set_member_hook
//!   - bho::intrusive::rbtree / bho::intrusive::set / bho::intrusive::multiset /
//!      bho::intrusive::set_base_hook / bho::intrusive::set_member_hook
//!   - bho::intrusive::avltree / bho::intrusive::avl_set / bho::intrusive::avl_multiset /
//!      bho::intrusive::avl_set_base_hook / bho::intrusive::avl_set_member_hook
//!   - bho::intrusive::splaytree / bho::intrusive::splay_set / bho::intrusive::splay_multiset
//!   - bho::intrusive::sgtree / bho::intrusive::sg_set / bho::intrusive::sg_multiset
//!   - bho::intrusive::treap / bho::intrusive::treap_set / bho::intrusive::treap_multiset
//!   - bho::intrusive::hashtable / bho::intrusive::unordered_set / bho::intrusive::unordered_multiset /
//!      bho::intrusive::unordered_set_base_hook / bho::intrusive::unordered_set_member_hook /
//!   - bho::intrusive::any_base_hook / bho::intrusive::any_member_hook
//!
//! It forward declares the following container or hook options:
//!   - bho::intrusive::constant_time_size / bho::intrusive::size_type / bho::intrusive::compare / bho::intrusive::equal
//!   - bho::intrusive::floating_point / bho::intrusive::priority / bho::intrusive::hash
//!   - bho::intrusive::value_traits / bho::intrusive::member_hook / bho::intrusive::function_hook / bho::intrusive::base_hook
//!   - bho::intrusive::void_pointer / bho::intrusive::tag / bho::intrusive::link_mode
//!   - bho::intrusive::optimize_size / bho::intrusive::linear / bho::intrusive::cache_last
//!   - bho::intrusive::bucket_traits / bho::intrusive::store_hash / bho::intrusive::optimize_multikey
//!   - bho::intrusive::power_2_buckets / bho::intrusive::cache_begin / bho::intrusive::compare_hash / bho::intrusive::incremental
//!
//! It forward declares the following value traits utilities:
//!   - bho::intrusive::value_traits / bho::intrusive::derivation_value_traits /
//!      bho::intrusive::trivial_value_traits
//!
//! Finally it forward declares the following general purpose utilities:
//!   - bho::intrusive::pointer_plus_bits / bho::intrusive::priority_compare.

#if !defined(BHO_INTRUSIVE_DOXYGEN_INVOKED)

#include <cstddef>
#include <asio2/bho/intrusive/link_mode.hpp>
#include <asio2/bho/intrusive/detail/workaround.hpp>

namespace bho {
namespace intrusive {

#if !defined(BHO_INTERPROCESS_DOXYGEN_INVOKED)
#  ifdef BHO_HAS_INTPTR_T
      using ::bho::uintptr_t;
#  else
      typedef std::size_t uintptr_t;
#  endif
#endif

////////////////////////////
//     Node algorithms
////////////////////////////

//Algorithms predeclarations
template<class NodeTraits>
class circular_list_algorithms;

template<class NodeTraits>
class circular_slist_algorithms;

template<class NodeTraits>
class linear_slist_algorithms;

template<class NodeTraits>
class bstree_algorithms;

template<class NodeTraits>
class rbtree_algorithms;

template<class NodeTraits>
class avltree_algorithms;

template<class NodeTraits>
class sgtree_algorithms;

template<class NodeTraits>
class splaytree_algorithms;

template<class NodeTraits>
class treap_algorithms;

////////////////////////////
//       Containers
////////////////////////////

//slist
#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   , class O6  = void
   >
#else
template<class T, class ...Options>
#endif
class slist;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   >
#else
template<class ...Options>
#endif
class slist_base_hook;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   >
#else
template<class ...Options>
#endif
class slist_member_hook;

//list
#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   >
#else
template<class T, class ...Options>
#endif
class list;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   >
#else
template<class ...Options>
#endif
class list_base_hook;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   >
#else
template<class ...Options>
#endif
class list_member_hook;

//rbtree/set/multiset
#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   , class O6  = void
   >
#else
template<class T, class ...Options>
#endif
class rbtree;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   , class O6  = void
   >
#else
template<class T, class ...Options>
#endif
class set;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   , class O6  = void
   >
#else
template<class T, class ...Options>
#endif
class multiset;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   >
#else
template<class ...Options>
#endif
class set_base_hook;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   >
#else
template<class ...Options>
#endif
class set_member_hook;

//splaytree/splay_set/splay_multiset
#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   , class O6  = void
   >
#else
template<class T, class ...Options>
#endif
class splaytree;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   , class O6  = void
   >
#else
template<class T, class ...Options>
#endif
class splay_set;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   , class O6  = void
   >
#else
template<class T, class ...Options>
#endif
class splay_multiset;

//avltree/avl_set/avl_multiset
#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   , class O6  = void
   >
#else
template<class T, class ...Options>
#endif
class avltree;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   , class O6  = void
   >
#else
template<class T, class ...Options>
#endif
class avl_set;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   , class O6  = void
   >
#else
template<class T, class ...Options>
#endif
class avl_multiset;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   >
#else
template<class ...Options>
#endif
class avl_set_base_hook;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   >
#else
template<class ...Options>
#endif
class avl_set_member_hook;


//treap/treap_set/treap_multiset
#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   , class O6  = void
   , class O7  = void
   >
#else
template<class T, class ...Options>
#endif
class treap;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   , class O6  = void
   , class O7  = void
   >
#else
template<class T, class ...Options>
#endif
class treap_set;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   , class O6  = void
   , class O7  = void
   >
#else
template<class T, class ...Options>
#endif
class treap_multiset;

//sgtree/sg_set/sg_multiset
#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   , class O6  = void
   >
#else
template<class T, class ...Options>
#endif
class sgtree;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   , class O6  = void
   >
#else
template<class T, class ...Options>
#endif
class sg_set;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   , class O6  = void
   >
#else
template<class T, class ...Options>
#endif
class sg_multiset;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   , class O6  = void
   >
#else
template<class T, class ...Options>
#endif
class bstree;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   , class O6  = void
   >
#else
template<class T, class ...Options>
#endif
class bs_set;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   , class O6  = void
   >
#else
template<class T, class ...Options>
#endif
class bs_multiset;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   >
#else
template<class ...Options>
#endif
class bs_set_base_hook;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   >
#else
template<class ...Options>
#endif
class bs_set_member_hook;

//hashtable/unordered_set/unordered_multiset

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   , class O6  = void
   , class O7  = void
   , class O8  = void
   , class O9  = void
   , class O10 = void
   >
#else
template<class T, class ...Options>
#endif
class hashtable;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   , class O6  = void
   , class O7  = void
   , class O8  = void
   , class O9  = void
   , class O10 = void
   , class O11 = void
   >
#else
template<class T, class ...Options>
#endif
class unordered_set;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class T
   , class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   , class O5  = void
   , class O6  = void
   , class O7  = void
   , class O8  = void
   , class O9  = void
   , class O10 = void
   , class O11 = void
   >
#else
template<class T, class ...Options>
#endif
class unordered_multiset;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   >
#else
template<class ...Options>
#endif
class unordered_set_base_hook;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   , class O4  = void
   >
#else
template<class ...Options>
#endif
class unordered_set_member_hook;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   >
#else
template<class ...Options>
#endif
class any_base_hook;

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template
   < class O1  = void
   , class O2  = void
   , class O3  = void
   >
#else
template<class ...Options>
#endif
class any_member_hook;

//Options

template<bool Enabled>
struct constant_time_size;

template<typename SizeType>
struct size_type;

template<typename Compare>
struct compare;

template<bool Enabled>
struct floating_point;

template<typename Equal>
struct equal;

template<typename Priority>
struct priority;

template<typename Hash>
struct hash;

template<typename ValueTraits> struct value_traits;

template< typename Parent
        , typename MemberHook
        , MemberHook Parent::* PtrToMember>
struct member_hook;

template<typename Functor>
struct function_hook;

template<typename BaseHook>
struct base_hook;

template<typename VoidPointer>
struct void_pointer;

template<typename Tag>
struct tag;

template<link_mode_type LinkType>
struct link_mode;

template<bool Enabled> struct
optimize_size;

template<bool Enabled>
struct linear;

template<bool Enabled>
struct cache_last;

template<typename BucketTraits>
struct bucket_traits;

template<bool Enabled>
struct store_hash;

template<bool Enabled>
struct optimize_multikey;

template<bool Enabled>
struct power_2_buckets;

template<bool Enabled>
struct cache_begin;

template<bool Enabled>
struct compare_hash;

template<bool Enabled>
struct incremental;

//Value traits

template<typename ValueTraits>
struct value_traits;

template< typename Parent
        , typename MemberHook
        , MemberHook Parent::* PtrToMember>
struct member_hook;

template< typename Functor>
struct function_hook;

template<typename BaseHook>
struct base_hook;

template<class T, class NodeTraits, link_mode_type LinkMode = safe_link>
struct derivation_value_traits;

template<class NodeTraits, link_mode_type LinkMode = normal_link>
struct trivial_value_traits;

//Additional utilities

template<typename VoidPointer, std::size_t Alignment>
struct max_pointer_plus_bits;

template<std::size_t Alignment>
struct max_pointer_plus_bits<void *, Alignment>;

template<typename Pointer, std::size_t NumBits>
struct pointer_plus_bits;

template<typename T, std::size_t NumBits>
struct pointer_plus_bits<T *, NumBits>;

template<typename Ptr>
struct pointer_traits;

template<typename T>
struct pointer_traits<T *>;

}  //namespace intrusive {
}  //namespace bho {

#endif   //#if !defined(BHO_INTRUSIVE_DOXYGEN_INVOKED)

#endif   //#ifndef BHO_INTRUSIVE_FWD_HPP
