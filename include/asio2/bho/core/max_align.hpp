#ifndef BHO_CORE_MAX_ALIGN_HPP_INCLUDED
#define BHO_CORE_MAX_ALIGN_HPP_INCLUDED

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

//  Copyright 2023 Peter Dimov
//  Distributed under the Boost Software License, Version 1.0.
//  https://www.boost.org/LICENSE_1_0.txt

#include <asio2/bho/core/alignof.hpp>
#include <asio2/bho/config.hpp>
#include <cstddef>

// BHO_CORE_HAS_FLOAT128

#if defined(BHO_HAS_FLOAT128)

# define BHO_CORE_HAS_FLOAT128

#elif defined(__SIZEOF_FLOAT128__)

# define BHO_CORE_HAS_FLOAT128

#elif defined(__GNUC__) && (__GNUC__ * 100 + __GNUC_MINOR__ >= 404) && defined(__i386__)

# define BHO_CORE_HAS_FLOAT128

#endif

// max_align_t, max_align

namespace bho
{
namespace core
{

union max_align_t
{
    char c;
    short s;
    int i;
    long l;

#if !defined(BHO_NO_LONG_LONG)

    bho::long_long_type ll;

#endif

#if defined(BHO_HAS_INT128)

    bho::int128_type i128;

#endif

    float f;
    double d;
    long double ld;

#if defined(BHO_CORE_HAS_FLOAT128)

    __float128 f128;

#endif

    void* p;
    void (*pf) ();

    int max_align_t::* pm;
    void (max_align_t::*pmf)();
};

BHO_CONSTEXPR_OR_CONST std::size_t max_align = BHO_CORE_ALIGNOF( max_align_t );

} // namespace core
} // namespace bho

#endif  // #ifndef BHO_CORE_MAX_ALIGN_HPP_INCLUDED
