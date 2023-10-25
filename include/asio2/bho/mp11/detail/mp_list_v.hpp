#ifndef BHO_MP11_DETAIL_MP_LIST_V_HPP_INCLUDED
#define BHO_MP11_DETAIL_MP_LIST_V_HPP_INCLUDED

// Copyright 2023 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#include <asio2/bho/mp11/detail/config.hpp>

namespace bho
{
namespace mp11
{

#if defined(BHO_MP11_HAS_TEMPLATE_AUTO)

// mp_list_v<A...>
template<auto... A> struct mp_list_v
{
};

#endif

} // namespace mp11
} // namespace bho

#endif // #ifndef BHO_MP11_DETAIL_MP_LIST_V_HPP_INCLUDED
