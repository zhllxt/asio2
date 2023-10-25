#ifndef BHO_DESCRIBE_ENUM_HPP_INCLUDED
#define BHO_DESCRIBE_ENUM_HPP_INCLUDED

// Copyright 2020 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <asio2/bho/describe/detail/config.hpp>

#if !defined(BHO_DESCRIBE_CXX14)

#define BHO_DESCRIBE_ENUM(E, ...)
#define BHO_DESCRIBE_NESTED_ENUM(E, ...)

#else

#include <asio2/bho/describe/detail/pp_for_each.hpp>
#include <asio2/bho/describe/detail/list.hpp>
#include <type_traits>

namespace bho
{
namespace describe
{
namespace detail
{

template<class D> struct enum_descriptor
{
    // can't use auto here because of the need to supply the definitions below
    static constexpr decltype(D::value()) value = D::value();
    static constexpr decltype(D::name()) name = D::name();
};

#ifndef __cpp_inline_variables
// GCC requires these definitions
template<class D> constexpr decltype(D::value()) enum_descriptor<D>::value;
template<class D> constexpr decltype(D::name()) enum_descriptor<D>::name;
#endif

template<class... T> auto enum_descriptor_fn_impl( int, T... )
{
    return list<enum_descriptor<T>...>();
}

#define BHO_DESCRIBE_ENUM_BEGIN(E) \
    inline auto bho_enum_descriptor_fn( E** ) \
    { return bho::describe::detail::enum_descriptor_fn_impl( 0

#define BHO_DESCRIBE_ENUM_ENTRY(E, e) , []{ struct _bho_desc { \
    static constexpr auto value() noexcept { return E::e; } \
    static constexpr auto name() noexcept { return #e; } }; return _bho_desc(); }()

#define BHO_DESCRIBE_ENUM_END(E) ); }

} // namespace detail

#if defined(_MSC_VER) && !defined(__clang__)

#define BHO_DESCRIBE_ENUM(E, ...) \
    namespace should_use_BHO_DESCRIBE_NESTED_ENUM {} \
    static_assert(std::is_enum<E>::value, "BHO_DESCRIBE_ENUM should only be used with enums"); \
    BHO_DESCRIBE_ENUM_BEGIN(E) \
    BHO_DESCRIBE_PP_FOR_EACH(BHO_DESCRIBE_ENUM_ENTRY, E, __VA_ARGS__) \
    BHO_DESCRIBE_ENUM_END(E)

#define BHO_DESCRIBE_NESTED_ENUM(E, ...) \
    static_assert(std::is_enum<E>::value, "BHO_DESCRIBE_NESTED_ENUM should only be used with enums"); \
    friend BHO_DESCRIBE_ENUM_BEGIN(E) \
    BHO_DESCRIBE_PP_FOR_EACH(BHO_DESCRIBE_ENUM_ENTRY, E, __VA_ARGS__) \
    BHO_DESCRIBE_ENUM_END(E)

#else

#define BHO_DESCRIBE_ENUM(E, ...) \
    namespace should_use_BHO_DESCRIBE_NESTED_ENUM {} \
    static_assert(std::is_enum<E>::value, "BHO_DESCRIBE_ENUM should only be used with enums"); \
    BHO_DESCRIBE_MAYBE_UNUSED BHO_DESCRIBE_ENUM_BEGIN(E) \
    BHO_DESCRIBE_PP_FOR_EACH(BHO_DESCRIBE_ENUM_ENTRY, E, ##__VA_ARGS__) \
    BHO_DESCRIBE_ENUM_END(E)

#define BHO_DESCRIBE_NESTED_ENUM(E, ...) \
    static_assert(std::is_enum<E>::value, "BHO_DESCRIBE_NESTED_ENUM should only be used with enums"); \
    BHO_DESCRIBE_MAYBE_UNUSED friend BHO_DESCRIBE_ENUM_BEGIN(E) \
    BHO_DESCRIBE_PP_FOR_EACH(BHO_DESCRIBE_ENUM_ENTRY, E, ##__VA_ARGS__) \
    BHO_DESCRIBE_ENUM_END(E)

#endif

} // namespace describe
} // namespace bho

#endif // defined(BHO_DESCRIBE_CXX14)

#if defined(_MSC_VER) && !defined(__clang__)

#define BHO_DEFINE_ENUM(E, ...) enum E { __VA_ARGS__ }; BHO_DESCRIBE_ENUM(E, __VA_ARGS__)
#define BHO_DEFINE_ENUM_CLASS(E, ...) enum class E { __VA_ARGS__ }; BHO_DESCRIBE_ENUM(E, __VA_ARGS__)

#define BHO_DEFINE_FIXED_ENUM(E, Base, ...) enum E: Base { __VA_ARGS__ }; BHO_DESCRIBE_ENUM(E, __VA_ARGS__)
#define BHO_DEFINE_FIXED_ENUM_CLASS(E, Base, ...) enum class E: Base { __VA_ARGS__ }; BHO_DESCRIBE_ENUM(E, __VA_ARGS__)

#else

#define BHO_DEFINE_ENUM(E, ...) enum E { __VA_ARGS__ }; BHO_DESCRIBE_ENUM(E, ##__VA_ARGS__)
#define BHO_DEFINE_ENUM_CLASS(E, ...) enum class E { __VA_ARGS__ }; BHO_DESCRIBE_ENUM(E, ##__VA_ARGS__)

#define BHO_DEFINE_FIXED_ENUM(E, Base, ...) enum E: Base { __VA_ARGS__ }; BHO_DESCRIBE_ENUM(E, ##__VA_ARGS__)
#define BHO_DEFINE_FIXED_ENUM_CLASS(E, Base, ...) enum class E: Base { __VA_ARGS__ }; BHO_DESCRIBE_ENUM(E, ##__VA_ARGS__)

#endif

#endif // #ifndef BHO_DESCRIBE_ENUM_HPP_INCLUDED
