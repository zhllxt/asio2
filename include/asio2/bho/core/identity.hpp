/*
Copyright 2021-2023 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#ifndef BHO_CORE_IDENTITY_HPP
#define BHO_CORE_IDENTITY_HPP

#include <asio2/bho/config.hpp>
#if !defined(BHO_NO_CXX11_RVALUE_REFERENCES)
#include <utility>
#endif

namespace bho {

struct identity {
    typedef void is_transparent;

#if !defined(BHO_NO_CXX11_RVALUE_REFERENCES)
    template<class T>
    BHO_CONSTEXPR T&& operator()(T&& value) const BHO_NOEXCEPT {
        return std::forward<T>(value);
    }
#else
    template<class T>
    BHO_CONSTEXPR const T& operator()(const T& value) const BHO_NOEXCEPT {
        return value;
    }

    template<class T>
    BHO_CONSTEXPR T& operator()(T& value) const BHO_NOEXCEPT {
        return value;
    }
#endif

    template<class>
    struct result { };

    template<class T>
    struct result<identity(T&)> {
        typedef T& type;
    };

#if !defined(BHO_NO_CXX11_RVALUE_REFERENCES)
    template<class T>
    struct result<identity(T)> {
        typedef T&& type;
    };

    template<class T>
    struct result<identity(T&&)> {
        typedef T&& type;
    };
#endif
};

} /* boost */

#endif
