/*
 *          Copyright Andrey Semashev 2007 - 2013.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

/*!
 * \file   explicit_operator_bool.hpp
 * \author Andrey Semashev
 * \date   08.03.2009
 *
 * This header defines a compatibility macro that implements an unspecified
 * \c bool operator idiom, which is superseded with explicit conversion operators in
 * C++11.
 */

#ifndef BHO_CORE_EXPLICIT_OPERATOR_BOOL_HPP
#define BHO_CORE_EXPLICIT_OPERATOR_BOOL_HPP

#include <asio2/bho/config.hpp>
#include <asio2/bho/config/workaround.hpp>

#ifdef BHO_HAS_PRAGMA_ONCE
#pragma once
#endif

#if !defined(BHO_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS)

/*!
 * \brief The macro defines an explicit operator of conversion to \c bool
 *
 * The macro should be used inside the definition of a class that has to
 * support the conversion. The class should also implement <tt>operator!</tt>,
 * in terms of which the conversion operator will be implemented.
 */
#define BHO_EXPLICIT_OPERATOR_BOOL()\
    BHO_FORCEINLINE explicit operator bool () const\
    {\
        return !this->operator! ();\
    }

/*!
 * \brief The macro defines a noexcept explicit operator of conversion to \c bool
 *
 * The macro should be used inside the definition of a class that has to
 * support the conversion. The class should also implement <tt>operator!</tt>,
 * in terms of which the conversion operator will be implemented.
 */
#define BHO_EXPLICIT_OPERATOR_BOOL_NOEXCEPT()\
    BHO_FORCEINLINE explicit operator bool () const BHO_NOEXCEPT\
    {\
        return !this->operator! ();\
    }

#if !BHO_WORKAROUND(BHO_GCC, < 40700)

/*!
 * \brief The macro defines a constexpr explicit operator of conversion to \c bool
 *
 * The macro should be used inside the definition of a class that has to
 * support the conversion. The class should also implement <tt>operator!</tt>,
 * in terms of which the conversion operator will be implemented.
 */
#define BHO_CONSTEXPR_EXPLICIT_OPERATOR_BOOL()\
    BHO_FORCEINLINE BHO_CONSTEXPR explicit operator bool () const BHO_NOEXCEPT\
    {\
        return !this->operator! ();\
    }

#else

#define BHO_CONSTEXPR_EXPLICIT_OPERATOR_BOOL() BHO_EXPLICIT_OPERATOR_BOOL_NOEXCEPT()

#endif

#else // !defined(BHO_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS)

#if (defined(__SUNPRO_CC) && (__SUNPRO_CC <= 0x530)) && !defined(BHO_NO_COMPILER_CONFIG)
// Sun C++ 5.3 can't handle the safe_bool idiom, so don't use it
#define BHO_NO_UNSPECIFIED_BOOL
#endif // (defined(__SUNPRO_CC) && (__SUNPRO_CC <= 0x530)) && !defined(BHO_NO_COMPILER_CONFIG)

#if !defined(BHO_NO_UNSPECIFIED_BOOL)

namespace bho {

namespace detail {

#if !defined(_MSC_VER) && !defined(__IBMCPP__)

    struct unspecified_bool
    {
        // NOTE TO THE USER: If you see this in error messages then you tried
        // to apply an unsupported operator on the object that supports
        // explicit conversion to bool.
        struct OPERATORS_NOT_ALLOWED;
        static void true_value(OPERATORS_NOT_ALLOWED*) {}
    };
    typedef void (*unspecified_bool_type)(unspecified_bool::OPERATORS_NOT_ALLOWED*);

#else

    // MSVC and VACPP are too eager to convert pointer to function to void* even though they shouldn't
    struct unspecified_bool
    {
        // NOTE TO THE USER: If you see this in error messages then you tried
        // to apply an unsupported operator on the object that supports
        // explicit conversion to bool.
        struct OPERATORS_NOT_ALLOWED;
        void true_value(OPERATORS_NOT_ALLOWED*) {}
    };
    typedef void (unspecified_bool::*unspecified_bool_type)(unspecified_bool::OPERATORS_NOT_ALLOWED*);

#endif

} // namespace detail

} // namespace bho

#define BHO_EXPLICIT_OPERATOR_BOOL()\
    BHO_FORCEINLINE operator bho::detail::unspecified_bool_type () const\
    {\
        return (!this->operator! () ? &bho::detail::unspecified_bool::true_value : (bho::detail::unspecified_bool_type)0);\
    }

#define BHO_EXPLICIT_OPERATOR_BOOL_NOEXCEPT()\
    BHO_FORCEINLINE operator bho::detail::unspecified_bool_type () const BHO_NOEXCEPT\
    {\
        return (!this->operator! () ? &bho::detail::unspecified_bool::true_value : (bho::detail::unspecified_bool_type)0);\
    }

#define BHO_CONSTEXPR_EXPLICIT_OPERATOR_BOOL()\
    BHO_FORCEINLINE BHO_CONSTEXPR operator bho::detail::unspecified_bool_type () const BHO_NOEXCEPT\
    {\
        return (!this->operator! () ? &bho::detail::unspecified_bool::true_value : (bho::detail::unspecified_bool_type)0);\
    }

#else // !defined(BHO_NO_UNSPECIFIED_BOOL)

#define BHO_EXPLICIT_OPERATOR_BOOL()\
    BHO_FORCEINLINE operator bool () const\
    {\
        return !this->operator! ();\
    }

#define BHO_EXPLICIT_OPERATOR_BOOL_NOEXCEPT()\
    BHO_FORCEINLINE operator bool () const BHO_NOEXCEPT\
    {\
        return !this->operator! ();\
    }

#define BHO_CONSTEXPR_EXPLICIT_OPERATOR_BOOL()\
    BHO_FORCEINLINE BHO_CONSTEXPR operator bool () const BHO_NOEXCEPT\
    {\
        return !this->operator! ();\
    }

#endif // !defined(BHO_NO_UNSPECIFIED_BOOL)

#endif // !defined(BHO_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS)

#endif // BHO_CORE_EXPLICIT_OPERATOR_BOOL_HPP
