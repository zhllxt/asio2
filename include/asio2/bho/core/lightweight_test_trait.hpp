#ifndef BHO_CORE_LIGHTWEIGHT_TEST_TRAIT_HPP
#define BHO_CORE_LIGHTWEIGHT_TEST_TRAIT_HPP

// MS compatible compilers support #pragma once

#if defined(_MSC_VER)
# pragma once
#endif

// bho/core/lightweight_test_trait.hpp
//
// BHO_TEST_TRAIT_TRUE, BHO_TEST_TRAIT_FALSE, BHO_TEST_TRAIT_SAME
//
// Copyright 2014, 2021 Peter Dimov
//
// Copyright 2019 Glen Joseph Fernandes
// (glenjofe@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt

#include <asio2/bho/core/lightweight_test.hpp>
#include <asio2/bho/core/type_name.hpp>
#include <asio2/bho/core/detail/is_same.hpp>
#include <asio2/bho/config.hpp>

namespace bho
{
namespace detail
{

template< class T > inline void test_trait_impl( char const * trait, void (*)( T ),
  bool expected, char const * file, int line, char const * function )
{
    if( T::value == expected )
    {
        test_results();
    }
    else
    {
        BHO_LIGHTWEIGHT_TEST_OSTREAM
            << file << "(" << line << "): predicate '" << trait << "' ["
            << bho::core::type_name<T>() << "]"
            << " test failed in function '" << function
            << "' (should have been " << ( expected? "true": "false" ) << ")"
            << std::endl;

        ++test_results().errors();
    }
}

template<class T> inline bool test_trait_same_impl_( T )
{
    return T::value;
}

template<class T1, class T2> inline void test_trait_same_impl( char const * types,
  bho::core::detail::is_same<T1, T2> same, char const * file, int line, char const * function )
{
    if( test_trait_same_impl_( same ) )
    {
        test_results();
    }
    else
    {
        BHO_LIGHTWEIGHT_TEST_OSTREAM
            << file << "(" << line << "): test 'is_same<" << types << ">'"
            << " failed in function '" << function
            << "' ('" << bho::core::type_name<T1>()
            << "' != '" << bho::core::type_name<T2>() << "')"
            << std::endl;

        ++test_results().errors();
    }
}

} // namespace detail
} // namespace bho

#define BHO_TEST_TRAIT_TRUE(type) ( ::bho::detail::test_trait_impl(#type, (void(*)type)0, true, __FILE__, __LINE__, BHO_CURRENT_FUNCTION) )
#define BHO_TEST_TRAIT_FALSE(type) ( ::bho::detail::test_trait_impl(#type, (void(*)type)0, false, __FILE__, __LINE__, BHO_CURRENT_FUNCTION) )

#if defined(__GNUC__)
// ignoring -Wvariadic-macros with #pragma doesn't work under GCC
# pragma GCC system_header
#endif

#define BHO_TEST_TRAIT_SAME(...) ( ::bho::detail::test_trait_same_impl(#__VA_ARGS__, ::bho::core::detail::is_same< __VA_ARGS__ >(), __FILE__, __LINE__, BHO_CURRENT_FUNCTION) )

#endif // #ifndef BHO_CORE_LIGHTWEIGHT_TEST_TRAIT_HPP
