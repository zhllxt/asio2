#ifndef BHO_DESCRIBE_DETAIL_BASES_HPP_INCLUDED
#define BHO_DESCRIBE_DETAIL_BASES_HPP_INCLUDED

// Copyright 2020 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <asio2/bho/describe/detail/compute_base_modifiers.hpp>
#include <asio2/bho/describe/detail/pp_for_each.hpp>
#include <asio2/bho/describe/detail/list.hpp>
#include <type_traits>

namespace bho
{
namespace describe
{
namespace detail
{

// base_descriptor
template<class C, class B> struct base_descriptor
{
    static_assert( std::is_base_of<B, C>::value, "A type listed as a base is not one" );

    using type = B;
    static constexpr unsigned modifiers = compute_base_modifiers<C, B>();
};

#ifndef __cpp_inline_variables
template<class C, class B> constexpr unsigned base_descriptor<C, B>::modifiers;
#endif

template<class... T> auto base_descriptor_fn_impl( int, T... )
{
    return list<T...>();
}

#define BHO_DESCRIBE_BASE_IMPL(C, B) , bho::describe::detail::base_descriptor<C, B>()

#if defined(_MSC_VER) && !defined(__clang__)

#define BHO_DESCRIBE_BASES(C, ...) inline auto bho_base_descriptor_fn( C** ) \
{ return bho::describe::detail::base_descriptor_fn_impl( 0 BHO_DESCRIBE_PP_FOR_EACH(BHO_DESCRIBE_BASE_IMPL, C, __VA_ARGS__) ); }

#else

#define BHO_DESCRIBE_BASES(C, ...) inline auto bho_base_descriptor_fn( C** ) \
{ return bho::describe::detail::base_descriptor_fn_impl( 0 BHO_DESCRIBE_PP_FOR_EACH(BHO_DESCRIBE_BASE_IMPL, C, ##__VA_ARGS__) ); }

#endif

} // namespace detail
} // namespace describe
} // namespace bho

#endif // #ifndef BHO_DESCRIBE_DETAIL_BASES_HPP_INCLUDED
