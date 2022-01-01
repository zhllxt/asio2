/*
Copyright 2019 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#ifndef BHO_CORE_ALLOC_CONSTRUCT_HPP
#define BHO_CORE_ALLOC_CONSTRUCT_HPP

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
    while (n > 0) {
        bho::allocator_destroy(a, p + --n);
    }
}

template<class A, class T>
inline void
alloc_destroy(noinit_adaptor<A>&, T* p)
{
    p->~T();
}

template<class A, class T>
inline void
alloc_destroy_n(noinit_adaptor<A>&, T* p, std::size_t n)
{
    while (n > 0) {
        p[--n].~T();
    }
}

namespace detail {

template<class A, class T>
class alloc_destroyer {
public:
    alloc_destroyer(A& a, T* p) BHO_NOEXCEPT
        : a_(a),
          p_(p),
          n_(0) { }

    ~alloc_destroyer() {
        bho::alloc_destroy_n(a_, p_, n_);
    }

    std::size_t& size() BHO_NOEXCEPT {
        return n_;
    }

private:
    alloc_destroyer(const alloc_destroyer&);
    alloc_destroyer& operator=(const alloc_destroyer&);

    A& a_;
    T* p_;
    std::size_t n_;
};

} /* detail */

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
    detail::alloc_destroyer<A, T> hold(a, p);
    for (std::size_t& i = hold.size(); i < n; ++i) {
        bho::allocator_construct(a, p + i);
    }
    hold.size() = 0;
}

template<class A, class T>
inline void
alloc_construct_n(A& a, T* p, std::size_t n, const T* l, std::size_t m)
{
    detail::alloc_destroyer<A, T> hold(a, p);
    for (std::size_t& i = hold.size(); i < n; ++i) {
        bho::allocator_construct(a, p + i, l[i % m]);
    }
    hold.size() = 0;
}

template<class A, class T, class I>
inline void
alloc_construct_n(A& a, T* p, std::size_t n, I b)
{
    detail::alloc_destroyer<A, T> hold(a, p);
    for (std::size_t& i = hold.size(); i < n; void(++i), void(++b)) {
        bho::allocator_construct(a, p + i, *b);
    }
    hold.size() = 0;
}

template<class A, class T>
inline void
alloc_construct(noinit_adaptor<A>&, T* p)
{
    ::new(static_cast<void*>(p)) T;
}

template<class A, class T>
inline void
alloc_construct_n(noinit_adaptor<A>& a, T* p, std::size_t n)
{
    detail::alloc_destroyer<noinit_adaptor<A>, T> hold(a, p);
    for (std::size_t& i = hold.size(); i < n; ++i) {
        ::new(static_cast<void*>(p + i)) T;
    }
    hold.size() = 0;
}

} /* boost */

#endif
