/*
Copyright 2018 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#ifndef BHO_CORE_EXCHANGE_HPP
#define BHO_CORE_EXCHANGE_HPP

#include <asio2/bho/config.hpp>
#if !defined(BHO_NO_CXX11_RVALUE_REFERENCES)
#include <asio2/bho/config/workaround.hpp>
#include <utility>
#endif

namespace bho {

#if defined(BHO_NO_CXX11_RVALUE_REFERENCES)
template<class T, class U>
inline T exchange(T& t, const U& u)
{
    T v = t;
    t = u;
    return v;
}
#else
#if BHO_WORKAROUND(BHO_MSVC, < 1800)
template<class T, class U>
inline T exchange(T& t, U&& u)
{
    T v = std::move(t);
    t = std::forward<U>(u);
    return v;
}
#else
template<class T, class U = T>
BHO_CXX14_CONSTEXPR inline T exchange(T& t, U&& u)
{
    T v = std::move(t);
    t = std::forward<U>(u);
    return v;
}
#endif
#endif

} /* boost */

#endif
