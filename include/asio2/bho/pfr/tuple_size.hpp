// Copyright (c) 2016-2023 Antony Polukhin
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#ifndef BHO_PFR_TUPLE_SIZE_HPP
#define BHO_PFR_TUPLE_SIZE_HPP
#pragma once

#include <asio2/bho/pfr/detail/config.hpp>

#include <type_traits>
#include <utility>      // metaprogramming stuff

#include <asio2/bho/pfr/detail/sequence_tuple.hpp>
#include <asio2/bho/pfr/detail/fields_count.hpp>

/// \file bho/pfr/tuple_size.hpp
/// Contains tuple-like interfaces to get fields count \forcedlink{tuple_size}, \forcedlink{tuple_size_v}.
///
/// \b Synopsis:
namespace bho { namespace pfr {

/// Has a static const member variable `value` that contains fields count in a T.
/// Works for any T that satisfies \aggregate.
///
/// \b Example:
/// \code
///     std::array<int, bho::pfr::tuple_size<my_structure>::value > a;
/// \endcode
template <class T>
using tuple_size = detail::size_t_< bho::pfr::detail::fields_count<T>() >;


/// `tuple_size_v` is a template variable that contains fields count in a T and
/// works for any T that satisfies \aggregate.
///
/// \b Example:
/// \code
///     std::array<int, bho::pfr::tuple_size_v<my_structure> > a;
/// \endcode
template <class T>
constexpr std::size_t tuple_size_v = tuple_size<T>::value;

}} // namespace bho::pfr

#endif // BHO_PFR_TUPLE_SIZE_HPP
