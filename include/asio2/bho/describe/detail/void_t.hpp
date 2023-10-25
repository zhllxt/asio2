#ifndef BHO_DESCRIBE_DETAIL_VOID_T_HPP_INCLUDED
#define BHO_DESCRIBE_DETAIL_VOID_T_HPP_INCLUDED

// Copyright 2021 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <asio2/bho/describe/detail/config.hpp>

#if defined(BHO_DESCRIBE_CXX11)

namespace bho
{
namespace describe
{
namespace detail
{

template<class...> struct make_void
{
    using type = void;
};

template<class... T> using void_t = typename make_void<T...>::type;

} // namespace detail
} // namespace describe
} // namespace bho

#endif // defined(BHO_DESCRIBE_CXX11)

#endif // #ifndef BHO_DESCRIBE_DETAIL_VOID_T_HPP_INCLUDED
