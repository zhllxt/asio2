//  (C) Copyright John Maddock 2001 - 2003. 
//  (C) Copyright Jens Maurer 2003. 
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  sun specific config options:

#define BHO_PLATFORM "Sun Solaris"

#define BHO_HAS_GETTIMEOFDAY

// boilerplate code:
#define BHO_HAS_UNISTD_H
#include <asio2/bho/config/detail/posix_features.hpp>

//
// pthreads don't actually work with gcc unless _PTHREADS is defined:
//
#if defined(__GNUC__) && defined(_POSIX_THREADS) && !defined(_PTHREADS)
# undef BHO_HAS_PTHREADS
#endif

#define BHO_HAS_STDINT_H 
#define BHO_HAS_PTHREAD_MUTEXATTR_SETTYPE 
#define BHO_HAS_LOG1P 
#define BHO_HAS_EXPM1


