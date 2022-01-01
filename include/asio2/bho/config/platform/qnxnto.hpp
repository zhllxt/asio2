//  (C) Copyright Jim Douglas 2005. 
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  QNX specific config options:

#define BHO_PLATFORM "QNX"

#define BHO_HAS_UNISTD_H
#include <asio2/bho/config/detail/posix_features.hpp>

// QNX claims XOpen version 5 compatibility, but doesn't have an nl_types.h
// or log1p and expm1:
#undef  BHO_HAS_NL_TYPES_H
#undef  BHO_HAS_LOG1P
#undef  BHO_HAS_EXPM1

#define BHO_HAS_PTHREADS
#define BHO_HAS_PTHREAD_MUTEXATTR_SETTYPE

#define BHO_HAS_GETTIMEOFDAY
#define BHO_HAS_CLOCK_GETTIME
#define BHO_HAS_NANOSLEEP





