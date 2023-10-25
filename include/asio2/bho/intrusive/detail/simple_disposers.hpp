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

#ifndef BHO_INTRUSIVE_DETAIL_SIMPLE_DISPOSERS_HPP
#define BHO_INTRUSIVE_DETAIL_SIMPLE_DISPOSERS_HPP

#include <asio2/bho/intrusive/detail/workaround.hpp>

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

namespace bho {
namespace intrusive {
namespace detail {

class null_disposer
{
   public:
   template <class Pointer>
   void operator()(Pointer)
   {}
};

template<class NodeAlgorithms>
class init_disposer
{
   typedef typename NodeAlgorithms::node_ptr node_ptr;

   public:
   BHO_INTRUSIVE_FORCEINLINE void operator()(node_ptr p)
   {  NodeAlgorithms::init(p);   }
};

}  //namespace detail{
}  //namespace intrusive{
}  //namespace bho{

#endif //BHO_INTRUSIVE_DETAIL_SIMPLE_DISPOSERS_HPP
