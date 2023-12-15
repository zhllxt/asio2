#ifndef BHO_ENDIAN_DETAIL_IS_TRIVIALLY_COPYABLE_HPP_INCLUDED
#define BHO_ENDIAN_DETAIL_IS_TRIVIALLY_COPYABLE_HPP_INCLUDED

// Copyright 2019, 2023 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#include <asio2/bho/config.hpp>
#include <type_traits>

namespace bho
{
namespace endian
{
namespace detail
{

#if defined( BHO_LIBSTDCXX_VERSION ) && BHO_LIBSTDCXX_VERSION < 50000

template<class T> struct is_trivially_copyable: std::integral_constant<bool,
    __has_trivial_copy(T) && __has_trivial_assign(T) && __has_trivial_destructor(T)> {};

#else

using std::is_trivially_copyable;

#endif

} // namespace detail
} // namespace endian
} // namespace bho

#endif  // BHO_ENDIAN_DETAIL_IS_TRIVIALLY_COPYABLE_HPP_INCLUDED
