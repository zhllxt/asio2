/*
Copyright 2023 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#ifndef BHO_CORE_SIZE_HPP
#define BHO_CORE_SIZE_HPP

#include <cstddef>

namespace bho {

template<class C>
inline constexpr auto
size(const C& c) noexcept(noexcept(c.size())) -> decltype(c.size())
{
    return c.size();
}

template<class T, std::size_t N>
inline constexpr std::size_t
size(T(&)[N]) noexcept
{
    return N;
}

} /* boost */

#endif
