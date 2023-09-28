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

#ifndef BHO_INTRUSIVE_DETAIL_UNCAST_HPP
#define BHO_INTRUSIVE_DETAIL_UNCAST_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/intrusive/detail/config_begin.hpp>
#include <asio2/bho/intrusive/pointer_traits.hpp>
#include <asio2/bho/intrusive/detail/mpl.hpp>

namespace bho {
namespace intrusive {
namespace detail {

template<class ConstNodePtr>
struct uncast_types
{
   typedef typename pointer_traits<ConstNodePtr>::element_type element_type;
   typedef typename remove_const<element_type>::type           non_const_type;
   typedef typename pointer_traits<ConstNodePtr>::
      template rebind_pointer<non_const_type>::type            non_const_pointer;
   typedef pointer_traits<non_const_pointer>                   non_const_traits;
};

template<class ConstNodePtr>
static typename uncast_types<ConstNodePtr>::non_const_pointer
   uncast(const ConstNodePtr & ptr)
{
   return uncast_types<ConstNodePtr>::non_const_traits::const_cast_from(ptr);
}

} //namespace detail {
} //namespace intrusive
} //namespace bho

#include <asio2/bho/intrusive/detail/config_end.hpp>

#endif //BHO_INTRUSIVE_DETAIL_UTILITIES_HPP
