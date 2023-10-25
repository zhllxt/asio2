/*
Copyright 2019 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#ifndef BHO_CORE_ALLOC_CONSTRUCT_HPP
#define BHO_CORE_ALLOC_CONSTRUCT_HPP

/*
This functionality is now in <asio2/bho/core/allocator_access.hpp>.
*/
#include <asio2/bho/core/noinit_adaptor.hpp>

namespace bho {

template<class A, class T>
inline void
alloc_destroy(A& a, T* p)
{
    bho::allocator_destroy(a, p);
}

template<class A, class T>
inline void
alloc_destroy_n(A& a, T* p, std::size_t n)
{
    bho::allocator_destroy_n(a, p, n);
}

template<class A, class T>
inline void
alloc_construct(A& a, T* p)
{
    bho::allocator_construct(a, p);
}

#if !defined(BHO_NO_CXX11_RVALUE_REFERENCES)
#if !defined(BHO_NO_CXX11_VARIADIC_TEMPLATES)
template<class A, class T, class U, class... V>
inline void
alloc_construct(A& a, T* p, U&& u, V&&... v)
{
    bho::allocator_construct(a, p, std::forward<U>(u),
        std::forward<V>(v)...);
}
#else
template<class A, class T, class U>
inline void
alloc_construct(A& a, T* p, U&& u)
{
    bho::allocator_construct(a, p, std::forward<U>(u));
}
#endif
#else
template<class A, class T, class U>
inline void
alloc_construct(A& a, T* p, const U& u)
{
    bho::allocator_construct(a, p, u);
}

template<class A, class T, class U>
inline void
alloc_construct(A& a, T* p, U& u)
{
    bho::allocator_construct(a, p, u);
}
#endif

template<class A, class T>
inline void
alloc_construct_n(A& a, T* p, std::size_t n)
{
    bho::allocator_construct_n(a, p, n);
}

template<class A, class T>
inline void
alloc_construct_n(A& a, T* p, std::size_t n, const T* l, std::size_t m)
{
    bho::allocator_construct_n(a, p, n, l, m);
}

template<class A, class T, class I>
inline void
alloc_construct_n(A& a, T* p, std::size_t n, I b)
{
    bho::allocator_construct_n(a, p, n, b);
}

} /* boost */

#endif
