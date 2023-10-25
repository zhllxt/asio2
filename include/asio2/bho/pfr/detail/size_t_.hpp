// Copyright (c) 2016-2023 Antony Polukhin
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BHO_PFR_DETAIL_SIZE_T_HPP
#define BHO_PFR_DETAIL_SIZE_T_HPP
#pragma once

namespace bho { namespace pfr { namespace detail {

///////////////////// General utility stuff
template <std::size_t Index>
using size_t_ = std::integral_constant<std::size_t, Index >;

}}} // namespace bho::pfr::detail

#endif // BHO_PFR_DETAIL_SIZE_T_HPP
