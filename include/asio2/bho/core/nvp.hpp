/*
Copyright 2019 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#ifndef BHO_CORE_NVP_HPP
#define BHO_CORE_NVP_HPP

#include <asio2/bho/core/addressof.hpp>
#include <asio2/bho/config.hpp>

namespace bho {
namespace serialization {

template<class T>
class nvp {
public:
    nvp(const char* n, T& v) BHO_NOEXCEPT
        : n_(n)
        , v_(bho::addressof(v)) { }

    const char* name() const BHO_NOEXCEPT {
        return n_;
    }

    T& value() const BHO_NOEXCEPT {
        return *v_;
    }

    const T& const_value() const BHO_NOEXCEPT {
        return *v_;
    }

private:
    const char* n_;
    T* v_;
};

template<class T>
inline const nvp<T>
make_nvp(const char* n, T& v) BHO_NOEXCEPT
{
    return nvp<T>(n, v);
}

} /* serialization */

using serialization::nvp;
using serialization::make_nvp;

} /* boost */

#define BHO_NVP(v) bho::make_nvp(BHO_STRINGIZE(v), v)

#endif
