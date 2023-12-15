// Copyright (C) 2007, 2008 Steven Watanabe, Joseph Gauterin, Niels Dekker
// Copyright (C) 2023 Andrey Semashev
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// For more information, see http://www.boost.org

#ifndef BHO_CORE_INVOKE_SWAP_HPP
#define BHO_CORE_INVOKE_SWAP_HPP

// Note: the implementation of this utility contains various workarounds:
// - invoke_swap_impl is put outside the boost namespace, to avoid infinite
// recursion (causing stack overflow) when swapping objects of a primitive
// type.
// - std::swap is imported with a using-directive, rather than
// a using-declaration, because some compilers (including MSVC 7.1,
// Borland 5.9.3, and Intel 8.1) don't do argument-dependent lookup
// when it has a using-declaration instead.
// - The main entry function is called invoke_swap rather than swap
// to avoid forming an infinite recursion when the arguments are not
// swappable.

#include <asio2/bho/core/enable_if.hpp>
#include <asio2/bho/config.hpp>
#if __cplusplus >= 201103L || defined(BHO_DINKUMWARE_STDLIB)
#include <utility> // for std::swap (C++11)
#else
#include <algorithm> // for std::swap (C++98)
#endif
#include <cstddef> // for std::size_t

#ifdef BHO_HAS_PRAGMA_ONCE
#pragma once
#endif

#if defined(BHO_GCC) && (BHO_GCC < 40700)
// gcc 4.6 ICEs on noexcept specifications below
#define BHO_CORE_SWAP_NOEXCEPT_IF(x)
#else
#define BHO_CORE_SWAP_NOEXCEPT_IF(x) BHO_NOEXCEPT_IF(x)
#endif

namespace bho_swap_impl {

// we can't use type_traits here

template<class T> struct is_const { enum _vt { value = 0 }; };
template<class T> struct is_const<T const> { enum _vt { value = 1 }; };

// Use std::swap if argument dependent lookup fails.
// We need to have this at namespace scope to be able to use unqualified swap() call
// in noexcept specification.
using namespace std;

template<class T>
BHO_GPU_ENABLED
inline void invoke_swap_impl(T& left, T& right) BHO_CORE_SWAP_NOEXCEPT_IF(BHO_NOEXCEPT_EXPR(swap(left, right)))
{
    swap(left, right);
}

template<class T, std::size_t N>
BHO_GPU_ENABLED
inline void invoke_swap_impl(T (& left)[N], T (& right)[N])
    BHO_CORE_SWAP_NOEXCEPT_IF(BHO_NOEXCEPT_EXPR(::bho_swap_impl::invoke_swap_impl(left[0], right[0])))
{
    for (std::size_t i = 0; i < N; ++i)
    {
        ::bho_swap_impl::invoke_swap_impl(left[i], right[i]);
    }
}

} // namespace bho_swap_impl

namespace bho {
namespace core {

template<class T>
BHO_GPU_ENABLED
inline typename enable_if_c< !::bho_swap_impl::is_const<T>::value >::type
invoke_swap(T& left, T& right)
    BHO_CORE_SWAP_NOEXCEPT_IF(BHO_NOEXCEPT_EXPR(::bho_swap_impl::invoke_swap_impl(left, right)))
{
    ::bho_swap_impl::invoke_swap_impl(left, right);
}

} // namespace core
} // namespace bho

#undef BHO_CORE_SWAP_NOEXCEPT_IF

#endif // BHO_CORE_INVOKE_SWAP_HPP
