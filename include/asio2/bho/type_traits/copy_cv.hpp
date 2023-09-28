#ifndef BHO_TYPE_TRAITS_COPY_CV_HPP_INCLUDED
#define BHO_TYPE_TRAITS_COPY_CV_HPP_INCLUDED

//
//  Copyright 2015 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt
//

#include <type_traits>

namespace bho
{

template<class T, class U> struct copy_cv
{
private:

    typedef typename std::conditional<std::is_const<U>::value, typename std::add_const<T>::type, T>::type CT;

public:

    typedef typename std::conditional<std::is_volatile<U>::value, typename std::add_volatile<CT>::type, CT>::type type;
};

#if !defined(BHO_NO_CXX11_TEMPLATE_ALIASES)

   template <class T, class U> using copy_cv_t = typename copy_cv<T, U>::type;

#endif

} // namespace bho

#endif // #ifndef BHO_TYPE_TRAITS_COPY_CV_HPP_INCLUDED
