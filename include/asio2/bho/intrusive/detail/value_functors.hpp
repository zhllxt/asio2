#ifndef BHO_INTRUSIVE_DETAIL_VALUE_FUNCTORS_HPP
#define BHO_INTRUSIVE_DETAIL_VALUE_FUNCTORS_HPP
///////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2017-2021. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <cstddef>

namespace bho {
namespace intrusive {

//Functors for member algorithm defaults
template<class ValueType>
struct value_less
{
   bool operator()(const ValueType &a, const ValueType &b) const
      {  return a < b;  }
};

//Functors for member algorithm defaults
template<class T>
struct value_less<T*>
{
   bool operator()(const T *a, const T* b) const
      {  return std::size_t(a) < std::size_t(b);  }
};

template<class ValueType>
struct value_equal
{
   bool operator()(const ValueType &a, const ValueType &b) const
      {  return a == b;  }
};

}  //namespace intrusive {
}  //namespace bho {

#endif   //BHO_INTRUSIVE_DETAIL_VALUE_FUNCTORS_HPP
