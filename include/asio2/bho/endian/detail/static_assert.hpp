#ifndef BHO_ENDIAN_DETAIL_STATIC_ASSERT_HPP_INCLUDED
#define BHO_ENDIAN_DETAIL_STATIC_ASSERT_HPP_INCLUDED

// Copyright 2023 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#define BHO_ENDIAN_STATIC_ASSERT(...) static_assert(__VA_ARGS__, #__VA_ARGS__)

#endif  // BHO_ENDIAN_DETAIL_STATIC_ASSERT_HPP_INCLUDED
