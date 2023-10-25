#ifndef BHO_CORE_DETAIL_IS_SAME_HPP_INCLUDED
#define BHO_CORE_DETAIL_IS_SAME_HPP_INCLUDED

// is_same<T1,T2>::value is true when T1 == T2
//
// Copyright 2014 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt

#include <asio2/bho/config.hpp>

#if defined(BHO_HAS_PRAGMA_ONCE)
# pragma once
#endif

namespace bho
{
namespace core
{
namespace detail
{

template< class T1, class T2 > struct is_same
{
    BHO_STATIC_CONSTANT( bool, value = false );
};

template< class T > struct is_same< T, T >
{
    BHO_STATIC_CONSTANT( bool, value = true );
};

} // namespace detail
} // namespace core
} // namespace bho

#endif // #ifndef BHO_CORE_DETAIL_IS_SAME_HPP_INCLUDED
