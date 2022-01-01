/*
Copyright 2019 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#ifndef BHO_CORE_DEFAULT_ALLOCATOR_HPP
#define BHO_CORE_DEFAULT_ALLOCATOR_HPP

#include <asio2/bho/config.hpp>
#include <new>

namespace bho {

#if defined(BHO_NO_EXCEPTIONS)
BHO_NORETURN void throw_exception(const std::exception&);
#endif

namespace default_ {

struct true_type {
    typedef bool value_type;
    typedef true_type type;

    BHO_STATIC_CONSTANT(bool, value = true);

    BHO_CONSTEXPR operator bool() const BHO_NOEXCEPT {
        return true;
    }

    BHO_CONSTEXPR bool operator()() const BHO_NOEXCEPT {
        return true;
    }
};

template<class T>
struct add_reference {
    typedef T& type;
};

template<>
struct add_reference<void> {
    typedef void type;
};

template<>
struct add_reference<const void> {
    typedef const void type;
};

template<class T>
struct default_allocator {
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef typename add_reference<T>::type reference;
    typedef typename add_reference<const T>::type const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef true_type propagate_on_container_move_assignment;
    typedef true_type is_always_equal;

    template<class U>
    struct rebind {
        typedef default_allocator<U> other;
    };

#if !defined(BHO_NO_CXX11_DEFAULTED_FUNCTIONS)
    default_allocator() = default;
#else
    BHO_CONSTEXPR default_allocator() BHO_NOEXCEPT { }
#endif

    template<class U>
    BHO_CONSTEXPR default_allocator(const default_allocator<U>&)
        BHO_NOEXCEPT { }

    BHO_CONSTEXPR std::size_t max_size() const BHO_NOEXCEPT {
        return static_cast<std::size_t>(-1) / (2 < sizeof(T) ? sizeof(T) : 2);
    }

#if !defined(BHO_NO_EXCEPTIONS)
    T* allocate(std::size_t n) {
        if (n > max_size()) {
            throw std::bad_alloc();
        }
        return static_cast<T*>(::operator new(sizeof(T) * n));
    }

    void deallocate(T* p, std::size_t) {
        ::operator delete(p);
    }
#else
    T* allocate(std::size_t n) {
        if (n > max_size()) {
            bho::throw_exception(std::bad_alloc());
        }
        void* p = ::operator new(sizeof(T) * n, std::nothrow);
        if (!p) {
            bho::throw_exception(std::bad_alloc());
        }
        return static_cast<T*>(p);
    }

    void deallocate(T* p, std::size_t) {
        ::operator delete(p, std::nothrow);
    }
#endif

#if (defined(BHO_LIBSTDCXX_VERSION) && BHO_LIBSTDCXX_VERSION < 60000) || \
    defined(BHO_NO_CXX11_ALLOCATOR)
    template<class U, class V>
    void construct(U* p, const V& v) {
        ::new(p) U(v);
    }

    template<class U>
    void destroy(U* p) {
        p->~U();
        (void)p;
    }
#endif
};

template<class T, class U>
BHO_CONSTEXPR inline bool
operator==(const default_allocator<T>&,
    const default_allocator<U>&) BHO_NOEXCEPT
{
    return true;
}

template<class T, class U>
BHO_CONSTEXPR inline bool
operator!=(const default_allocator<T>&,
    const default_allocator<U>&) BHO_NOEXCEPT
{
    return false;
}

} /* default_ */

using default_::default_allocator;

} /* boost */

#endif
