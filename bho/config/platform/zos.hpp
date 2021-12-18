//  Copyright (c) 2017 Dynatrace
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt

//  See http://www.boost.org for most recent version.

//  Platform setup for IBM z/OS.

#define BHO_PLATFORM "IBM z/OS"

#include <features.h> // For __UU, __C99, __TR1, ...

#if defined(__UU)
#  define BHO_HAS_GETTIMEOFDAY
#endif

#if defined(_OPEN_THREADS) || defined(__SUSV3_THR)
#  define BHO_HAS_PTHREADS
#  define BHO_HAS_PTHREAD_MUTEXATTR_SETTYPE
#  define BHO_HAS_THREADS
#endif

#if defined(__SUSV3) || defined(__SUSV3_THR)
#  define BHO_HAS_SCHED_YIELD
#endif

#define BHO_HAS_SIGACTION
#define BHO_HAS_UNISTD_H
#define BHO_HAS_DIRENT_H
#define BHO_HAS_NL_TYPES_H
