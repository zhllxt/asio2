#ifndef BHO_DESCRIBE_ENUM_TO_STRING_HPP_INCLUDED
#define BHO_DESCRIBE_ENUM_TO_STRING_HPP_INCLUDED

// Copyright 2020, 2021 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <asio2/bho/describe/detail/config.hpp>

#if defined(BHO_DESCRIBE_CXX14)

#include <asio2/bho/describe/enumerators.hpp>
#include <asio2/bho/mp11/algorithm.hpp>

#if defined(_MSC_VER) && _MSC_VER == 1900
# pragma warning(push)
# pragma warning(disable: 4100) // unreferenced formal parameter
#endif

namespace bho
{
namespace describe
{

template<class E, class De = describe_enumerators<E>>
char const * enum_to_string( E e, char const* def ) noexcept
{
    char const * r = def;

    mp11::mp_for_each<De>([&](auto D){

        if( e == D.value ) r = D.name;

    });

    return r;
}

} // namespace describe
} // namespace bho

#if defined(_MSC_VER) && _MSC_VER == 1900
# pragma warning(pop)
#endif

#endif // defined(BHO_DESCRIBE_CXX14)

#endif // #ifndef BHO_DESCRIBE_ENUM_TO_STRING_HPP_INCLUDED
