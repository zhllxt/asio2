//  (C) Copyright John Maddock 2001. 
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  BeOS specific config options:

#define BHO_PLATFORM "BeOS"

#define BHO_NO_CWCHAR
#define BHO_NO_CWCTYPE
#define BHO_HAS_UNISTD_H

#define BHO_HAS_BETHREADS

#ifndef BHO_DISABLE_THREADS
#  define BHO_HAS_THREADS
#endif

// boilerplate code:
#include <asio2/bho/config/detail/posix_features.hpp>
 


