/*
Copyright 2017-2021 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#ifndef BHO_CORE_POINTER_TRAITS_HPP
#define BHO_CORE_POINTER_TRAITS_HPP

#include <asio2/bho/config.hpp>
#include <asio2/bho/core/addressof.hpp>
#include <cstddef>

namespace bho {
namespace detail {

struct ptr_none { };

template<class>
struct ptr_valid {
    typedef void type;
};

template<class>
struct ptr_first {
    typedef ptr_none type;
};

#if !defined(BHO_NO_CXX11_VARIADIC_TEMPLATES)
template<template<class, class...> class T, class U, class... Args>
struct ptr_first<T<U, Args...> > {
    typedef U type;
};
#else
template<template<class> class T, class U>
struct ptr_first<T<U> > {
    typedef U type;
};

template<template<class, class> class T, class U1, class U2>
struct ptr_first<T<U1, U2> > {
    typedef U1 type;
};

template<template<class, class, class> class T, class U1, class U2, class U3>
struct ptr_first<T<U1, U2, U3> > {
    typedef U1 type;
};
#endif

template<class T, class = void>
struct ptr_element {
    typedef typename ptr_first<T>::type type;
};

template<class T>
struct ptr_element<T, typename ptr_valid<typename T::element_type>::type> {
    typedef typename T::element_type type;
};

template<class, class = void>
struct ptr_difference {
    typedef std::ptrdiff_t type;
};

template<class T>
struct ptr_difference<T,
    typename ptr_valid<typename T::difference_type>::type> {
    typedef typename T::difference_type type;
};

template<class, class>
struct ptr_transform { };

#if !defined(BHO_NO_CXX11_VARIADIC_TEMPLATES)
template<template<class, class...> class T, class U, class... Args, class V>
struct ptr_transform<T<U, Args...>, V> {
    typedef T<V, Args...> type;
};
#else
template<template<class> class T, class U, class V>
struct ptr_transform<T<U>, V> {
    typedef T<V> type;
};

template<template<class, class> class T, class U1, class U2, class V>
struct ptr_transform<T<U1, U2>, V> {
    typedef T<V, U2> type;
};

template<template<class, class, class> class T,
    class U1, class U2, class U3, class V>
struct ptr_transform<T<U1, U2, U3>, V> {
    typedef T<V, U2, U3> type;
};
#endif

template<class T, class U, class = void>
struct ptr_rebind
    : ptr_transform<T, U> { };

#if !defined(BHO_NO_CXX11_TEMPLATE_ALIASES)
template<class T, class U>
struct ptr_rebind<T, U,
    typename ptr_valid<typename T::template rebind<U> >::type> {
    typedef typename T::template rebind<U> type;
};
#else
template<class T, class U>
struct ptr_rebind<T, U,
    typename ptr_valid<typename T::template rebind<U>::other>::type> {
    typedef typename T::template rebind<U>::other type;
};
#endif

#if !defined(BHO_NO_CXX11_DECLTYPE_N3276)
template<class T, class E>
class ptr_to_expr {
    template<class>
    struct result {
        char x, y;
    };

    static E& source();

    template<class O>
    static auto check(int) -> result<decltype(O::pointer_to(source()))>;

    template<class>
    static char check(long);

public:
    BHO_STATIC_CONSTEXPR bool value = sizeof(check<T>(0)) > 1;
};

template<class T, class E>
struct ptr_to_expr<T*, E> {
    BHO_STATIC_CONSTEXPR bool value = true;
};

template<class T, class E>
struct ptr_has_to {
    BHO_STATIC_CONSTEXPR bool value = ptr_to_expr<T, E>::value;
};
#else
template<class, class>
struct ptr_has_to {
    BHO_STATIC_CONSTEXPR bool value = true;
};
#endif

template<class T>
struct ptr_has_to<T, void> {
    BHO_STATIC_CONSTEXPR bool value = false;
};

template<class T>
struct ptr_has_to<T, const void> {
    BHO_STATIC_CONSTEXPR bool value = false;
};

template<class T>
struct ptr_has_to<T, volatile void> {
    BHO_STATIC_CONSTEXPR bool value = false;
};

template<class T>
struct ptr_has_to<T, const volatile void> {
    BHO_STATIC_CONSTEXPR bool value = false;
};

template<class T, class E, bool = ptr_has_to<T, E>::value>
struct ptr_to { };

template<class T, class E>
struct ptr_to<T, E, true> {
    static T pointer_to(E& v) {
        return T::pointer_to(v);
    }
};

template<class T>
struct ptr_to<T*, T, true> {
    static T* pointer_to(T& v) BHO_NOEXCEPT {
        return bho::addressof(v);
    }
};

template<class T, class E>
struct ptr_traits
    : ptr_to<T, E> {
    typedef T pointer;
    typedef E element_type;
    typedef typename ptr_difference<T>::type difference_type;

    template<class U>
    struct rebind_to
        : ptr_rebind<T, U> { };

#if !defined(BHO_NO_CXX11_TEMPLATE_ALIASES)
    template<class U>
    using rebind = typename rebind_to<U>::type;
#endif
};

template<class T>
struct ptr_traits<T, ptr_none> { };

} /* detail */

template<class T>
struct pointer_traits
    : detail::ptr_traits<T, typename detail::ptr_element<T>::type> { };

template<class T>
struct pointer_traits<T*>
    : detail::ptr_to<T*, T> {
    typedef T* pointer;
    typedef T element_type;
    typedef std::ptrdiff_t difference_type;

    template<class U>
    struct rebind_to {
        typedef U* type;
    };

#if !defined(BHO_NO_CXX11_TEMPLATE_ALIASES)
    template<class U>
    using rebind = typename rebind_to<U>::type;
#endif
};

template<class T>
BHO_CONSTEXPR inline T*
to_address(T* v) BHO_NOEXCEPT
{
    return v;
}

#if !defined(BHO_NO_CXX14_RETURN_TYPE_DEDUCTION)
namespace detail {

template<class T>
inline T*
ptr_address(T* v, int) BHO_NOEXCEPT
{
    return v;
}

template<class T>
inline auto
ptr_address(const T& v, int) BHO_NOEXCEPT
-> decltype(bho::pointer_traits<T>::to_address(v))
{
    return bho::pointer_traits<T>::to_address(v);
}

template<class T>
inline auto
ptr_address(const T& v, long) BHO_NOEXCEPT
{
    return bho::detail::ptr_address(v.operator->(), 0);
}

} /* detail */

template<class T>
inline auto
to_address(const T& v) BHO_NOEXCEPT
{
    return bho::detail::ptr_address(v, 0);
}
#else
template<class T>
inline typename pointer_traits<T>::element_type*
to_address(const T& v) BHO_NOEXCEPT
{
    return bho::to_address(v.operator->());
}
#endif

} /* boost */

#endif
