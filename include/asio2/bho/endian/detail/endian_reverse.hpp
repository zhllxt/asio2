#ifndef BHO_ENDIAN_DETAIL_ENDIAN_REVERSE_HPP_INCLUDED
#define BHO_ENDIAN_DETAIL_ENDIAN_REVERSE_HPP_INCLUDED

// Copyright 2019, 2020 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <asio2/bho/endian/detail/integral_by_size.hpp>
#include <asio2/bho/endian/detail/intrinsic.hpp>
#include <asio2/bho/endian/detail/is_scoped_enum.hpp>
#include <asio2/bho/endian/detail/is_integral.hpp>
#include <asio2/bho/endian/detail/static_assert.hpp>
#include <asio2/bho/config.hpp>
#include <type_traits>
#include <cstdint>
#include <cstddef>
#include <cstring>

#if defined(BHO_ENDIAN_NO_INTRINSICS)
# if defined(BHO_NO_CXX14_CONSTEXPR)
#  define BHO_ENDIAN_CONSTEXPR
# else
#  define BHO_ENDIAN_CONSTEXPR constexpr
# endif
#else
# if defined(BHO_ENDIAN_CONSTEXPR_INTRINSICS)
#  define BHO_ENDIAN_CONSTEXPR BHO_CONSTEXPR
# else
#  define BHO_ENDIAN_CONSTEXPR
# endif
#endif

namespace bho
{
namespace endian
{

namespace detail
{

//  -- portable approach suggested by tymofey, with avoidance of undefined behavior
//     as suggested by Giovanni Piero Deretta, with a further refinement suggested
//     by Pyry Jahkola.
//  -- intrinsic approach suggested by reviewers, and by David Stone, who provided
//     his Boost licensed macro implementation (detail/intrinsic.hpp)

inline std::uint8_t BHO_CONSTEXPR endian_reverse_impl( std::uint8_t x ) BHO_NOEXCEPT
{
    return x;
}

inline std::uint16_t BHO_ENDIAN_CONSTEXPR endian_reverse_impl( std::uint16_t x ) BHO_NOEXCEPT
{
#ifdef BHO_ENDIAN_NO_INTRINSICS

    return (x << 8) | (x >> 8);

#else

    return BHO_ENDIAN_INTRINSIC_BYTE_SWAP_2(x);

#endif
}

inline std::uint32_t BHO_ENDIAN_CONSTEXPR endian_reverse_impl( std::uint32_t x ) BHO_NOEXCEPT
{
#ifdef BHO_ENDIAN_NO_INTRINSICS

    std::uint32_t step16 = x << 16 | x >> 16;
    return ((step16 << 8) & 0xff00ff00) | ((step16 >> 8) & 0x00ff00ff);

#else

    return BHO_ENDIAN_INTRINSIC_BYTE_SWAP_4(x);

#endif
}

inline std::uint64_t BHO_ENDIAN_CONSTEXPR endian_reverse_impl( std::uint64_t x ) BHO_NOEXCEPT
{
#ifdef BHO_ENDIAN_NO_INTRINSICS

    std::uint64_t step32 = x << 32 | x >> 32;
    std::uint64_t step16 = (step32 & 0x0000FFFF0000FFFFULL) << 16 | (step32 & 0xFFFF0000FFFF0000ULL) >> 16;
    return (step16 & 0x00FF00FF00FF00FFULL) << 8 | (step16 & 0xFF00FF00FF00FF00ULL) >> 8;

#else

    return BHO_ENDIAN_INTRINSIC_BYTE_SWAP_8(x);

# endif
}

#if defined(__SIZEOF_INT128__)

inline __uint128_t BHO_ENDIAN_CONSTEXPR endian_reverse_impl( __uint128_t x ) BHO_NOEXCEPT
{
    return endian_reverse_impl( static_cast<std::uint64_t>( x >> 64 ) ) |
        static_cast<__uint128_t>( endian_reverse_impl( static_cast<std::uint64_t>( x ) ) ) << 64;
}

#endif

// is_endian_reversible

template<class T> struct is_endian_reversible: std::integral_constant<bool,
    (is_integral<T>::value && !std::is_same<T, bool>::value) || is_scoped_enum<T>::value>
{
};

// is_endian_reversible_inplace

template<class T> struct is_endian_reversible_inplace: std::integral_constant<bool,
    std::is_integral<T>::value || std::is_enum<T>::value || std::is_same<T, float>::value || std::is_same<T, double>::value>
{
};

} // namespace detail

// Requires:
//   T is non-bool integral or scoped enumeration type

template<class T> inline BHO_CONSTEXPR
    typename std::enable_if< !std::is_class<T>::value, T >::type
    endian_reverse( T x ) BHO_NOEXCEPT
{
    BHO_ENDIAN_STATIC_ASSERT( detail::is_endian_reversible<T>::value );

    typedef typename detail::integral_by_size< sizeof(T) >::type uintN_t;

    return static_cast<T>( detail::endian_reverse_impl( static_cast<uintN_t>( x ) ) );
}

// Requires:
//   T is integral, enumeration, float or double

template<class T> inline
    typename std::enable_if< !std::is_class<T>::value >::type
    endian_reverse_inplace( T & x ) BHO_NOEXCEPT
{
    BHO_ENDIAN_STATIC_ASSERT( detail::is_endian_reversible_inplace<T>::value );

    typename detail::integral_by_size< sizeof(T) >::type x2;

    std::memcpy( &x2, &x, sizeof(T) );

    x2 = detail::endian_reverse_impl( x2 );

    std::memcpy( &x, &x2, sizeof(T) );
}

// Default implementation for user-defined types

template<class T> inline
    typename std::enable_if< std::is_class<T>::value >::type
    endian_reverse_inplace( T & x ) BHO_NOEXCEPT
{
    x = endian_reverse( x );
}

// endian_reverse_inplace for arrays

template<class T, std::size_t N>
inline void endian_reverse_inplace( T (&x)[ N ] ) BHO_NOEXCEPT
{
    for( std::size_t i = 0; i < N; ++i )
    {
        endian_reverse_inplace( x[i] );
    }
}

} // namespace endian
} // namespace bho

#endif  // BHO_ENDIAN_DETAIL_ENDIAN_REVERSE_HPP_INCLUDED
