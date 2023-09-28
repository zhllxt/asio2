//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2015-2015. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BHO_INTRUSIVE_DETAIL_TREE_VALUE_COMPARE_HPP
#define BHO_INTRUSIVE_DETAIL_TREE_VALUE_COMPARE_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/intrusive/detail/workaround.hpp>
#include <asio2/bho/intrusive/detail/mpl.hpp>
#include <asio2/bho/intrusive/detail/ebo_functor_holder.hpp>
#include <asio2/bho/intrusive/pointer_traits.hpp>

namespace bho{
namespace intrusive{

//Needed to support smart references to value types
template <class From, class ValuePtr>
struct disable_if_smartref_to
   : detail::disable_if_c 
      <  detail::is_same
            <From, typename pointer_traits
               <ValuePtr>
                  ::reference>::value
      || detail::is_same
            <From, typename pointer_traits
                     < typename pointer_rebind
                           < ValuePtr
                           , const typename bho::movelib::pointer_element<ValuePtr>::type>::type>
                  ::reference>::value
      >
{};

//This function object takes a KeyCompare function object
//and compares values that contains keys using KeyOfValue
template< class ValuePtr, class KeyCompare, class KeyOfValue, class Ret = bool
        , bool = bho::intrusive::detail::is_same
   <typename bho::movelib::pointer_element<ValuePtr>::type, typename KeyOfValue::type>::value >
struct tree_value_compare
   :  public bho::intrusive::detail::ebo_functor_holder<KeyCompare>
{
   typedef typename
      bho::movelib::pointer_element<ValuePtr>::type value_type;
   typedef KeyCompare                                 key_compare;
   typedef KeyOfValue                                 key_of_value;
   typedef typename KeyOfValue::type                  key_type;

   typedef bho::intrusive::detail::ebo_functor_holder<KeyCompare> base_t;

   BHO_INTRUSIVE_FORCEINLINE tree_value_compare()
      :  base_t()
   {}

   BHO_INTRUSIVE_FORCEINLINE explicit tree_value_compare(const key_compare &kcomp)
      :  base_t(kcomp)
   {}

   BHO_INTRUSIVE_FORCEINLINE tree_value_compare (const tree_value_compare &x)
      :  base_t(x.base_t::get())
   {}

   BHO_INTRUSIVE_FORCEINLINE tree_value_compare &operator=(const tree_value_compare &x)
   {  this->base_t::get() = x.base_t::get();   return *this;  }

   BHO_INTRUSIVE_FORCEINLINE tree_value_compare &operator=(const key_compare &x)
   {  this->base_t::get() = x;   return *this;  }

   BHO_INTRUSIVE_FORCEINLINE const key_compare &key_comp() const
   {  return static_cast<const key_compare &>(*this);  }

   BHO_INTRUSIVE_FORCEINLINE Ret operator()(const key_type &key) const
   {  return this->key_comp()(key);   }

   BHO_INTRUSIVE_FORCEINLINE Ret operator()(const value_type &value) const
   {  return this->key_comp()(KeyOfValue()(value));  }

   template<class U>
   BHO_INTRUSIVE_FORCEINLINE Ret operator()( const U &nonkey
                                             , typename disable_if_smartref_to<U, ValuePtr>::type* = 0) const
   {  return this->key_comp()(nonkey);  }

   BHO_INTRUSIVE_FORCEINLINE Ret operator()(const key_type &key1, const key_type &key2) const
   {  return this->key_comp()(key1, key2);  }

   BHO_INTRUSIVE_FORCEINLINE Ret operator()(const value_type &value1, const value_type &value2) const
   {  return this->key_comp()(KeyOfValue()(value1), KeyOfValue()(value2));  }

   BHO_INTRUSIVE_FORCEINLINE Ret operator()(const key_type &key1, const value_type &value2) const
   {  return this->key_comp()(key1, KeyOfValue()(value2));  }

   BHO_INTRUSIVE_FORCEINLINE Ret operator()(const value_type &value1, const key_type &key2) const
   {  return this->key_comp()(KeyOfValue()(value1), key2);  }

   template<class U>
   BHO_INTRUSIVE_FORCEINLINE Ret operator()( const key_type &key1, const U &nonkey2
                                              , typename disable_if_smartref_to<U, ValuePtr>::type* = 0) const
   {  return this->key_comp()(key1, nonkey2);  }

   template<class U>
   BHO_INTRUSIVE_FORCEINLINE Ret operator()( const U &nonkey1, const key_type &key2
                                              , typename disable_if_smartref_to<U, ValuePtr>::type* = 0) const
   {  return this->key_comp()(nonkey1, key2);  }

   template<class U>
   BHO_INTRUSIVE_FORCEINLINE Ret operator()( const value_type &value1, const U &nonvalue2
                                              , typename disable_if_smartref_to<U, ValuePtr>::type* = 0) const
   {  return this->key_comp()(KeyOfValue()(value1), nonvalue2);  }

   template<class U>
   BHO_INTRUSIVE_FORCEINLINE Ret operator()( const U &nonvalue1, const value_type &value2
                                              , typename disable_if_smartref_to<U, ValuePtr>::type* = 0) const
   {  return this->key_comp()(nonvalue1, KeyOfValue()(value2));  }
};

template<class ValuePtr, class KeyCompare, class KeyOfValue, class Ret>
struct tree_value_compare<ValuePtr, KeyCompare, KeyOfValue, Ret, true>
   :  public bho::intrusive::detail::ebo_functor_holder<KeyCompare>
{
   typedef typename
      bho::movelib::pointer_element<ValuePtr>::type value_type;
   typedef KeyCompare                                 key_compare;
   typedef KeyOfValue                                 key_of_value;
   typedef typename KeyOfValue::type                  key_type;

   typedef bho::intrusive::detail::ebo_functor_holder<KeyCompare> base_t;


   BHO_INTRUSIVE_FORCEINLINE tree_value_compare()
      :  base_t()
   {}

   BHO_INTRUSIVE_FORCEINLINE explicit tree_value_compare(const key_compare &kcomp)
      :  base_t(kcomp)
   {}

   BHO_INTRUSIVE_FORCEINLINE tree_value_compare (const tree_value_compare &x)
      :  base_t(x.base_t::get())
   {}

   BHO_INTRUSIVE_FORCEINLINE tree_value_compare &operator=(const tree_value_compare &x)
   {  this->base_t::get() = x.base_t::get();   return *this;  }

   BHO_INTRUSIVE_FORCEINLINE tree_value_compare &operator=(const key_compare &x)
   {  this->base_t::get() = x;   return *this;  }

   BHO_INTRUSIVE_FORCEINLINE const key_compare &key_comp() const
   {  return static_cast<const key_compare &>(*this);  }

   BHO_INTRUSIVE_FORCEINLINE Ret operator()(const key_type &key) const
   {  return this->key_comp()(key);   }

   template<class U>
   BHO_INTRUSIVE_FORCEINLINE Ret operator()( const U &nonkey
                                             , typename disable_if_smartref_to<U, ValuePtr>::type* = 0) const
   {  return this->key_comp()(nonkey);  }

   BHO_INTRUSIVE_FORCEINLINE Ret operator()(const key_type &key1, const key_type &key2) const
   {  return this->key_comp()(key1, key2);  }

   template<class U>
   BHO_INTRUSIVE_FORCEINLINE Ret operator()( const key_type &key1, const U &nonkey2
                                              , typename disable_if_smartref_to<U, ValuePtr>::type* = 0) const
   {  return this->key_comp()(key1, nonkey2);  }

   template<class U>
   BHO_INTRUSIVE_FORCEINLINE Ret operator()(const U &nonkey1, const key_type &key2
                                              , typename disable_if_smartref_to<U, ValuePtr>::type* = 0) const
   {  return this->key_comp()(nonkey1, key2);  }
};

}  //namespace intrusive{
}  //namespace bho{

#endif   //#ifdef BHO_INTRUSIVE_DETAIL_TREE_VALUE_COMPARE_HPP
