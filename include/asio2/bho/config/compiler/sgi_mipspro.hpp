//  (C) Copyright John Maddock 2001 - 2002. 
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  SGI C++ compiler setup:

#define BHO_COMPILER "SGI Irix compiler version " BHO_STRINGIZE(_COMPILER_VERSION)

#include <asio2/bho/config/compiler/common_edg.hpp>

//
// Threading support:
// Turn this on unconditionally here, it will get turned off again later
// if no threading API is detected.
//
#define BHO_HAS_THREADS
#define BHO_NO_TWO_PHASE_NAME_LOOKUP

#undef BHO_NO_SWPRINTF
#undef BHO_DEDUCED_TYPENAME

//
// version check:
// probably nothing to do here?


