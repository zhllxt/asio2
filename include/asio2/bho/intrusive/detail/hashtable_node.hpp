/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga  2007-2014
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef BHO_INTRUSIVE_HASHTABLE_NODE_HPP
#define BHO_INTRUSIVE_HASHTABLE_NODE_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/intrusive/detail/workaround.hpp>
#include <asio2/bho/intrusive/detail/assert.hpp>
#include <asio2/bho/intrusive/pointer_traits.hpp>
#include <asio2/bho/intrusive/detail/mpl.hpp>
#include <asio2/bho/intrusive/trivial_value_traits.hpp>
#include <asio2/bho/intrusive/slist.hpp> //make_slist
#include <cstddef>
#include <climits>
#include <asio2/bho/move/core.hpp>


namespace bho {
namespace intrusive {

template <class Slist>
struct bucket_impl : public Slist
{
   typedef Slist slist_type;
   BHO_INTRUSIVE_FORCEINLINE bucket_impl()
   {}

   BHO_INTRUSIVE_FORCEINLINE bucket_impl(const bucket_impl &)
   {}

   BHO_INTRUSIVE_FORCEINLINE ~bucket_impl()
   {
      //This bucket is still being used!
      BHO_INTRUSIVE_INVARIANT_ASSERT(Slist::empty());
   }

   BHO_INTRUSIVE_FORCEINLINE bucket_impl &operator=(const bucket_impl&)
   {
      //This bucket is still in use!
      BHO_INTRUSIVE_INVARIANT_ASSERT(Slist::empty());
      return *this;
   }
};

template<class Slist>
struct bucket_traits_impl
{
   private:
   BHO_COPYABLE_AND_MOVABLE(bucket_traits_impl)

   public:
   /// @cond

   typedef typename pointer_traits
      <typename Slist::pointer>::template rebind_pointer
         < bucket_impl<Slist> >::type                                bucket_ptr;
   typedef Slist slist;
   typedef typename Slist::size_type size_type;
   /// @endcond

   BHO_INTRUSIVE_FORCEINLINE bucket_traits_impl(bucket_ptr buckets, size_type len)
      :  buckets_(buckets), buckets_len_(len)
   {}

   BHO_INTRUSIVE_FORCEINLINE bucket_traits_impl(const bucket_traits_impl &x)
      : buckets_(x.buckets_), buckets_len_(x.buckets_len_)
   {}

   BHO_INTRUSIVE_FORCEINLINE bucket_traits_impl(BHO_RV_REF(bucket_traits_impl) x)
      : buckets_(x.buckets_), buckets_len_(x.buckets_len_)
   {  x.buckets_ = bucket_ptr();   x.buckets_len_ = 0;  }

   BHO_INTRUSIVE_FORCEINLINE bucket_traits_impl& operator=(BHO_RV_REF(bucket_traits_impl) x)
   {
      buckets_ = x.buckets_; buckets_len_ = x.buckets_len_;
      x.buckets_ = bucket_ptr();   x.buckets_len_ = 0; return *this;
   }

   BHO_INTRUSIVE_FORCEINLINE bucket_traits_impl& operator=(BHO_COPY_ASSIGN_REF(bucket_traits_impl) x)
   {
      buckets_ = x.buckets_;  buckets_len_ = x.buckets_len_; return *this;
   }

   BHO_INTRUSIVE_FORCEINLINE const bucket_ptr &bucket_begin() const
   {  return buckets_;  }

   BHO_INTRUSIVE_FORCEINLINE size_type  bucket_count() const BHO_NOEXCEPT
   {  return buckets_len_;  }

   private:
   bucket_ptr  buckets_;
   size_type   buckets_len_;
};

template <class NodeTraits>
struct hash_reduced_slist_node_traits
{
   template <class U> static detail::no_type test(...);
   template <class U> static detail::yes_type test(typename U::reduced_slist_node_traits*);
   static const bool value = sizeof(test<NodeTraits>(0)) == sizeof(detail::yes_type);
};

template <class NodeTraits>
struct apply_reduced_slist_node_traits
{
   typedef typename NodeTraits::reduced_slist_node_traits type;
};

template <class NodeTraits>
struct reduced_slist_node_traits
{
   typedef typename detail::eval_if_c
      < hash_reduced_slist_node_traits<NodeTraits>::value
      , apply_reduced_slist_node_traits<NodeTraits>
      , detail::identity<NodeTraits>
      >::type type;
};

template<class NodeTraits>
struct get_slist_impl
{
   typedef trivial_value_traits<NodeTraits, normal_link> trivial_traits;

   //Reducing symbol length
   struct type : make_slist
      < typename NodeTraits::node
      , bho::intrusive::value_traits<trivial_traits>
      , bho::intrusive::constant_time_size<false>
      , bho::intrusive::size_type<std::size_t>
      >::type
   {};
};

template<class BucketValueTraits, bool IsConst>
class hashtable_iterator
{
   typedef typename BucketValueTraits::value_traits            value_traits;
   typedef typename BucketValueTraits::bucket_traits           bucket_traits;

   typedef iiterator< value_traits, IsConst
                    , std::forward_iterator_tag>   types_t;
   public:
   typedef typename types_t::iterator_type::difference_type    difference_type;
   typedef typename types_t::iterator_type::value_type         value_type;
   typedef typename types_t::iterator_type::pointer            pointer;
   typedef typename types_t::iterator_type::reference          reference;
   typedef typename types_t::iterator_type::iterator_category  iterator_category;

   private:
   typedef typename value_traits::node_traits                  node_traits;
   typedef typename node_traits::node_ptr                      node_ptr;
   typedef typename get_slist_impl
      < typename reduced_slist_node_traits
         <node_traits>::type >::type                           slist_impl;
   typedef typename slist_impl::iterator                       siterator;
   typedef typename slist_impl::const_iterator                 const_siterator;
   typedef bucket_impl<slist_impl>                             bucket_type;

   typedef typename pointer_traits
      <pointer>::template rebind_pointer
         < const BucketValueTraits >::type                     const_bucketvaltraits_ptr;
   typedef typename slist_impl::size_type                      size_type;
   class nat;
   typedef typename
      detail::if_c< IsConst
                  , hashtable_iterator<BucketValueTraits, false>
                  , nat>::type                                 nonconst_iterator;

   BHO_INTRUSIVE_FORCEINLINE static node_ptr downcast_bucket(typename bucket_type::node_ptr p)
   {
      return pointer_traits<node_ptr>::
         pointer_to(static_cast<typename node_traits::node&>(*p));
   }

   public:

   BHO_INTRUSIVE_FORCEINLINE hashtable_iterator ()
      : slist_it_()  //Value initialization to achieve "null iterators" (N3644)
   {}

   BHO_INTRUSIVE_FORCEINLINE explicit hashtable_iterator(siterator ptr, const BucketValueTraits *cont)
      : slist_it_ (ptr)
      , traitsptr_ (cont ? pointer_traits<const_bucketvaltraits_ptr>::pointer_to(*cont) : const_bucketvaltraits_ptr() )
   {}

   BHO_INTRUSIVE_FORCEINLINE hashtable_iterator(const hashtable_iterator &other)
      :  slist_it_(other.slist_it()), traitsptr_(other.get_bucket_value_traits())
   {}

   BHO_INTRUSIVE_FORCEINLINE hashtable_iterator(const nonconst_iterator &other)
      :  slist_it_(other.slist_it()), traitsptr_(other.get_bucket_value_traits())
   {}

   BHO_INTRUSIVE_FORCEINLINE const siterator &slist_it() const
   { return slist_it_; }

   BHO_INTRUSIVE_FORCEINLINE hashtable_iterator<BucketValueTraits, false> unconst() const
   {  return hashtable_iterator<BucketValueTraits, false>(this->slist_it(), this->get_bucket_value_traits());   }

   BHO_INTRUSIVE_FORCEINLINE hashtable_iterator& operator++()
   {  this->increment();   return *this;   }

   BHO_INTRUSIVE_FORCEINLINE hashtable_iterator &operator=(const hashtable_iterator &other)
   {  slist_it_ = other.slist_it(); traitsptr_ = other.get_bucket_value_traits();   return *this;  }

   BHO_INTRUSIVE_FORCEINLINE hashtable_iterator operator++(int)
   {
      hashtable_iterator result (*this);
      this->increment();
      return result;
   }

   BHO_INTRUSIVE_FORCEINLINE friend bool operator== (const hashtable_iterator& i, const hashtable_iterator& i2)
   { return i.slist_it_ == i2.slist_it_; }

   BHO_INTRUSIVE_FORCEINLINE friend bool operator!= (const hashtable_iterator& i, const hashtable_iterator& i2)
   { return !(i == i2); }

   BHO_INTRUSIVE_FORCEINLINE reference operator*() const
   { return *this->operator ->(); }

   BHO_INTRUSIVE_FORCEINLINE pointer operator->() const
   {
      return this->priv_value_traits().to_value_ptr
         (downcast_bucket(slist_it_.pointed_node()));
   }

   BHO_INTRUSIVE_FORCEINLINE const const_bucketvaltraits_ptr &get_bucket_value_traits() const
   {  return traitsptr_;  }

   BHO_INTRUSIVE_FORCEINLINE const value_traits &priv_value_traits() const
   {  return traitsptr_->priv_value_traits();  }

   BHO_INTRUSIVE_FORCEINLINE const bucket_traits &priv_bucket_traits() const
   {  return traitsptr_->priv_bucket_traits();  }

   private:
   void increment()
   {
      const bucket_traits &rbuck_traits = this->priv_bucket_traits();
      bucket_type* const buckets = bho::movelib::to_raw_pointer(rbuck_traits.bucket_begin());
      const size_type buckets_len = rbuck_traits.bucket_count();

      ++slist_it_;
      const typename slist_impl::node_ptr n = slist_it_.pointed_node();
      const siterator first_bucket_bbegin = buckets->end();
      if(first_bucket_bbegin.pointed_node() <= n && n <= buckets[buckets_len-1].cend().pointed_node()){
         //If one-past the node is inside the bucket then look for the next non-empty bucket
         //1. get the bucket_impl from the iterator
         const bucket_type &b = static_cast<const bucket_type&>
            (bucket_type::slist_type::container_from_end_iterator(slist_it_));

         //2. Now just calculate the index b has in the bucket array
         size_type n_bucket = static_cast<size_type>(&b - buckets);

         //3. Iterate until a non-empty bucket is found
         do{
            if (++n_bucket >= buckets_len){  //bucket overflow, return end() iterator
               slist_it_ = buckets->before_begin();
               return;
            }
         }
         while (buckets[n_bucket].empty());
         slist_it_ = buckets[n_bucket].begin();
      }
      else{
         //++slist_it_ yield to a valid object
      }
   }

   siterator                  slist_it_;
   const_bucketvaltraits_ptr  traitsptr_;
};

}  //namespace intrusive {
}  //namespace bho {

#endif
