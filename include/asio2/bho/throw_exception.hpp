#ifndef BHO_THROW_EXCEPTION_HPP_INCLUDED
#define BHO_THROW_EXCEPTION_HPP_INCLUDED

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

//  bho/throw_exception.hpp
//
//  Copyright (c) 2002, 2018-2022 Peter Dimov
//  Copyright (c) 2008-2009 Emil Dotchevski and Reverge Studios, Inc.
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
//  http://www.boost.org/libs/throw_exception

#include <asio2/bho/assert/source_location.hpp>
#include <asio2/bho/config.hpp>
#include <asio2/bho/config/workaround.hpp>
#include <exception>
#include <utility>
#include <cstddef>
#if !defined(BHO_NO_CXX11_HDR_TYPE_TRAITS)
#include <type_traits>
#endif

#if !defined( BHO_EXCEPTION_DISABLE ) && defined( BHO_BORLANDC ) && BHO_WORKAROUND( BHO_BORLANDC, BHO_TESTED_AT(0x593) )
# define BHO_EXCEPTION_DISABLE
#endif

namespace bho
{

#if defined( BHO_NO_EXCEPTIONS )

BHO_NORETURN void throw_exception( std::exception const & e ); // user defined
BHO_NORETURN void throw_exception( std::exception const & e, bho::source_location const & loc ); // user defined

#endif

// All boost exceptions are required to derive from std::exception,
// to ensure compatibility with BHO_NO_EXCEPTIONS.

inline void throw_exception_assert_compatibility( std::exception const & ) {}

// bho::throw_exception

#if !defined( BHO_NO_EXCEPTIONS )

#if defined( BHO_EXCEPTION_DISABLE )

template<class E> BHO_NORETURN void throw_exception( E const & e )
{
    throw_exception_assert_compatibility( e );
    throw e;
}

template<class E> BHO_NORETURN void throw_exception( E const & e, bho::source_location const & )
{
    throw_exception_assert_compatibility( e );
    throw e;
}

#else // defined( BHO_EXCEPTION_DISABLE )

template<class E> BHO_NORETURN void throw_exception( E const & e )
{
    throw_exception_assert_compatibility( e );
    throw e;
}

template<class E> BHO_NORETURN void throw_exception( E const & e, bho::source_location const & loc )
{
    throw_exception_assert_compatibility( e );
    ((void)loc); throw e;
}

#endif // defined( BHO_EXCEPTION_DISABLE )

#endif // !defined( BHO_NO_EXCEPTIONS )

} // namespace bho

// BHO_THROW_EXCEPTION

#define BHO_THROW_EXCEPTION(x) ::bho::throw_exception(x, BHO_CURRENT_LOCATION)


#endif // #ifndef BHO_THROW_EXCEPTION_HPP_INCLUDED
