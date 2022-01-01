//  abi_sufffix header  -------------------------------------------------------//

// (c) Copyright John Maddock 2003
   
// Use, modification and distribution are subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt).

// This header should be #included AFTER code that was preceded by a #include
// <asio2/bho/config/abi_prefix.hpp>.

#ifndef BHO_CONFIG_ABI_PREFIX_HPP
# error Header bho/config/abi_suffix.hpp must only be used after bho/config/abi_prefix.hpp
#else
# undef BHO_CONFIG_ABI_PREFIX_HPP
#endif

// the suffix header occurs after all of our code:
#ifdef BHO_HAS_ABI_HEADERS
#  include BHO_ABI_SUFFIX
#endif

#if defined( BHO_BORLANDC )
#pragma nopushoptwarn
#endif
