/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga  2006-2013
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef BHO_INTRUSIVE_TRIVIAL_VALUE_TRAITS_HPP
#define BHO_INTRUSIVE_TRIVIAL_VALUE_TRAITS_HPP

#include <asio2/bho/intrusive/detail/config_begin.hpp>
#include <asio2/bho/intrusive/detail/workaround.hpp>
#include <asio2/bho/intrusive/intrusive_fwd.hpp>
#include <asio2/bho/intrusive/link_mode.hpp>
#include <asio2/bho/intrusive/pointer_traits.hpp>

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

namespace bho {
namespace intrusive {

//!This value traits template is used to create value traits
//!from user defined node traits where value_traits::value_type and
//!node_traits::node should be equal
template<class NodeTraits, link_mode_type LinkMode
   #ifdef BHO_INTRUSIVE_DOXYGEN_INVOKED
   = safe_link
   #endif
>
struct trivial_value_traits
{
   typedef NodeTraits                                          node_traits;
   typedef typename node_traits::node_ptr                      node_ptr;
   typedef typename node_traits::const_node_ptr                const_node_ptr;
   typedef typename node_traits::node                          value_type;
   typedef node_ptr                                            pointer;
   typedef const_node_ptr                                      const_pointer;
   static const link_mode_type link_mode = LinkMode;
   BHO_INTRUSIVE_FORCEINLINE static node_ptr       to_node_ptr (value_type &value) BHO_NOEXCEPT
      {  return pointer_traits<node_ptr>::pointer_to(value);  }
   BHO_INTRUSIVE_FORCEINLINE static const_node_ptr to_node_ptr (const value_type &value) BHO_NOEXCEPT
      {  return pointer_traits<const_node_ptr>::pointer_to(value);  }
   BHO_INTRUSIVE_FORCEINLINE static pointer  to_value_ptr(node_ptr n) BHO_NOEXCEPT
      {  return n; }
   BHO_INTRUSIVE_FORCEINLINE static const_pointer  to_value_ptr(const_node_ptr n) BHO_NOEXCEPT
      {  return n; }
};

} //namespace intrusive
} //namespace bho

#include <asio2/bho/intrusive/detail/config_end.hpp>

#endif //BHO_INTRUSIVE_TRIVIAL_VALUE_TRAITS_HPP
