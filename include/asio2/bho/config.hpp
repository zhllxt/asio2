//  Boost config.hpp configuration header file  ------------------------------//

//  (C) Copyright John Maddock 2002.
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/config for most recent version.

//  Boost config.hpp policy and rationale documentation has been moved to
//  http://www.boost.org/libs/config
//
//  CAUTION: This file is intended to be completely stable -
//           DO NOT MODIFY THIS FILE!
//

#ifndef BHO_CONFIG_HPP
#define BHO_CONFIG_HPP

// if we don't have a user config, then use the default location:
#if !defined(BHO_USER_CONFIG) && !defined(BHO_NO_USER_CONFIG)
#  define BHO_USER_CONFIG <asio2/bho/config/user.hpp>
#if 0
// For dependency trackers:
#  include <asio2/bho/config/user.hpp>
#endif
#endif
// include it first:
#ifdef BHO_USER_CONFIG
#  include BHO_USER_CONFIG
#endif

// if we don't have a compiler config set, try and find one:
#if !defined(BHO_COMPILER_CONFIG) && !defined(BHO_NO_COMPILER_CONFIG) && !defined(BHO_NO_CONFIG)
#  include <asio2/bho/config/detail/select_compiler_config.hpp>
#endif
// if we have a compiler config, include it now:
#ifdef BHO_COMPILER_CONFIG
#  include BHO_COMPILER_CONFIG
#endif

// if we don't have a std library config set, try and find one:
#if !defined(BHO_STDLIB_CONFIG) && !defined(BHO_NO_STDLIB_CONFIG) && !defined(BHO_NO_CONFIG) && defined(__cplusplus)
#  include <asio2/bho/config/detail/select_stdlib_config.hpp>
#endif
// if we have a std library config, include it now:
#ifdef BHO_STDLIB_CONFIG
#  include BHO_STDLIB_CONFIG
#endif

// if we don't have a platform config set, try and find one:
#if !defined(BHO_PLATFORM_CONFIG) && !defined(BHO_NO_PLATFORM_CONFIG) && !defined(BHO_NO_CONFIG)
#  include <asio2/bho/config/detail/select_platform_config.hpp>
#endif
// if we have a platform config, include it now:
#ifdef BHO_PLATFORM_CONFIG
#  include BHO_PLATFORM_CONFIG
#endif

// get config suffix code:
#include <asio2/bho/config/detail/suffix.hpp>

#ifdef BHO_HAS_PRAGMA_ONCE
#pragma once
#endif

#endif  // BHO_CONFIG_HPP
