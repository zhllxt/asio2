//  (C) Copyright John Maddock 2001 - 2002. 
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  IBM/Aix specific config options:

#define BHO_PLATFORM "IBM Aix"

#define BHO_HAS_UNISTD_H
#define BHO_HAS_NL_TYPES_H
#define BHO_HAS_NANOSLEEP
#define BHO_HAS_CLOCK_GETTIME

// This needs support in "asio2/bho/cstdint.hpp" exactly like FreeBSD.
// This platform has header named <inttypes.h> which includes all
// the things needed.
#define BHO_HAS_STDINT_H

// Threading API's:
#define BHO_HAS_PTHREADS
#define BHO_HAS_PTHREAD_DELAY_NP
#define BHO_HAS_SCHED_YIELD
//#define BHO_HAS_PTHREAD_YIELD

// boilerplate code:
#include <asio2/bho/config/detail/posix_features.hpp>




