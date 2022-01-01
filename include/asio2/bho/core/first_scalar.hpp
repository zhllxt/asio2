/*
Copyright 2019 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#ifndef BHO_CORE_FIRST_SCALAR_HPP
#define BHO_CORE_FIRST_SCALAR_HPP

#include <asio2/bho/config.hpp>
#include <cstddef>

namespace bho {
namespace detail {

template<class T>
struct make_scalar {
    typedef T type;
};

template<class T, std::size_t N>
struct make_scalar<T[N]> {
    typedef typename make_scalar<T>::type type;
};

} /* detail */

template<class T>
BHO_CONSTEXPR inline T*
first_scalar(T* p) BHO_NOEXCEPT
{
    return p;
}

template<class T, std::size_t N>
BHO_CONSTEXPR inline typename detail::make_scalar<T>::type*
first_scalar(T (*p)[N]) BHO_NOEXCEPT
{
    return bho::first_scalar(&(*p)[0]);
}

} /* boost */

#endif
