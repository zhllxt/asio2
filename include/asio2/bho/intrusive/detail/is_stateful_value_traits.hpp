/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga  2009-2013.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef BHO_INTRUSIVE_DETAIL_IS_STATEFUL_VALUE_TRAITS_HPP
#define BHO_INTRUSIVE_DETAIL_IS_STATEFUL_VALUE_TRAITS_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#if defined(_MSC_VER) && (_MSC_VER <= 1310)

#include <asio2/bho/intrusive/detail/mpl.hpp>

namespace bho {
namespace intrusive {
namespace detail {

template<class ValueTraits>
struct is_stateful_value_traits
{
   static const bool value = !detail::is_empty<ValueTraits>::value;
};

}}}

#else

#include <asio2/bho/intrusive/detail/function_detector.hpp>

BHO_INTRUSIVE_CREATE_FUNCTION_DETECTOR(to_node_ptr, bho_intrusive)
BHO_INTRUSIVE_CREATE_FUNCTION_DETECTOR(to_value_ptr, bho_intrusive)

namespace bho {
namespace intrusive {
namespace detail {

template<class ValueTraits>
struct is_stateful_value_traits
{
   typedef typename ValueTraits::node_ptr       node_ptr;
   typedef typename ValueTraits::pointer        pointer;
   typedef typename ValueTraits::value_type     value_type;
   typedef typename ValueTraits::const_node_ptr const_node_ptr;
   typedef typename ValueTraits::const_pointer  const_pointer;

   typedef ValueTraits value_traits;

   static const bool value =
      (bho::intrusive::function_detector::NonStaticFunction ==
         (BHO_INTRUSIVE_DETECT_FUNCTION(ValueTraits, bho_intrusive, node_ptr, to_node_ptr, (value_type&) )))
      ||
      (bho::intrusive::function_detector::NonStaticFunction ==
         (BHO_INTRUSIVE_DETECT_FUNCTION(ValueTraits, bho_intrusive, pointer, to_value_ptr, (node_ptr) )))
      ||
      (bho::intrusive::function_detector::NonStaticFunction ==
         (BHO_INTRUSIVE_DETECT_FUNCTION(ValueTraits, bho_intrusive, const_node_ptr, to_node_ptr, (const value_type&) )))
      ||
      (bho::intrusive::function_detector::NonStaticFunction ==
         (BHO_INTRUSIVE_DETECT_FUNCTION(ValueTraits, bho_intrusive, const_pointer, to_value_ptr, (const_node_ptr) )))
      ;
};

}}}

#endif

#endif   //@ifndef BHO_INTRUSIVE_DETAIL_IS_STATEFUL_VALUE_TRAITS_HPP
