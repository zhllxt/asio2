#ifndef BHO_DESCRIBE_CLASS_HPP_INCLUDED
#define BHO_DESCRIBE_CLASS_HPP_INCLUDED

// Copyright 2020 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <asio2/bho/describe/detail/config.hpp>

#if !defined(BHO_DESCRIBE_CXX14)

#define BHO_DESCRIBE_CLASS(C, Bases, Public, Protected, Private)
#define BHO_DESCRIBE_STRUCT(C, Bases, Members)

#else

#include <asio2/bho/describe/detail/bases.hpp>
#include <asio2/bho/describe/detail/members.hpp>
#include <type_traits>

namespace bho
{
namespace describe
{

#if defined(_MSC_VER) && !defined(__clang__)

#define BHO_DESCRIBE_PP_UNPACK(...) __VA_ARGS__

#define BHO_DESCRIBE_CLASS(C, Bases, Public, Protected, Private) \
    friend BHO_DESCRIBE_BASES(C, BHO_DESCRIBE_PP_UNPACK Bases) \
    friend BHO_DESCRIBE_PUBLIC_MEMBERS(C, BHO_DESCRIBE_PP_UNPACK Public) \
    friend BHO_DESCRIBE_PROTECTED_MEMBERS(C, BHO_DESCRIBE_PP_UNPACK Protected) \
    friend BHO_DESCRIBE_PRIVATE_MEMBERS(C, BHO_DESCRIBE_PP_UNPACK Private)

#define BHO_DESCRIBE_STRUCT(C, Bases, Members) \
    static_assert(std::is_class<C>::value || std::is_union<C>::value, "BHO_DESCRIBE_STRUCT should only be used with class types"); \
    BHO_DESCRIBE_BASES(C, BHO_DESCRIBE_PP_UNPACK Bases) \
    BHO_DESCRIBE_PUBLIC_MEMBERS(C, BHO_DESCRIBE_PP_UNPACK Members) \
    BHO_DESCRIBE_PROTECTED_MEMBERS(C) \
    BHO_DESCRIBE_PRIVATE_MEMBERS(C)

#else

#if defined(__GNUC__) && __GNUC__ >= 8
# define BHO_DESCRIBE_PP_UNPACK(...) __VA_OPT__(,) __VA_ARGS__
#else
# define BHO_DESCRIBE_PP_UNPACK(...) , ##__VA_ARGS__
#endif

#define BHO_DESCRIBE_BASES_(...) BHO_DESCRIBE_BASES(__VA_ARGS__)
#define BHO_DESCRIBE_PUBLIC_MEMBERS_(...) BHO_DESCRIBE_PUBLIC_MEMBERS(__VA_ARGS__)
#define BHO_DESCRIBE_PROTECTED_MEMBERS_(...) BHO_DESCRIBE_PROTECTED_MEMBERS(__VA_ARGS__)
#define BHO_DESCRIBE_PRIVATE_MEMBERS_(...) BHO_DESCRIBE_PRIVATE_MEMBERS(__VA_ARGS__)

#define BHO_DESCRIBE_CLASS(C, Bases, Public, Protected, Private) \
    BHO_DESCRIBE_MAYBE_UNUSED friend BHO_DESCRIBE_BASES_(C BHO_DESCRIBE_PP_UNPACK Bases) \
    BHO_DESCRIBE_MAYBE_UNUSED friend BHO_DESCRIBE_PUBLIC_MEMBERS_(C BHO_DESCRIBE_PP_UNPACK Public) \
    BHO_DESCRIBE_MAYBE_UNUSED friend BHO_DESCRIBE_PROTECTED_MEMBERS_(C BHO_DESCRIBE_PP_UNPACK Protected) \
    BHO_DESCRIBE_MAYBE_UNUSED friend BHO_DESCRIBE_PRIVATE_MEMBERS_(C BHO_DESCRIBE_PP_UNPACK Private)

#define BHO_DESCRIBE_STRUCT(C, Bases, Members) \
    static_assert(std::is_class<C>::value || std::is_union<C>::value, "BHO_DESCRIBE_STRUCT should only be used with class types"); \
    BHO_DESCRIBE_MAYBE_UNUSED BHO_DESCRIBE_BASES_(C BHO_DESCRIBE_PP_UNPACK Bases) \
    BHO_DESCRIBE_MAYBE_UNUSED BHO_DESCRIBE_PUBLIC_MEMBERS_(C BHO_DESCRIBE_PP_UNPACK Members) \
    BHO_DESCRIBE_MAYBE_UNUSED BHO_DESCRIBE_PROTECTED_MEMBERS_(C) \
    BHO_DESCRIBE_MAYBE_UNUSED BHO_DESCRIBE_PRIVATE_MEMBERS_(C)

#endif

} // namespace describe
} // namespace bho

#endif // !defined(BHO_DESCRIBE_CXX14)

#endif // #ifndef BHO_DESCRIBE_CLASS_HPP_INCLUDED
