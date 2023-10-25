#ifndef BHO_DESCRIBE_DESCRIPTOR_BY_NAME_HPP_INCLUDED
#define BHO_DESCRIBE_DESCRIPTOR_BY_NAME_HPP_INCLUDED

// Copyright 2021 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <asio2/bho/describe/detail/cx_streq.hpp>
#include <asio2/bho/describe/detail/config.hpp>

#if defined(BHO_DESCRIBE_CXX14)

#include <asio2/bho/mp11/algorithm.hpp>
#include <asio2/bho/mp11/bind.hpp>
#include <asio2/bho/mp11/integral.hpp>

namespace bho
{
namespace describe
{

namespace detail
{

template<class D, class N> using match_by_name = mp11::mp_bool<cx_streq(N::name(), D::name)>;

#define BHO_DESCRIBE_MAKE_NAME_IMPL2(s, k) struct _bho_name_##s##_##k { static constexpr char const * name() { return #s; } }
#define BHO_DESCRIBE_MAKE_NAME_IMPL(s, k) BHO_DESCRIBE_MAKE_NAME_IMPL2(s, k)

} // namespace detail

#define BHO_DESCRIBE_MAKE_NAME(s) BHO_DESCRIBE_MAKE_NAME_IMPL(s, __LINE__)

template<class L, class N> using descriptor_by_name = mp11::mp_at<L, mp11::mp_find_if_q<L, mp11::mp_bind_back<detail::match_by_name, N>>>;

} // namespace describe
} // namespace bho

#endif // defined(BHO_DESCRIBE_CXX14)

#endif // #ifndef BHO_DESCRIBE_DESCRIPTOR_BY_NAME_HPP_INCLUDED
