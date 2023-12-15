#ifndef BHO_ENDIAN_DETAIL_IS_INTEGRAL_HPP_INCLUDED
#define BHO_ENDIAN_DETAIL_IS_INTEGRAL_HPP_INCLUDED

// Copyright 2023 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#include <type_traits>

namespace bho
{
namespace endian
{
namespace detail
{

template<class T> struct is_integral: std::is_integral<T>
{
};

#if defined(__SIZEOF_INT128__) && defined(_MSC_VER)

template<> struct std::is_integral<__int128_t>: std::true_type
{
};

template<> struct std::is_integral<__uint128_t>: std::true_type
{
};

#endif

} // namespace detail
} // namespace endian
} // namespace bho

#endif  // BHO_ENDIAN_DETAIL_IS_INTEGRAL_HPP_INCLUDED
