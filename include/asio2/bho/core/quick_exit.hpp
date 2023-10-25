#ifndef BHO_CORE_QUICK_EXIT_HPP_INCLUDED
#define BHO_CORE_QUICK_EXIT_HPP_INCLUDED

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

//  bho/core/quick_exit.hpp
//
//  Copyright 2018 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <asio2/bho/config.hpp>
#include <stdlib.h>

#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)

extern "C" _CRTIMP __cdecl __MINGW_NOTHROW  void _exit (int) __MINGW_ATTRIB_NORETURN;

#endif

#if defined(__CYGWIN__) && __cplusplus < 201103L

extern "C" _Noreturn void quick_exit(int);

#endif

namespace bho
{

BHO_NORETURN inline void quick_exit( int code ) BHO_NOEXCEPT
{
#if defined(_MSC_VER) && _MSC_VER < 1900

    ::_exit( code );

#elif defined(__MINGW32__)

    ::_exit( code );

#elif defined(__APPLE__)

    ::_Exit( code );

#else

    ::quick_exit( code );

#endif
}

} // namespace bho

#endif  // #ifndef BHO_CORE_QUICK_EXIT_HPP_INCLUDED
