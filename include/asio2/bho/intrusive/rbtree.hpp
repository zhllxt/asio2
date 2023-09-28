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
#ifndef BHO_INTRUSIVE_RBTREE_HPP
#define BHO_INTRUSIVE_RBTREE_HPP

#include <asio2/bho/intrusive/detail/config_begin.hpp>
#include <asio2/bho/intrusive/intrusive_fwd.hpp>
#include <cstddef>
#include <asio2/bho/intrusive/detail/minimal_less_equal_header.hpp>
#include <asio2/bho/intrusive/detail/minimal_pair_header.hpp>   //std::pair

#include <asio2/bho/intrusive/set_hook.hpp>
#include <asio2/bho/intrusive/detail/rbtree_node.hpp>
#include <asio2/bho/intrusive/bstree.hpp>
#include <asio2/bho/intrusive/detail/tree_node.hpp>
#include <asio2/bho/intrusive/detail/mpl.hpp>
#include <asio2/bho/intrusive/pointer_traits.hpp>
#include <asio2/bho/intrusive/detail/get_value_traits.hpp>
#include <asio2/bho/intrusive/rbtree_algorithms.hpp>
#include <asio2/bho/intrusive/link_mode.hpp>

#include <asio2/bho/move/utility_core.hpp>
#include <asio2/bho/static_assert.hpp>

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

namespace bho {
namespace intrusive {

/// @cond

struct default_rbtree_hook_applier
{  template <class T> struct apply{ typedef typename T::default_rbtree_hook type;  };  };

template<>
struct is_default_hook_tag<default_rbtree_hook_applier>
{  static const bool value = true;  };

struct rbtree_defaults
   : bstree_defaults
{
   typedef default_rbtree_hook_applier proto_value_traits;
};

/// @endcond

//! The class template rbtree is an intrusive red-black tree container, that
//! is used to construct intrusive set and multiset containers. The no-throw
//! guarantee holds only, if the key_compare object
//! doesn't throw.
//!
//! The template parameter \c T is the type to be managed by the container.
//! The user can specify additional options and if no options are provided
//! default options are used.
//!
//! The container supports the following options:
//! \c base_hook<>/member_hook<>/value_traits<>,
//! \c constant_time_size<>, \c size_type<> and
//! \c compare<>.
#if defined(BHO_INTRUSIVE_DOXYGEN_INVOKED)
template<class T, class ...Options>
#else
template<class ValueTraits, class VoidOrKeyOfValue, class VoidOrKeyComp, class SizeType, bool ConstantTimeSize, typename HeaderHolder>
#endif
class rbtree_impl
   /// @cond
   :  public bstree_impl<ValueTraits, VoidOrKeyOfValue, VoidOrKeyComp, SizeType, ConstantTimeSize, RbTreeAlgorithms, HeaderHolder>
   /// @endcond
{
   public:
   typedef ValueTraits                                               value_traits;
   /// @cond
   typedef bstree_impl< ValueTraits, VoidOrKeyOfValue, VoidOrKeyComp, SizeType
                      , ConstantTimeSize, RbTreeAlgorithms
                      , HeaderHolder>                                tree_type;
   typedef tree_type                                                 implementation_defined;
   /// @endcond

   typedef typename implementation_defined::pointer                  pointer;
   typedef typename implementation_defined::const_pointer            const_pointer;
   typedef typename implementation_defined::value_type               value_type;
   typedef typename implementation_defined::key_type                 key_type;
   typedef typename implementation_defined::key_of_value             key_of_value;
   typedef typename implementation_defined::reference                reference;
   typedef typename implementation_defined::const_reference          const_reference;
   typedef typename implementation_defined::difference_type          difference_type;
   typedef typename implementation_defined::size_type                size_type;
   typedef typename implementation_defined::value_compare            value_compare;
   typedef typename implementation_defined::key_compare              key_compare;
   typedef typename implementation_defined::iterator                 iterator;
   typedef typename implementation_defined::const_iterator           const_iterator;
   typedef typename implementation_defined::reverse_iterator         reverse_iterator;
   typedef typename implementation_defined::const_reverse_iterator   const_reverse_iterator;
   typedef typename implementation_defined::node_traits              node_traits;
   typedef typename implementation_defined::node                     node;
   typedef typename implementation_defined::node_ptr                 node_ptr;
   typedef typename implementation_defined::const_node_ptr           const_node_ptr;
   typedef typename implementation_defined::node_algorithms          node_algorithms;

   static const bool constant_time_size = implementation_defined::constant_time_size;
   /// @cond
   private:

   //noncopyable
   BHO_MOVABLE_BUT_NOT_COPYABLE(rbtree_impl)

   /// @endcond

   public:

   typedef typename implementation_defined::insert_commit_data insert_commit_data;

   //! @copydoc ::bho::intrusive::bstree::bstree()
   rbtree_impl()
      :  tree_type()
   {}

   //! @copydoc ::bho::intrusive::bstree::bstree(const key_compare &,const value_traits &)
   explicit rbtree_impl( const key_compare &cmp, const value_traits &v_traits = value_traits())
      :  tree_type(cmp, v_traits)
   {}

   //! @copydoc ::bho::intrusive::bstree::bstree(bool,Iterator,Iterator,const key_compare &,const value_traits &)
   template<class Iterator>
   rbtree_impl( bool unique, Iterator b, Iterator e
              , const key_compare &cmp     = key_compare()
              , const value_traits &v_traits = value_traits())
      : tree_type(unique, b, e, cmp, v_traits)
   {}

   //! @copydoc ::bho::intrusive::bstree::bstree(bstree &&)
   rbtree_impl(BHO_RV_REF(rbtree_impl) x)
      :  tree_type(BHO_MOVE_BASE(tree_type, x))
   {}

   //! @copydoc ::bho::intrusive::bstree::operator=(bstree &&)
   rbtree_impl& operator=(BHO_RV_REF(rbtree_impl) x)
   {  return static_cast<rbtree_impl&>(tree_type::operator=(BHO_MOVE_BASE(tree_type, x))); }

   #ifdef BHO_INTRUSIVE_DOXYGEN_INVOKED
   //! @copydoc ::bho::intrusive::bstree::~bstree()
   ~rbtree_impl();

   //! @copydoc ::bho::intrusive::bstree::begin()
   iterator begin() BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::begin()const
   const_iterator begin() const BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::cbegin()const
   const_iterator cbegin() const BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::end()
   iterator end() BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::end()const
   const_iterator end() const BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::cend()const
   const_iterator cend() const BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::rbegin()
   reverse_iterator rbegin() BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::rbegin()const
   const_reverse_iterator rbegin() const BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::crbegin()const
   const_reverse_iterator crbegin() const BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::rend()
   reverse_iterator rend() BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::rend()const
   const_reverse_iterator rend() const BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::crend()const
   const_reverse_iterator crend() const BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::root()
   iterator root() BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::root()const
   const_iterator root() const BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::croot()const
   const_iterator croot() const BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::container_from_end_iterator(iterator)
   static rbtree_impl &container_from_end_iterator(iterator end_iterator) BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::container_from_end_iterator(const_iterator)
   static const rbtree_impl &container_from_end_iterator(const_iterator end_iterator) BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::container_from_iterator(iterator)
   static rbtree_impl &container_from_iterator(iterator it) BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::container_from_iterator(const_iterator)
   static const rbtree_impl &container_from_iterator(const_iterator it) BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::key_comp()const
   key_compare key_comp() const;

   //! @copydoc ::bho::intrusive::bstree::value_comp()const
   value_compare value_comp() const;

   //! @copydoc ::bho::intrusive::bstree::empty()const
   bool empty() const BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::size()const
   size_type size() const BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::swap
   void swap(rbtree_impl& other);

   //! @copydoc ::bho::intrusive::bstree::clone_from(const bstree&,Cloner,Disposer)
   template <class Cloner, class Disposer>
   void clone_from(const rbtree_impl &src, Cloner cloner, Disposer disposer);

   #else //BHO_INTRUSIVE_DOXYGEN_INVOKED

   using tree_type::clone_from;

   #endif   //#ifdef BHO_INTRUSIVE_DOXYGEN_INVOKED

   //! @copydoc ::bho::intrusive::bstree::clone_from(bstree&&,Cloner,Disposer)
   template <class Cloner, class Disposer>
   void clone_from(BHO_RV_REF(rbtree_impl) src, Cloner cloner, Disposer disposer)
   {  tree_type::clone_from(BHO_MOVE_BASE(tree_type, src), cloner, disposer);  }

   #ifdef BHO_INTRUSIVE_DOXYGEN_INVOKED

   //! @copydoc ::bho::intrusive::bstree::clone_from(bstree&&,Cloner,Disposer)
   template <class Cloner, class Disposer>
   void clone_from(rbtree_impl &&src, Cloner cloner, Disposer disposer);

   //! @copydoc ::bho::intrusive::bstree::insert_equal(reference)
   iterator insert_equal(reference value);

   //! @copydoc ::bho::intrusive::bstree::insert_equal(const_iterator,reference)
   iterator insert_equal(const_iterator hint, reference value);

   //! @copydoc ::bho::intrusive::bstree::insert_equal(Iterator,Iterator)
   template<class Iterator>
   void insert_equal(Iterator b, Iterator e);

   //! @copydoc ::bho::intrusive::bstree::insert_unique(reference)
   std::pair<iterator, bool> insert_unique(reference value);

   //! @copydoc ::bho::intrusive::bstree::insert_unique(const_iterator,reference)
   iterator insert_unique(const_iterator hint, reference value);

   //! @copydoc ::bho::intrusive::bstree::insert_unique_check(const KeyType&,KeyTypeKeyCompare,insert_commit_data&)
   template<class KeyType, class KeyTypeKeyCompare>
   std::pair<iterator, bool> insert_unique_check
      (const KeyType &key, KeyTypeKeyCompare comp, insert_commit_data &commit_data);

   //! @copydoc ::bho::intrusive::bstree::insert_unique_check(const_iterator,const KeyType&,KeyTypeKeyCompare,insert_commit_data&)
   template<class KeyType, class KeyTypeKeyCompare>
   std::pair<iterator, bool> insert_unique_check
      (const_iterator hint, const KeyType &key
      ,KeyTypeKeyCompare comp, insert_commit_data &commit_data);

   //! @copydoc ::bho::intrusive::bstree::insert_unique_check(const key_type&,insert_commit_data&)
   std::pair<iterator, bool> insert_unique_check
      (const key_type &key, insert_commit_data &commit_data);

   //! @copydoc ::bho::intrusive::bstree::insert_unique_check(const_iterator,const key_type&,insert_commit_data&)
   std::pair<iterator, bool> insert_unique_check
      (const_iterator hint, const key_type &key, insert_commit_data &commit_data);

   //! @copydoc ::bho::intrusive::bstree::insert_unique_commit
   iterator insert_unique_commit(reference value, const insert_commit_data &commit_data) BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::insert_unique(Iterator,Iterator)
   template<class Iterator>
   void insert_unique(Iterator b, Iterator e);

   //! @copydoc ::bho::intrusive::bstree::insert_before
   iterator insert_before(const_iterator pos, reference value) BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::push_back
   void push_back(reference value) BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::push_front
   void push_front(reference value) BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::erase(const_iterator)
   iterator erase(const_iterator i) BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::erase(const_iterator,const_iterator)
   iterator erase(const_iterator b, const_iterator e) BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::erase(const key_type &key)
   size_type erase(const key_type &key);

   //! @copydoc ::bho::intrusive::bstree::erase(const KeyType&,KeyTypeKeyCompare)
   template<class KeyType, class KeyTypeKeyCompare>
   size_type erase(const KeyType& key, KeyTypeKeyCompare comp);

   //! @copydoc ::bho::intrusive::bstree::erase_and_dispose(const_iterator,Disposer)
   template<class Disposer>
   iterator erase_and_dispose(const_iterator i, Disposer disposer) BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::erase_and_dispose(const_iterator,const_iterator,Disposer)
   template<class Disposer>
   iterator erase_and_dispose(const_iterator b, const_iterator e, Disposer disposer) BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::erase_and_dispose(const key_type &, Disposer)
   template<class Disposer>
   size_type erase_and_dispose(const key_type &key, Disposer disposer);

   //! @copydoc ::bho::intrusive::bstree::erase_and_dispose(const KeyType&,KeyTypeKeyCompare,Disposer)
   template<class KeyType, class KeyTypeKeyCompare, class Disposer>
   size_type erase_and_dispose(const KeyType& key, KeyTypeKeyCompare comp, Disposer disposer);

   //! @copydoc ::bho::intrusive::bstree::clear
   void clear() BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::clear_and_dispose
   template<class Disposer>
   void clear_and_dispose(Disposer disposer) BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::count(const key_type &)const
   size_type count(const key_type &key) const;

   //! @copydoc ::bho::intrusive::bstree::count(const KeyType&,KeyTypeKeyCompare)const
   template<class KeyType, class KeyTypeKeyCompare>
   size_type count(const KeyType& key, KeyTypeKeyCompare comp) const;

   //! @copydoc ::bho::intrusive::bstree::lower_bound(const key_type &)
   iterator lower_bound(const key_type &key);

   //! @copydoc ::bho::intrusive::bstree::lower_bound(const KeyType&,KeyTypeKeyCompare)
   template<class KeyType, class KeyTypeKeyCompare>
   iterator lower_bound(const KeyType& key, KeyTypeKeyCompare comp);

   //! @copydoc ::bho::intrusive::bstree::lower_bound(const key_type &)const
   const_iterator lower_bound(const key_type &key) const;

   //! @copydoc ::bho::intrusive::bstree::lower_bound(const KeyType&,KeyTypeKeyCompare)const
   template<class KeyType, class KeyTypeKeyCompare>
   const_iterator lower_bound(const KeyType& key, KeyTypeKeyCompare comp) const;

   //! @copydoc ::bho::intrusive::bstree::upper_bound(const key_type &)
   iterator upper_bound(const key_type &key);

   //! @copydoc ::bho::intrusive::bstree::upper_bound(const KeyType&,KeyTypeKeyCompare)
   template<class KeyType, class KeyTypeKeyCompare>
   iterator upper_bound(const KeyType& key, KeyTypeKeyCompare comp);

   //! @copydoc ::bho::intrusive::bstree::upper_bound(const key_type &)const
   const_iterator upper_bound(const key_type &key) const;

   //! @copydoc ::bho::intrusive::bstree::upper_bound(const KeyType&,KeyTypeKeyCompare)const
   template<class KeyType, class KeyTypeKeyCompare>
   const_iterator upper_bound(const KeyType& key, KeyTypeKeyCompare comp) const;

   //! @copydoc ::bho::intrusive::bstree::find(const key_type &)
   iterator find(const key_type &key);

   //! @copydoc ::bho::intrusive::bstree::find(const KeyType&,KeyTypeKeyCompare)
   template<class KeyType, class KeyTypeKeyCompare>
   iterator find(const KeyType& key, KeyTypeKeyCompare comp);

   //! @copydoc ::bho::intrusive::bstree::find(const key_type &)const
   const_iterator find(const key_type &key) const;

   //! @copydoc ::bho::intrusive::bstree::find(const KeyType&,KeyTypeKeyCompare)const
   template<class KeyType, class KeyTypeKeyCompare>
   const_iterator find(const KeyType& key, KeyTypeKeyCompare comp) const;

   //! @copydoc ::bho::intrusive::bstree::equal_range(const key_type &)
   std::pair<iterator,iterator> equal_range(const key_type &key);

   //! @copydoc ::bho::intrusive::bstree::equal_range(const KeyType&,KeyTypeKeyCompare)
   template<class KeyType, class KeyTypeKeyCompare>
   std::pair<iterator,iterator> equal_range(const KeyType& key, KeyTypeKeyCompare comp);

   //! @copydoc ::bho::intrusive::bstree::equal_range(const key_type &)const
   std::pair<const_iterator, const_iterator>
      equal_range(const key_type &key) const;

   //! @copydoc ::bho::intrusive::bstree::equal_range(const KeyType&,KeyTypeKeyCompare)const
   template<class KeyType, class KeyTypeKeyCompare>
   std::pair<const_iterator, const_iterator>
      equal_range(const KeyType& key, KeyTypeKeyCompare comp) const;

   //! @copydoc ::bho::intrusive::bstree::bounded_range(const key_type &,const key_type &,bool,bool)
   std::pair<iterator,iterator> bounded_range
      (const key_type &lower, const key_type &upper_key, bool left_closed, bool right_closed);

   //! @copydoc ::bho::intrusive::bstree::bounded_range(const KeyType&,const KeyType&,KeyTypeKeyCompare,bool,bool)
   template<class KeyType, class KeyTypeKeyCompare>
   std::pair<iterator,iterator> bounded_range
      (const KeyType& lower_key, const KeyType& upper_key, KeyTypeKeyCompare comp, bool left_closed, bool right_closed);

   //! @copydoc ::bho::intrusive::bstree::bounded_range(const key_type &,const key_type &,bool,bool)const
   std::pair<const_iterator, const_iterator>
      bounded_range(const key_type &lower_key, const key_type &upper_key, bool left_closed, bool right_closed) const;

   //! @copydoc ::bho::intrusive::bstree::bounded_range(const KeyType&,const KeyType&,KeyTypeKeyCompare,bool,bool)const
   template<class KeyType, class KeyTypeKeyCompare>
   std::pair<const_iterator, const_iterator> bounded_range
         (const KeyType& lower_key, const KeyType& upper_key, KeyTypeKeyCompare comp, bool left_closed, bool right_closed) const;

   //! @copydoc ::bho::intrusive::bstree::s_iterator_to(reference)
   static iterator s_iterator_to(reference value) BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::s_iterator_to(const_reference)
   static const_iterator s_iterator_to(const_reference value) BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::iterator_to(reference)
   iterator iterator_to(reference value) BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::iterator_to(const_reference)const
   const_iterator iterator_to(const_reference value) const BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::init_node(reference)
   static void init_node(reference value) BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::unlink_leftmost_without_rebalance
   pointer unlink_leftmost_without_rebalance() BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::replace_node
   void replace_node(iterator replace_this, reference with_this) BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::remove_node
   void remove_node(reference value) BHO_NOEXCEPT;

   //! @copydoc ::bho::intrusive::bstree::merge_unique(bstree<T, Options2...>&)
   template<class T, class ...Options2>
   void merge_unique(rbtree<T, Options2...> &);

   //! @copydoc ::bho::intrusive::bstree::merge_equal(bstree<T, Options2...>&)
   template<class T, class ...Options2>
   void merge_equal(rbtree<T, Options2...> &);

   friend bool operator< (const rbtree_impl &x, const rbtree_impl &y);

   friend bool operator==(const rbtree_impl &x, const rbtree_impl &y);

   friend bool operator!= (const rbtree_impl &x, const rbtree_impl &y);

   friend bool operator>(const rbtree_impl &x, const rbtree_impl &y);

   friend bool operator<=(const rbtree_impl &x, const rbtree_impl &y);

   friend bool operator>=(const rbtree_impl &x, const rbtree_impl &y);

   friend void swap(rbtree_impl &x, rbtree_impl &y);
   #endif   //#ifdef BHO_INTRUSIVE_DOXYGEN_INVOKED
};


//! Helper metafunction to define a \c rbtree that yields to the same type when the
//! same options (either explicitly or implicitly) are used.
#if defined(BHO_INTRUSIVE_DOXYGEN_INVOKED) || defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template<class T, class ...Options>
#else
template<class T, class O1 = void, class O2 = void
                , class O3 = void, class O4 = void
                , class O5 = void, class O6 = void>
#endif
struct make_rbtree
{
   /// @cond
   typedef typename pack_options
      < rbtree_defaults,
      #if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
      O1, O2, O3, O4, O5, O6
      #else
      Options...
      #endif
      >::type packed_options;

   typedef typename detail::get_value_traits
      <T, typename packed_options::proto_value_traits>::type value_traits;

   typedef rbtree_impl
         < value_traits
         , typename packed_options::key_of_value
         , typename packed_options::compare
         , typename packed_options::size_type
         , packed_options::constant_time_size
         , typename packed_options::header_holder_type
         > implementation_defined;
   /// @endcond
   typedef implementation_defined type;
};


#ifndef BHO_INTRUSIVE_DOXYGEN_INVOKED

#if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
template<class T, class O1, class O2, class O3, class O4, class O5, class O6>
#else
template<class T, class ...Options>
#endif
class rbtree
   :  public make_rbtree<T,
      #if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
      O1, O2, O3, O4, O5, O6
      #else
      Options...
      #endif
      >::type
{
   typedef typename make_rbtree
      <T,
      #if !defined(BHO_INTRUSIVE_VARIADIC_TEMPLATES)
      O1, O2, O3, O4, O5, O6
      #else
      Options...
      #endif
      >::type   Base;
   BHO_MOVABLE_BUT_NOT_COPYABLE(rbtree)

   public:
   typedef typename Base::key_compare        key_compare;
   typedef typename Base::value_traits       value_traits;
   typedef typename Base::iterator           iterator;
   typedef typename Base::const_iterator     const_iterator;
   typedef typename Base::reverse_iterator           reverse_iterator;
   typedef typename Base::const_reverse_iterator     const_reverse_iterator;

   //Assert if passed value traits are compatible with the type
   BHO_STATIC_ASSERT((detail::is_same<typename value_traits::value_type, T>::value));

   BHO_INTRUSIVE_FORCEINLINE rbtree()
      :  Base()
   {}

   BHO_INTRUSIVE_FORCEINLINE explicit rbtree( const key_compare &cmp, const value_traits &v_traits = value_traits())
      :  Base(cmp, v_traits)
   {}

   template<class Iterator>
   BHO_INTRUSIVE_FORCEINLINE rbtree( bool unique, Iterator b, Iterator e
         , const key_compare &cmp = key_compare()
         , const value_traits &v_traits = value_traits())
      :  Base(unique, b, e, cmp, v_traits)
   {}

   BHO_INTRUSIVE_FORCEINLINE rbtree(BHO_RV_REF(rbtree) x)
      :  Base(BHO_MOVE_BASE(Base, x))
   {}

   BHO_INTRUSIVE_FORCEINLINE rbtree& operator=(BHO_RV_REF(rbtree) x)
   {  return static_cast<rbtree &>(this->Base::operator=(BHO_MOVE_BASE(Base, x)));  }

   template <class Cloner, class Disposer>
   BHO_INTRUSIVE_FORCEINLINE void clone_from(const rbtree &src, Cloner cloner, Disposer disposer)
   {  Base::clone_from(src, cloner, disposer);  }

   template <class Cloner, class Disposer>
   BHO_INTRUSIVE_FORCEINLINE void clone_from(BHO_RV_REF(rbtree) src, Cloner cloner, Disposer disposer)
   {  Base::clone_from(BHO_MOVE_BASE(Base, src), cloner, disposer);  }

   BHO_INTRUSIVE_FORCEINLINE static rbtree &container_from_end_iterator(iterator end_iterator) BHO_NOEXCEPT
   {  return static_cast<rbtree &>(Base::container_from_end_iterator(end_iterator));   }

   BHO_INTRUSIVE_FORCEINLINE static const rbtree &container_from_end_iterator(const_iterator end_iterator) BHO_NOEXCEPT
   {  return static_cast<const rbtree &>(Base::container_from_end_iterator(end_iterator));   }

   BHO_INTRUSIVE_FORCEINLINE static rbtree &container_from_iterator(iterator it) BHO_NOEXCEPT
   {  return static_cast<rbtree &>(Base::container_from_iterator(it));   }

   BHO_INTRUSIVE_FORCEINLINE static const rbtree &container_from_iterator(const_iterator it) BHO_NOEXCEPT
   {  return static_cast<const rbtree &>(Base::container_from_iterator(it));   }
};

#endif

} //namespace intrusive
} //namespace bho

#include <asio2/bho/intrusive/detail/config_end.hpp>

#endif //BHO_INTRUSIVE_RBTREE_HPP
