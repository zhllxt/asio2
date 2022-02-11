#ifndef BHO_ENDIAN_DETAIL_IS_TRIVIALLY_COPYABLE_HPP_INCLUDED
#define BHO_ENDIAN_DETAIL_IS_TRIVIALLY_COPYABLE_HPP_INCLUDED

// Copyright 2019 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#include <asio2/bho/config.hpp>
#include <type_traits>

#if !defined(BHO_NO_CXX11_HDR_TYPE_TRAITS)
# include <type_traits>
#endif

namespace bho
{
namespace endian
{
namespace detail
{

#if !defined(BHO_NO_CXX11_HDR_TYPE_TRAITS)

using std::is_trivially_copyable;

#else

template<class T> struct is_trivially_copyable: std::integral_constant<bool,
    std::is_trivially_copyable<T>::value && std::is_trivially_assignable<T>::value && std::is_trivially_destructible<T>::value> {};

#endif

} // namespace detail
} // namespace endian
} // namespace bho

#endif  // BHO_ENDIAN_DETAIL_IS_TRIVIALLY_COPYABLE_HPP_INCLUDED
