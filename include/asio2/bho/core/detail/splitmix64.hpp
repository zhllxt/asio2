#ifndef BHO_CORE_DETAIL_SPLITMIX64_HPP_INCLUDED
#define BHO_CORE_DETAIL_SPLITMIX64_HPP_INCLUDED

// Copyright 2020 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// An implementation of splitmix64 for testing purposes,
// derived from Sebastiano Vigna's public domain implementation
// http://xorshift.di.unimi.it/splitmix64.c

#include <asio2/bho/cstdint.hpp>

namespace bho
{
namespace detail
{

class splitmix64
{
private:

    bho::uint64_t x_;

public:

    splitmix64(): x_( 0 )
    {
    }

    explicit splitmix64( bho::uint64_t seed ): x_( seed )
    {
    }

    bho::uint64_t operator()()
    {
        x_ += 0x9e3779b97f4a7c15;

        bho::uint64_t z = x_;

        z ^= z >> 30;
        z *= 0xbf58476d1ce4e5b9;
        z ^= z >> 27;
        z *= 0x94d049bb133111eb;
        z ^= z >> 31;

        return z;
    }
};

} // namespace detail
} // namespace bho

#endif // #ifndef BHO_CORE_DETAIL_SPLITMIX64_HPP_INCLUDED
