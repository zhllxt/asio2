/*
Copyright 2018 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#ifndef BHO_CORE_EMPTY_VALUE_HPP
#define BHO_CORE_EMPTY_VALUE_HPP

#include <asio2/bho/config.hpp>
#if !defined(BHO_NO_CXX11_RVALUE_REFERENCES)
#include <utility>
#endif

#if defined(BHO_GCC_VERSION) && (BHO_GCC_VERSION >= 40700)
#define BHO_DETAIL_EMPTY_VALUE_BASE
#elif defined(BHO_INTEL) && defined(_MSC_VER) && (_MSC_VER >= 1800)
#define BHO_DETAIL_EMPTY_VALUE_BASE
#elif defined(BHO_MSVC) && (BHO_MSVC >= 1800)
#define BHO_DETAIL_EMPTY_VALUE_BASE
#elif defined(BHO_CLANG) && !defined(__CUDACC__)
#if __has_feature(is_empty) && __has_feature(is_final)
#define BHO_DETAIL_EMPTY_VALUE_BASE
#endif
#endif

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4510)
#endif

namespace bho {

template<class T>
struct use_empty_value_base {
    enum {
#if defined(BHO_DETAIL_EMPTY_VALUE_BASE)
        value = __is_empty(T) && !__is_final(T)
#else
        value = false
#endif
    };
};

struct empty_init_t { };

namespace empty_ {

template<class T, unsigned N = 0,
    bool E = bho::use_empty_value_base<T>::value>
class empty_value {
public:
    typedef T type;

#if !defined(BHO_NO_CXX11_DEFAULTED_FUNCTIONS)
    empty_value() = default;
#else
    BHO_CONSTEXPR empty_value() { }
#endif

    BHO_CONSTEXPR empty_value(bho::empty_init_t)
        : value_() { }

#if !defined(BHO_NO_CXX11_RVALUE_REFERENCES)
#if !defined(BHO_NO_CXX11_VARIADIC_TEMPLATES)
    template<class U, class... Args>
    BHO_CONSTEXPR empty_value(bho::empty_init_t, U&& value, Args&&... args)
        : value_(std::forward<U>(value), std::forward<Args>(args)...) { }
#else
    template<class U>
    BHO_CONSTEXPR empty_value(bho::empty_init_t, U&& value)
        : value_(std::forward<U>(value)) { }
#endif
#else
    template<class U>
    BHO_CONSTEXPR empty_value(bho::empty_init_t, const U& value)
        : value_(value) { }

    template<class U>
    BHO_CONSTEXPR empty_value(bho::empty_init_t, U& value)
        : value_(value) { }
#endif

    BHO_CONSTEXPR const T& get() const BHO_NOEXCEPT {
        return value_;
    }

    BHO_CXX14_CONSTEXPR T& get() BHO_NOEXCEPT {
        return value_;
    }

private:
    T value_;
};

#if !defined(BHO_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
template<class T, unsigned N>
class empty_value<T, N, true>
    : T {
public:
    typedef T type;

#if !defined(BHO_NO_CXX11_DEFAULTED_FUNCTIONS)
    empty_value() = default;
#else
    BHO_CONSTEXPR empty_value() { }
#endif

    BHO_CONSTEXPR empty_value(bho::empty_init_t)
        : T() { }

#if !defined(BHO_NO_CXX11_RVALUE_REFERENCES)
#if !defined(BHO_NO_CXX11_VARIADIC_TEMPLATES)
    template<class U, class... Args>
    BHO_CONSTEXPR empty_value(bho::empty_init_t, U&& value, Args&&... args)
        : T(std::forward<U>(value), std::forward<Args>(args)...) { }
#else
    template<class U>
    BHO_CONSTEXPR empty_value(bho::empty_init_t, U&& value)
        : T(std::forward<U>(value)) { }
#endif
#else
    template<class U>
    BHO_CONSTEXPR empty_value(bho::empty_init_t, const U& value)
        : T(value) { }

    template<class U>
    BHO_CONSTEXPR empty_value(bho::empty_init_t, U& value)
        : T(value) { }
#endif

    BHO_CONSTEXPR const T& get() const BHO_NOEXCEPT {
        return *this;
    }

    BHO_CXX14_CONSTEXPR T& get() BHO_NOEXCEPT {
        return *this;
    }
};
#endif

} /* empty_ */

using empty_::empty_value;

BHO_INLINE_CONSTEXPR empty_init_t empty_init = empty_init_t();

} /* boost */

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif
