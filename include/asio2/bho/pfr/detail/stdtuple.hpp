// Copyright (c) 2016-2023 Antony Polukhin
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BHO_PFR_DETAIL_STDTUPLE_HPP
#define BHO_PFR_DETAIL_STDTUPLE_HPP
#pragma once

#include <asio2/bho/pfr/detail/config.hpp>

#include <utility>      // metaprogramming stuff
#include <tuple>

#include <asio2/bho/pfr/detail/sequence_tuple.hpp>

namespace bho { namespace pfr { namespace detail {

template <class T, std::size_t... I>
constexpr auto make_stdtuple_from_tietuple(const T& t, std::index_sequence<I...>) noexcept {
    return std::make_tuple(
        bho::pfr::detail::sequence_tuple::get<I>(t)...
    );
}

template <class T, std::size_t... I>
constexpr auto make_stdtiedtuple_from_tietuple(const T& t, std::index_sequence<I...>) noexcept {
    return std::tie(
        bho::pfr::detail::sequence_tuple::get<I>(t)...
    );
}

template <class T, std::size_t... I>
constexpr auto make_conststdtiedtuple_from_tietuple(const T& t, std::index_sequence<I...>) noexcept {
    return std::tuple<
        std::add_lvalue_reference_t<std::add_const_t<
            std::remove_reference_t<decltype(bho::pfr::detail::sequence_tuple::get<I>(t))>
        >>...
    >(
        bho::pfr::detail::sequence_tuple::get<I>(t)...
    );
}

}}} // namespace bho::pfr::detail

#endif // BHO_PFR_DETAIL_STDTUPLE_HPP
