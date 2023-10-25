/*
Copyright 2019 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#ifndef BHO_CORE_NOINIT_ADAPTOR_HPP
#define BHO_CORE_NOINIT_ADAPTOR_HPP

#include <asio2/bho/core/allocator_access.hpp>

namespace bho {

template<class A>
struct noinit_adaptor
    : A {
    typedef void _default_construct_destroy;

    template<class U>
    struct rebind {
        typedef noinit_adaptor<typename allocator_rebind<A, U>::type> other;
    };

    noinit_adaptor()
        : A() { }

#if !defined(BHO_NO_CXX11_RVALUE_REFERENCES)
    template<class U>
    noinit_adaptor(U&& u) BHO_NOEXCEPT
        : A(std::forward<U>(u)) { }
#else
    template<class U>
    noinit_adaptor(const U& u) BHO_NOEXCEPT
        : A(u) { }

    template<class U>
    noinit_adaptor(U& u) BHO_NOEXCEPT
        : A(u) { }
#endif

    template<class U>
    noinit_adaptor(const noinit_adaptor<U>& u) BHO_NOEXCEPT
        : A(static_cast<const A&>(u)) { }

    template<class U>
    void construct(U* p) {
        ::new((void*)p) U;
    }

#if defined(BHO_NO_CXX11_ALLOCATOR)
    template<class U, class V>
    void construct(U* p, const V& v) {
        ::new((void*)p) U(v);
    }
#endif

    template<class U>
    void destroy(U* p) {
        p->~U();
        (void)p;
    }
};

template<class T, class U>
inline bool
operator==(const noinit_adaptor<T>& lhs,
    const noinit_adaptor<U>& rhs) BHO_NOEXCEPT
{
    return static_cast<const T&>(lhs) == static_cast<const U&>(rhs);
}

template<class T, class U>
inline bool
operator!=(const noinit_adaptor<T>& lhs,
    const noinit_adaptor<U>& rhs) BHO_NOEXCEPT
{
    return !(lhs == rhs);
}

template<class A>
inline noinit_adaptor<A>
noinit_adapt(const A& a) BHO_NOEXCEPT
{
    return noinit_adaptor<A>(a);
}

} /* boost */

#endif
