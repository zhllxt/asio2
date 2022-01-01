#ifndef BHO_CONFIG_PRAGMA_MESSAGE_HPP_INCLUDED
#define BHO_CONFIG_PRAGMA_MESSAGE_HPP_INCLUDED

//  Copyright 2017 Peter Dimov.
//
//  Distributed under the Boost Software License, Version 1.0.
//
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt
//
//  BHO_PRAGMA_MESSAGE("message")
//
//  Expands to the equivalent of #pragma message("message")
//
//  Note that this header is C compatible.

#include <asio2/bho/config/helper_macros.hpp>

#if defined(BHO_DISABLE_PRAGMA_MESSAGE)
# define BHO_PRAGMA_MESSAGE(x)
#elif defined(__INTEL_COMPILER)
# define BHO_PRAGMA_MESSAGE(x) __pragma(message(__FILE__ "(" BHO_STRINGIZE(__LINE__) "): note: " x))
#elif defined(__GNUC__)
# define BHO_PRAGMA_MESSAGE(x) _Pragma(BHO_STRINGIZE(message(x)))
#elif defined(_MSC_VER)
# define BHO_PRAGMA_MESSAGE(x) __pragma(message(__FILE__ "(" BHO_STRINGIZE(__LINE__) "): note: " x))
#else
# define BHO_PRAGMA_MESSAGE(x)
#endif

#endif // BHO_CONFIG_PRAGMA_MESSAGE_HPP_INCLUDED
