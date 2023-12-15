// Copyright (C) 2007, 2008 Steven Watanabe, Joseph Gauterin, Niels Dekker
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// For more information, see http://www.boost.org


#ifndef BHO_CORE_SWAP_HPP
#define BHO_CORE_SWAP_HPP

// Note: the implementation of this utility contains various workarounds:
// - bho::swap has two template arguments, instead of one, to
// avoid ambiguity when swapping objects of a Boost type that does
// not have its own bho::swap overload.

#include <asio2/bho/core/enable_if.hpp>
#include <asio2/bho/config.hpp>
#include <asio2/bho/config/header_deprecated.hpp>
#include <asio2/bho/core/invoke_swap.hpp>

#ifdef BHO_HAS_PRAGMA_ONCE
#pragma once
#endif

BHO_HEADER_DEPRECATED("asio2/bho/core/invoke_swap.hpp")

namespace bho
{
  template<class T1, class T2>
  BHO_GPU_ENABLED
  BHO_DEPRECATED("This function is deprecated, use bho::core::invoke_swap instead.")
  inline typename enable_if_c< !bho_swap_impl::is_const<T1>::value && !bho_swap_impl::is_const<T2>::value >::type
  swap(T1& left, T2& right)
  {
    bho::core::invoke_swap(left, right);
  }
}

#endif // BHO_CORE_SWAP_HPP
