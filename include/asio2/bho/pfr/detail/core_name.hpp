// Copyright (c) 2023 Bela Schaum, X-Ryl669, Denis Mikhailov.
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


// Initial implementation by Bela Schaum, https://github.com/schaumb
// The way to make it union and UB free by X-Ryl669, https://github.com/X-Ryl669
//

#ifndef BHO_PFR_DETAIL_CORE_NAME_HPP
#define BHO_PFR_DETAIL_CORE_NAME_HPP
#pragma once

#include <asio2/bho/pfr/detail/config.hpp>

// Each core_name provides `bho::pfr::detail::get_name` and
// `bho::pfr::detail::tie_as_names_tuple` functions.
//
// The whole functional of extracting field's names is build on top of those
// two functions.
#if BHO_PFR_CORE_NAME_ENABLED
#include <asio2/bho/pfr/detail/core_name20_static.hpp>
#else
#include <asio2/bho/pfr/detail/core_name14_disabled.hpp>
#endif

#endif // BHO_PFR_DETAIL_CORE_NAME_HPP

