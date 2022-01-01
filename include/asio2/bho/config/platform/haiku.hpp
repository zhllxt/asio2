//  (C) Copyright Jessica Hamilton 2014.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  Haiku specific config options:

#define BHO_PLATFORM "Haiku"

#define BHO_HAS_UNISTD_H
#define BHO_HAS_STDINT_H

#ifndef BHO_DISABLE_THREADS
#  define BHO_HAS_THREADS
#endif

#define BHO_NO_CXX11_HDR_TYPE_TRAITS
#define BHO_NO_CXX11_ATOMIC_SMART_PTR
#define BHO_NO_CXX11_STATIC_ASSERT
#define BHO_NO_CXX11_VARIADIC_MACROS

//
// thread API's not auto detected:
//
#define BHO_HAS_SCHED_YIELD
#define BHO_HAS_GETTIMEOFDAY

// boilerplate code:
#include <asio2/bho/config/detail/posix_features.hpp>
