#ifndef BHO_MP11_MPL_TUPLE_HPP_INCLUDED
#define BHO_MP11_MPL_TUPLE_HPP_INCLUDED

// Copyright 2017, 2019 Peter Dimov.
//
// Distributed under the Boost Software License, Version 1.0.
//
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt

#include <asio2/bho/mp11/detail/mpl_common.hpp>
#include <tuple>

namespace bho
{
namespace mpl
{

template< typename Sequence > struct sequence_tag;

template<class... T> struct sequence_tag<std::tuple<T...>>
{
    using type = aux::mp11_tag;
};

} // namespace mpl
} // namespace bho

#endif // #ifndef BHO_MP11_MPL_TUPLE_HPP_INCLUDED
