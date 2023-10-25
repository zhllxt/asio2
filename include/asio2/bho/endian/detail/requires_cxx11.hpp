#ifndef BHO_ENDIAN_DETAIL_REQUIRES_CXX11_HPP_INCLUDED
#define BHO_ENDIAN_DETAIL_REQUIRES_CXX11_HPP_INCLUDED

// Copyright 2023 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <asio2/bho/config.hpp>
#include <asio2/bho/config/pragma_message.hpp>

#if defined(BHO_NO_CXX11_VARIADIC_TEMPLATES) || \
    defined(BHO_NO_CXX11_RVALUE_REFERENCES) || \
    defined(BHO_NO_CXX11_DECLTYPE) || \
    defined(BHO_NO_CXX11_CONSTEXPR) || \
    defined(BHO_NO_CXX11_NOEXCEPT) || \
    defined(BHO_NO_CXX11_SCOPED_ENUMS) || \
    defined(BHO_NO_CXX11_DEFAULTED_FUNCTIONS)

BHO_PRAGMA_MESSAGE("C++03 support was deprecated in BHO.Endian 1.82 and will be removed in BHO.Endian 1.84. Please open an issue in https://github.com/boostorg/endian if you want it retained.")

#endif

#endif // #ifndef BHO_ENDIAN_DETAIL_REQUIRES_CXX11_HPP_INCLUDED
