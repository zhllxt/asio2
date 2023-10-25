// Copyright (c) 2016-2023 Antony Polukhin
// Copyright (c) 2022 Denis Mikhailov
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BHO_PFR_DETAIL_CONFIG_HPP
#define BHO_PFR_DETAIL_CONFIG_HPP
#pragma once

#include <asio2/bho/pfr/config.hpp>

#if !BHO_PFR_ENABLED

#error BHO.PFR library is not supported in your environment.             \
       Try one of the possible solutions:                                  \
       1. try to take away an '-DBHO_PFR_ENABLED=0', if it exists        \
       2. enable C++14;                                                    \
       3. enable C++17;                                                    \
       4. update your compiler;                                            \
       or disable this error by '-DBHO_PFR_ENABLED=1' if you really know what are you doing.

#endif // !BHO_PFR_ENABLED

#endif // BHO_PFR_DETAIL_CONFIG_HPP

