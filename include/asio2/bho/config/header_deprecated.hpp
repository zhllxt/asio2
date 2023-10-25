#ifndef BHO_CONFIG_HEADER_DEPRECATED_HPP_INCLUDED
#define BHO_CONFIG_HEADER_DEPRECATED_HPP_INCLUDED

//  Copyright 2017 Peter Dimov.
//
//  Distributed under the Boost Software License, Version 1.0.
//
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt
//
//  BHO_HEADER_DEPRECATED("<alternative>")
//
//  Expands to the equivalent of
//    BHO_PRAGMA_MESSAGE("This header is deprecated. Use <alternative> instead.")
//
//  Note that this header is C compatible.

#include <asio2/bho/config/pragma_message.hpp>

#if defined(BHO_ALLOW_DEPRECATED_HEADERS) || defined(BHO_ALLOW_DEPRECATED)
# define BHO_HEADER_DEPRECATED(a)
#else
# define BHO_HEADER_DEPRECATED(a) BHO_PRAGMA_MESSAGE("This header is deprecated. Use " a " instead.")
#endif

#endif // BHO_CONFIG_HEADER_DEPRECATED_HPP_INCLUDED
