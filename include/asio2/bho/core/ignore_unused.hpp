// Copyright (c) 2014 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BHO_CORE_IGNORE_UNUSED_HPP
#define BHO_CORE_IGNORE_UNUSED_HPP

#include <asio2/bho/config.hpp>

namespace bho {

#if !defined(BHO_NO_CXX11_VARIADIC_TEMPLATES)

#if !defined(BHO_NO_CXX11_RVALUE_REFERENCES)

template <typename... Ts>
BHO_FORCEINLINE BHO_CXX14_CONSTEXPR void ignore_unused(Ts&& ...)
{}

#else

template <typename... Ts>
BHO_FORCEINLINE BHO_CXX14_CONSTEXPR void ignore_unused(Ts const& ...)
{}

#endif

template <typename... Ts>
BHO_FORCEINLINE BHO_CXX14_CONSTEXPR void ignore_unused()
{}

#else // !defined(BHO_NO_CXX11_VARIADIC_TEMPLATES)

template <typename T1>
BHO_FORCEINLINE BHO_CXX14_CONSTEXPR void ignore_unused(T1&)
{}

template <typename T1>
BHO_FORCEINLINE BHO_CXX14_CONSTEXPR void ignore_unused(T1 const&)
{}

template <typename T1, typename T2>
BHO_FORCEINLINE BHO_CXX14_CONSTEXPR void ignore_unused(T1&, T2&)
{}

template <typename T1, typename T2>
BHO_FORCEINLINE BHO_CXX14_CONSTEXPR void ignore_unused(T1 const&, T2 const&)
{}

template <typename T1, typename T2, typename T3>
BHO_FORCEINLINE BHO_CXX14_CONSTEXPR void ignore_unused(T1&, T2&, T3&)
{}

template <typename T1, typename T2, typename T3>
BHO_FORCEINLINE BHO_CXX14_CONSTEXPR void ignore_unused(T1 const&, T2 const&, T3 const&)
{}

template <typename T1, typename T2, typename T3, typename T4>
BHO_FORCEINLINE BHO_CXX14_CONSTEXPR void ignore_unused(T1&, T2&, T3&, T4&)
{}

template <typename T1, typename T2, typename T3, typename T4>
BHO_FORCEINLINE BHO_CXX14_CONSTEXPR void ignore_unused(T1 const&, T2 const&, T3 const&, T4 const&)
{}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
BHO_FORCEINLINE BHO_CXX14_CONSTEXPR void ignore_unused(T1&, T2&, T3&, T4&, T5&)
{}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
BHO_FORCEINLINE BHO_CXX14_CONSTEXPR void ignore_unused(T1 const&, T2 const&, T3 const&, T4 const&, T5 const&)
{}

template <typename T1>
BHO_FORCEINLINE BHO_CXX14_CONSTEXPR void ignore_unused()
{}

template <typename T1, typename T2>
BHO_FORCEINLINE BHO_CXX14_CONSTEXPR void ignore_unused()
{}

template <typename T1, typename T2, typename T3>
BHO_FORCEINLINE BHO_CXX14_CONSTEXPR void ignore_unused()
{}

template <typename T1, typename T2, typename T3, typename T4>
BHO_FORCEINLINE BHO_CXX14_CONSTEXPR void ignore_unused()
{}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
BHO_FORCEINLINE BHO_CXX14_CONSTEXPR void ignore_unused()
{}

#endif

} // namespace bho

#endif // BHO_CORE_IGNORE_UNUSED_HPP
