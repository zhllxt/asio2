/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga  2014-2014
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef BHO_INTRUSIVE_DETAIL_SIZE_HOLDER_HPP
#define BHO_INTRUSIVE_DETAIL_SIZE_HOLDER_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/intrusive/detail/workaround.hpp>

namespace bho {
namespace intrusive {
namespace detail {

template<bool ConstantSize, class SizeType, class Tag = void>
struct size_holder
{
   static const bool constant_time_size = ConstantSize;
   typedef SizeType  size_type;

   BHO_INTRUSIVE_FORCEINLINE SizeType get_size() const
   {  return size_;  }

   BHO_INTRUSIVE_FORCEINLINE void set_size(SizeType size)
   {  size_ = size; }

   BHO_INTRUSIVE_FORCEINLINE void decrement()
   {  --size_; }

   BHO_INTRUSIVE_FORCEINLINE void increment()
   {  ++size_; }

   BHO_INTRUSIVE_FORCEINLINE void increase(SizeType n)
   {  size_ += n; }

   BHO_INTRUSIVE_FORCEINLINE void decrease(SizeType n)
   {  size_ -= n; }

   BHO_INTRUSIVE_FORCEINLINE void swap(size_holder &other)
   {  SizeType tmp(size_); size_ = other.size_; other.size_ = tmp; }

   SizeType size_;
};

template<class SizeType, class Tag>
struct size_holder<false, SizeType, Tag>
{
   static const bool constant_time_size = false;
   typedef SizeType  size_type;

   BHO_INTRUSIVE_FORCEINLINE size_type get_size() const
   {  return 0;  }

   BHO_INTRUSIVE_FORCEINLINE void set_size(size_type)
   {}

   BHO_INTRUSIVE_FORCEINLINE void decrement()
   {}

   BHO_INTRUSIVE_FORCEINLINE void increment()
   {}

   BHO_INTRUSIVE_FORCEINLINE void increase(SizeType)
   {}

   BHO_INTRUSIVE_FORCEINLINE void decrease(SizeType)
   {}

   BHO_INTRUSIVE_FORCEINLINE void swap(size_holder){}
};

}  //namespace detail{
}  //namespace intrusive{
}  //namespace bho{

#endif //BHO_INTRUSIVE_DETAIL_SIZE_HOLDER_HPP
