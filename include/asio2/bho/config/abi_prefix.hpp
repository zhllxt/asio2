//  abi_prefix header  -------------------------------------------------------//

// (c) Copyright John Maddock 2003
   
// Use, modification and distribution are subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt).

#ifndef BHO_CONFIG_ABI_PREFIX_HPP
# define BHO_CONFIG_ABI_PREFIX_HPP
#else
# error double inclusion of header bho/config/abi_prefix.hpp is an error
#endif

#include <asio2/bho/config.hpp>

// this must occur after all other includes and before any code appears:
#ifdef BHO_HAS_ABI_HEADERS
#  include BHO_ABI_PREFIX
#endif

#if defined( BHO_BORLANDC )
#pragma nopushoptwarn
#endif

