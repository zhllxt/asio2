#ifndef BHO_CORE_BIT_HPP_INCLUDED
#define BHO_CORE_BIT_HPP_INCLUDED

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// bho/core/bit.hpp
//
// A portable version of the C++20 standard header <bit>
//
// Copyright 2020 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <asio2/bho/config.hpp>
#include <asio2/bho/static_assert.hpp>
#include <asio2/bho/cstdint.hpp>
#include <limits>
#include <cstring>

#if defined(_MSC_VER)

# include <intrin.h>
# pragma intrinsic(_BitScanForward)
# pragma intrinsic(_BitScanReverse)

# if defined(_M_X64)
#  pragma intrinsic(_BitScanForward64)
#  pragma intrinsic(_BitScanReverse64)
# endif

# pragma warning(push)
# pragma warning(disable: 4127) // conditional expression is constant
# pragma warning(disable: 4244) // conversion from int to T

#endif // defined(_MSC_VER)

namespace bho
{
namespace core
{

// bit_cast

template<class To, class From>
To bit_cast( From const & from ) BHO_NOEXCEPT
{
    BHO_STATIC_ASSERT( sizeof(To) == sizeof(From) );

    To to;
    std::memcpy( &to, &from, sizeof(To) );
    return to;
}

// countl

#if defined(__GNUC__) || defined(__clang__)

namespace detail
{

BHO_CONSTEXPR inline int countl_impl( unsigned char x ) BHO_NOEXCEPT
{
    return x? __builtin_clz( x ) - ( std::numeric_limits<unsigned int>::digits - std::numeric_limits<unsigned char>::digits ): std::numeric_limits<unsigned char>::digits;
}

BHO_CONSTEXPR inline int countl_impl( unsigned short x ) BHO_NOEXCEPT
{
    return x? __builtin_clz( x ) - ( std::numeric_limits<unsigned int>::digits - std::numeric_limits<unsigned short>::digits ): std::numeric_limits<unsigned short>::digits;
}

BHO_CONSTEXPR inline int countl_impl( unsigned int x ) BHO_NOEXCEPT
{
    return x? __builtin_clz( x ): std::numeric_limits<unsigned int>::digits;
}

BHO_CONSTEXPR inline int countl_impl( unsigned long x ) BHO_NOEXCEPT
{
    return x? __builtin_clzl( x ): std::numeric_limits<unsigned long>::digits;
}

BHO_CONSTEXPR inline int countl_impl( unsigned long long x ) BHO_NOEXCEPT
{
    return x? __builtin_clzll( x ): std::numeric_limits<unsigned long long>::digits;
}

} // namespace detail

template<class T>
BHO_CONSTEXPR int countl_zero( T x ) BHO_NOEXCEPT
{
    return bho::core::detail::countl_impl( x );
}

#else // defined(__GNUC__) || defined(__clang__)

namespace detail
{

inline int countl_impl( bho::uint32_t x ) BHO_NOEXCEPT
{
#if defined(_MSC_VER)

    unsigned long r;

    if( _BitScanReverse( &r, x ) )
    {
        return 31 - static_cast<int>( r );
    }
    else
    {
        return 32;
    }

#else

    static unsigned char const mod37[ 37 ] = { 32, 31, 6, 30, 9, 5, 0, 29, 16, 8, 2, 4, 21, 0, 19, 28, 25, 15, 0, 7, 10, 1, 17, 3, 22, 20, 26, 0, 11, 18, 23, 27, 12, 24, 13, 14, 0 };

    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;

    return mod37[ x % 37 ];

#endif
}

inline int countl_impl( bho::uint64_t x ) BHO_NOEXCEPT
{
#if defined(_MSC_VER) && defined(_M_X64)

    unsigned long r;

    if( _BitScanReverse64( &r, x ) )
    {
        return 63 - static_cast<int>( r );
    }
    else
    {
        return 64;
    }

#else

    return static_cast<bho::uint32_t>( x >> 32 ) != 0?
        bho::core::detail::countl_impl( static_cast<bho::uint32_t>( x >> 32 ) ):
        bho::core::detail::countl_impl( static_cast<bho::uint32_t>( x ) ) + 32;

#endif
}

inline int countl_impl( bho::uint8_t x ) BHO_NOEXCEPT
{
    return bho::core::detail::countl_impl( static_cast<bho::uint32_t>( x ) ) - 24;
}

inline int countl_impl( bho::uint16_t x ) BHO_NOEXCEPT
{
    return bho::core::detail::countl_impl( static_cast<bho::uint32_t>( x ) ) - 16;
}

} // namespace detail

template<class T>
int countl_zero( T x ) BHO_NOEXCEPT
{
    BHO_STATIC_ASSERT( sizeof(T) == sizeof(bho::uint8_t) || sizeof(T) == sizeof(bho::uint16_t) || sizeof(T) == sizeof(bho::uint32_t) || sizeof(T) == sizeof(bho::uint64_t) );

    if( sizeof(T) == sizeof(bho::uint8_t) )
    {
        return bho::core::detail::countl_impl( static_cast<bho::uint8_t>( x ) );
    }
    else if( sizeof(T) == sizeof(bho::uint16_t) )
    {
        return bho::core::detail::countl_impl( static_cast<bho::uint16_t>( x ) );
    }
    else if( sizeof(T) == sizeof(bho::uint32_t) )
    {
        return bho::core::detail::countl_impl( static_cast<bho::uint32_t>( x ) );
    }
    else
    {
        return bho::core::detail::countl_impl( static_cast<bho::uint64_t>( x ) );
    }
}

#endif // defined(__GNUC__) || defined(__clang__)

template<class T>
BHO_CONSTEXPR int countl_one( T x ) BHO_NOEXCEPT
{
    return bho::core::countl_zero( static_cast<T>( ~x ) );
}

// countr

#if defined(__GNUC__) || defined(__clang__)

namespace detail
{

BHO_CONSTEXPR inline int countr_impl( unsigned char x ) BHO_NOEXCEPT
{
    return x? __builtin_ctz( x ): std::numeric_limits<unsigned char>::digits;
}

BHO_CONSTEXPR inline int countr_impl( unsigned short x ) BHO_NOEXCEPT
{
    return x? __builtin_ctz( x ): std::numeric_limits<unsigned short>::digits;
}

BHO_CONSTEXPR inline int countr_impl( unsigned int x ) BHO_NOEXCEPT
{
    return x? __builtin_ctz( x ): std::numeric_limits<unsigned int>::digits;
}

BHO_CONSTEXPR inline int countr_impl( unsigned long x ) BHO_NOEXCEPT
{
    return x? __builtin_ctzl( x ): std::numeric_limits<unsigned long>::digits;
}

BHO_CONSTEXPR inline int countr_impl( unsigned long long x ) BHO_NOEXCEPT
{
    return x? __builtin_ctzll( x ): std::numeric_limits<unsigned long long>::digits;
}

} // namespace detail

template<class T>
BHO_CONSTEXPR int countr_zero( T x ) BHO_NOEXCEPT
{
    return bho::core::detail::countr_impl( x );
}

#else // defined(__GNUC__) || defined(__clang__)

namespace detail
{

inline int countr_impl( bho::uint32_t x ) BHO_NOEXCEPT
{
#if defined(_MSC_VER)

    unsigned long r;

    if( _BitScanForward( &r, x ) )
    {
        return static_cast<int>( r );
    }
    else
    {
        return 32;
    }

#else

    static unsigned char const mod37[ 37 ] = { 32, 0, 1, 26, 2, 23, 27, 0, 3, 16, 24, 30, 28, 11, 0, 13, 4, 7, 17, 0, 25, 22, 31, 15, 29, 10, 12, 6, 0, 21, 14, 9, 5, 20, 8, 19, 18 };
    return mod37[ ( -(bho::int32_t)x & x ) % 37 ];

#endif
}

inline int countr_impl( bho::uint64_t x ) BHO_NOEXCEPT
{
#if defined(_MSC_VER) && defined(_M_X64)

    unsigned long r;

    if( _BitScanForward64( &r, x ) )
    {
        return static_cast<int>( r );
    }
    else
    {
        return 64;
    }

#else

    return static_cast<bho::uint32_t>( x ) != 0?
        bho::core::detail::countr_impl( static_cast<bho::uint32_t>( x ) ):
        bho::core::detail::countr_impl( static_cast<bho::uint32_t>( x >> 32 ) ) + 32;

#endif
}

inline int countr_impl( bho::uint8_t x ) BHO_NOEXCEPT
{
    return bho::core::detail::countr_impl( static_cast<bho::uint32_t>( x ) | 0x100 );
}

inline int countr_impl( bho::uint16_t x ) BHO_NOEXCEPT
{
    return bho::core::detail::countr_impl( static_cast<bho::uint32_t>( x ) | 0x10000 );
}

} // namespace detail

template<class T>
int countr_zero( T x ) BHO_NOEXCEPT
{
    BHO_STATIC_ASSERT( sizeof(T) == sizeof(bho::uint8_t) || sizeof(T) == sizeof(bho::uint16_t) || sizeof(T) == sizeof(bho::uint32_t) || sizeof(T) == sizeof(bho::uint64_t) );

    if( sizeof(T) == sizeof(bho::uint8_t) )
    {
        return bho::core::detail::countr_impl( static_cast<bho::uint8_t>( x ) );
    }
    else if( sizeof(T) == sizeof(bho::uint16_t) )
    {
        return bho::core::detail::countr_impl( static_cast<bho::uint16_t>( x ) );
    }
    else if( sizeof(T) == sizeof(bho::uint32_t) )
    {
        return bho::core::detail::countr_impl( static_cast<bho::uint32_t>( x ) );
    }
    else
    {
        return bho::core::detail::countr_impl( static_cast<bho::uint64_t>( x ) );
    }
}

#endif // defined(__GNUC__) || defined(__clang__)

template<class T>
BHO_CONSTEXPR int countr_one( T x ) BHO_NOEXCEPT
{
    return bho::core::countr_zero( static_cast<T>( ~x ) );
}

// popcount

#if defined(__GNUC__) || defined(__clang__)

#if defined(__clang__) && __clang_major__ * 100 + __clang_minor__ < 304
# define BHO_CORE_POPCOUNT_CONSTEXPR
#else
# define BHO_CORE_POPCOUNT_CONSTEXPR BHO_CONSTEXPR
#endif

namespace detail
{

BHO_CORE_POPCOUNT_CONSTEXPR inline int popcount_impl( unsigned char x ) BHO_NOEXCEPT
{
    return __builtin_popcount( x );
}

BHO_CORE_POPCOUNT_CONSTEXPR inline int popcount_impl( unsigned short x ) BHO_NOEXCEPT
{
    return __builtin_popcount( x );
}

BHO_CORE_POPCOUNT_CONSTEXPR inline int popcount_impl( unsigned int x ) BHO_NOEXCEPT
{
    return __builtin_popcount( x );
}

BHO_CORE_POPCOUNT_CONSTEXPR inline int popcount_impl( unsigned long x ) BHO_NOEXCEPT
{
    return __builtin_popcountl( x );
}

BHO_CORE_POPCOUNT_CONSTEXPR inline int popcount_impl( unsigned long long x ) BHO_NOEXCEPT
{
    return __builtin_popcountll( x );
}

} // namespace detail

#undef BHO_CORE_POPCOUNT_CONSTEXPR

template<class T>
BHO_CONSTEXPR int popcount( T x ) BHO_NOEXCEPT
{
    return bho::core::detail::popcount_impl( x );
}

#else // defined(__GNUC__) || defined(__clang__)

namespace detail
{

BHO_CXX14_CONSTEXPR inline int popcount_impl( bho::uint32_t x ) BHO_NOEXCEPT
{
    x = x - ( ( x >> 1 ) & 0x55555555 );
    x = ( x & 0x33333333 ) + ( ( x >> 2 ) & 0x33333333 );
    x = ( x + ( x >> 4 ) ) & 0x0F0F0F0F;

    return static_cast<unsigned>( ( x * 0x01010101 ) >> 24 );
}

BHO_CXX14_CONSTEXPR inline int popcount_impl( bho::uint64_t x ) BHO_NOEXCEPT
{
    x = x - ( ( x >> 1 ) & 0x5555555555555555 );
    x = ( x & 0x3333333333333333 ) + ( ( x >> 2 ) & 0x3333333333333333 );
    x = ( x + ( x >> 4 ) ) & 0x0F0F0F0F0F0F0F0F;

    return static_cast<unsigned>( ( x * 0x0101010101010101 ) >> 56 );
}

} // namespace detail

template<class T>
BHO_CXX14_CONSTEXPR int popcount( T x ) BHO_NOEXCEPT
{
    BHO_STATIC_ASSERT( sizeof(T) <= sizeof(bho::uint64_t) );

    if( sizeof(T) <= sizeof(bho::uint32_t) )
    {
        return bho::core::detail::popcount_impl( static_cast<bho::uint32_t>( x ) );
    }
    else
    {
        return bho::core::detail::popcount_impl( static_cast<bho::uint64_t>( x ) );
    }
}

#endif // defined(__GNUC__) || defined(__clang__)

// rotating

template<class T>
BHO_CXX14_CONSTEXPR T rotl( T x, int s ) BHO_NOEXCEPT
{
    unsigned const mask = std::numeric_limits<T>::digits - 1;
    return x << (s & mask) | x >> ((-s) & mask);
}

template<class T>
BHO_CXX14_CONSTEXPR T rotr( T x, int s ) BHO_NOEXCEPT
{
    unsigned const mask = std::numeric_limits<T>::digits - 1;
    return x >> (s & mask) | x << ((-s) & mask);
}

// integral powers of 2

template<class T>
BHO_CONSTEXPR bool has_single_bit( T x ) BHO_NOEXCEPT
{
    return x != 0 && ( x & ( x - 1 ) ) == 0;
}

template<class T>
BHO_CONSTEXPR T bit_width( T x ) BHO_NOEXCEPT
{
    return std::numeric_limits<T>::digits - bho::core::countl_zero( x );
}

template<class T>
BHO_CONSTEXPR T bit_floor( T x ) BHO_NOEXCEPT
{
    return x == 0? 0: T(1) << ( bho::core::bit_width( x ) - 1 );
}

namespace detail
{

BHO_CXX14_CONSTEXPR inline bho::uint32_t bit_ceil_impl( bho::uint32_t x ) BHO_NOEXCEPT
{
    if( x == 0 )
    {
        return 0;
    }

    --x;

    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;

    ++x;

    return x;
}

BHO_CXX14_CONSTEXPR inline bho::uint64_t bit_ceil_impl( bho::uint64_t x ) BHO_NOEXCEPT
{
    if( x == 0 )
    {
        return 0;
    }

    --x;

    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;

    ++x;

    return x;
}

} // namespace detail

template<class T>
BHO_CXX14_CONSTEXPR T bit_ceil( T x ) BHO_NOEXCEPT
{
    BHO_STATIC_ASSERT( sizeof(T) <= sizeof(bho::uint64_t) );

    if( sizeof(T) <= sizeof(bho::uint32_t) )
    {
        return static_cast<T>( bho::core::detail::bit_ceil_impl( static_cast<bho::uint32_t>( x ) ) );
    }
    else
    {
        return static_cast<T>( bho::core::detail::bit_ceil_impl( static_cast<bho::uint64_t>( x ) ) );
    }
}

// endian

#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

# define BHO_CORE_BIT_NATIVE_INITIALIZER =little

#elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

# define BHO_CORE_BIT_NATIVE_INITIALIZER =big

#elif defined(__BYTE_ORDER__) && defined(__ORDER_PDP_ENDIAN__) && __BYTE_ORDER__ == __ORDER_PDP_ENDIAN__

# define BHO_CORE_BIT_NATIVE_INITIALIZER

#elif defined(__LITTLE_ENDIAN__)

# define BHO_CORE_BIT_NATIVE_INITIALIZER =little

#elif defined(__BIG_ENDIAN__)

# define BHO_CORE_BIT_NATIVE_INITIALIZER =big

#elif defined(_MSC_VER) || defined(__i386__) || defined(__x86_64__)

# define BHO_CORE_BIT_NATIVE_INITIALIZER =little

#else

# define BHO_CORE_BIT_NATIVE_INITIALIZER

#endif

#if !defined(BHO_NO_CXX11_SCOPED_ENUMS)

enum class endian
{
    big,
    little,
    native BHO_CORE_BIT_NATIVE_INITIALIZER
};

typedef endian endian_type;

#else

namespace endian
{

enum type
{
    big,
    little,
    native BHO_CORE_BIT_NATIVE_INITIALIZER
};

} // namespace endian

typedef endian::type endian_type;

#endif

#undef BHO_CORE_BIT_NATIVE_INITIALIZER

} // namespace core
} // namespace bho

#if defined(_MSC_VER)
# pragma warning(pop)
#endif

#endif  // #ifndef BHO_CORE_BIT_HPP_INCLUDED
