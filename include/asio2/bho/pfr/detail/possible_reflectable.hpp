// Copyright (c) 2022 Denis Mikhailov
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BHO_PFR_DETAIL_POSSIBLE_REFLECTABLE_HPP
#define BHO_PFR_DETAIL_POSSIBLE_REFLECTABLE_HPP
#pragma once

#include <asio2/bho/pfr/detail/config.hpp>
#include <asio2/bho/pfr/traits_fwd.hpp>

#include <type_traits> // for std::is_aggregate

namespace bho { namespace pfr { namespace detail {

///////////////////// Returns false when the type exactly wasn't be reflectable
template <class T, class WhatFor>
constexpr decltype(is_reflectable<T, WhatFor>::value) possible_reflectable(long) noexcept {
    return is_reflectable<T, WhatFor>::value;
}

#if BHO_PFR_ENABLE_IMPLICIT_REFLECTION

template <class T, class WhatFor>
constexpr bool possible_reflectable(int) noexcept {
#   if  defined(__cpp_lib_is_aggregate)
    using type = std::remove_cv_t<T>;
    return std::is_aggregate<type>();
#   else
    return true;
#   endif
}

#else

template <class T, class WhatFor>
constexpr bool possible_reflectable(int) noexcept {
    // negative answer here won't change behaviour in PFR-dependent libraries(like Fusion)
    return false;
}

#endif

}}} // namespace bho::pfr::detail

#endif // BHO_PFR_DETAIL_POSSIBLE_REFLECTABLE_HPP


