#ifndef BHO_CORE_IS_SAME_HPP_INCLUDED
#define BHO_CORE_IS_SAME_HPP_INCLUDED

// is_same<T1,T2>::value is true when T1 == T2
//
// Copyright 2014 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt

#include <asio2/bho/config.hpp>
#include <asio2/bho/core/detail/is_same.hpp>

#if defined(BHO_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <asio2/bho/config/header_deprecated.hpp>

#include <type_traits>

namespace bho
{

namespace core
{

using bho::core::detail::is_same;

} // namespace core

} // namespace bho

#endif // #ifndef BHO_CORE_IS_SAME_HPP_INCLUDED
