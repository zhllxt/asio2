// Copyright (c) 2023 Bela Schaum, X-Ryl669, Denis Mikhailov.
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


// Initial implementation by Bela Schaum, https://github.com/schaumb
// The way to make it union and UB free by X-Ryl669, https://github.com/X-Ryl669
//

#ifndef BHO_PFR_DETAIL_CORE_NAME14_DISABLED_HPP
#define BHO_PFR_DETAIL_CORE_NAME14_DISABLED_HPP
#pragma once

#include <asio2/bho/pfr/detail/config.hpp>
#include <asio2/bho/pfr/detail/sequence_tuple.hpp>

namespace bho { namespace pfr { namespace detail {

template <class T, std::size_t I>
constexpr auto get_name() noexcept {
    static_assert(
        sizeof(T) && false,
        "====================> BHO.PFR: Field's names extracting functionality requires C++20."
    );

    return nullptr;
}

template <class T>
constexpr auto tie_as_names_tuple() noexcept {
    static_assert(
        sizeof(T) && false,
        "====================> BHO.PFR: Field's names extracting functionality requires C++20."
    );

    return detail::sequence_tuple::make_sequence_tuple();
}

}}} // namespace bho::pfr::detail

#endif // BHO_PFR_DETAIL_CORE_NAME14_DISABLED_HPP

