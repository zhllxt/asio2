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

#ifndef BHO_INTRUSIVE_DETAIL_EMPTY_NODE_CHECKER_HPP
#define BHO_INTRUSIVE_DETAIL_EMPTY_NODE_CHECKER_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

namespace bho {
namespace intrusive {
namespace detail {

template<class ValueTraits>
struct empty_node_checker
{
   typedef ValueTraits                             value_traits;
   typedef typename value_traits::node_traits      node_traits;
   typedef typename node_traits::const_node_ptr    const_node_ptr;

   struct return_type {};

   void operator () (const_node_ptr, const return_type&, const return_type&, return_type&) {}
};

}  //namespace detail{
}  //namespace intrusive{
}  //namespace bho{

#endif //BHO_INTRUSIVE_DETAIL_EMPTY_NODE_CHECKER_HPP
