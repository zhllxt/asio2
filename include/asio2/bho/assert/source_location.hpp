#ifndef BHO_ASSERT_SOURCE_LOCATION_HPP_INCLUDED
#define BHO_ASSERT_SOURCE_LOCATION_HPP_INCLUDED

// http://www.boost.org/libs/assert
//
// Copyright 2019, 2021 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#include <asio2/bho/current_function.hpp>
#include <asio2/bho/config.hpp>
#include <asio2/bho/cstdint.hpp>
#include <iosfwd>
#include <string>
#include <cstdio>

namespace bho
{

struct source_location
{
private:

    char const * file_;
    char const * function_;
    bho::uint_least32_t line_;
    bho::uint_least32_t column_;

public:

    BHO_CONSTEXPR source_location() BHO_NOEXCEPT: file_( "(unknown)" ), function_( "(unknown)" ), line_( 0 ), column_( 0 )
    {
    }

    BHO_CONSTEXPR source_location( char const * file, bho::uint_least32_t ln, char const * function, bho::uint_least32_t col = 0 ) BHO_NOEXCEPT: file_( file ), function_( function ), line_( ln ), column_( col )
    {
    }

    BHO_CONSTEXPR char const * file_name() const BHO_NOEXCEPT
    {
        return file_;
    }

    BHO_CONSTEXPR char const * function_name() const BHO_NOEXCEPT
    {
        return function_;
    }

    BHO_CONSTEXPR bho::uint_least32_t line() const BHO_NOEXCEPT
    {
        return line_;
    }

    BHO_CONSTEXPR bho::uint_least32_t column() const BHO_NOEXCEPT
    {
        return column_;
    }

#if defined(BHO_MSVC)
# pragma warning( push )
# pragma warning( disable: 4996 )
#endif

    std::string to_string() const
    {
        if( line() == 0 )
        {
            return "(unknown source location)";
        }

        std::string r = file_name();

        char buffer[ 16 ];

        std::sprintf( buffer, ":%ld", static_cast<long>( line() ) );
        r += buffer;

        if( column() )
        {
            std::sprintf( buffer, ":%ld", static_cast<long>( column() ) );
            r += buffer;
        }

        r += " in function '";
        r += function_name();
        r += '\'';

        return r;
    }

#if defined(BHO_MSVC)
# pragma warning( pop )
#endif

};

template<class E, class T> std::basic_ostream<E, T> & operator<<( std::basic_ostream<E, T> & os, source_location const & loc )
{
    os << loc.to_string();
    return os;
}

} // namespace bho

#if defined( BHO_DISABLE_CURRENT_LOCATION )

#  define BHO_CURRENT_LOCATION ::bho::source_location()

#elif defined(__clang_analyzer__)

// Cast to char const* to placate clang-tidy
// https://bugs.llvm.org/show_bug.cgi?id=28480
#  define BHO_CURRENT_LOCATION ::bho::source_location(__FILE__, __LINE__, static_cast<char const*>(BHO_CURRENT_FUNCTION))

#else

#  define BHO_CURRENT_LOCATION ::bho::source_location(__FILE__, __LINE__, BHO_CURRENT_FUNCTION)

#endif

#endif // #ifndef BHO_ASSERT_SOURCE_LOCATION_HPP_INCLUDED
