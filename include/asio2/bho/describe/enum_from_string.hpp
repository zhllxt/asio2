#ifndef BHO_DESCRIBE_ENUM_FROM_STRING_HPP_INCLUDED
#define BHO_DESCRIBE_ENUM_FROM_STRING_HPP_INCLUDED

// Copyright 2020, 2021 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <asio2/bho/describe/detail/config.hpp>

#if defined(BHO_DESCRIBE_CXX14)

#include <asio2/bho/describe/enumerators.hpp>
#include <asio2/bho/mp11/algorithm.hpp>
#include <cstring>
#include <type_traits>

#if defined(_MSC_VER) && _MSC_VER == 1900
# pragma warning(push)
# pragma warning(disable: 4100) // unreferenced formal parameter
#endif

namespace bho
{
namespace describe
{

template<class E, class De = describe_enumerators<E>>
bool enum_from_string( char const* name, E& e ) noexcept
{
    bool found = false;

    mp11::mp_for_each<De>([&](auto D){

        if( !found && std::strcmp( D.name, name ) == 0 )
        {
            found = true;
            e = D.value;
        }

    });

    return found;
}

template<class S, class E, class De = describe_enumerators<E>,
    class En = std::enable_if_t<
        std::is_same<typename S::value_type, char>::value &&
        std::is_same<typename S::traits_type::char_type, char>::value
    >
>
bool enum_from_string( S const& name, E& e ) noexcept
{
    bool found = false;

    mp11::mp_for_each<De>([&](auto D){

        if( !found && name == D.name )
        {
            found = true;
            e = D.value;
        }

    });

    return found;
}

} // namespace describe
} // namespace bho

#if defined(_MSC_VER) && _MSC_VER == 1900
# pragma warning(pop)
#endif

#endif // defined(BHO_DESCRIBE_CXX14)

#endif // #ifndef BHO_DESCRIBE_ENUM_FROM_STRING_HPP_INCLUDED
