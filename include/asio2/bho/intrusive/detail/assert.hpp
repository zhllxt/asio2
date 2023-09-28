/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga  2006-2013
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef BHO_INTRUSIVE_DETAIL_ASSERT_HPP
#define BHO_INTRUSIVE_DETAIL_ASSERT_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#pragma once
#endif

#if !defined(BHO_INTRUSIVE_INVARIANT_ASSERT)
   #include <asio2/bho/assert.hpp>
   #define BHO_INTRUSIVE_INVARIANT_ASSERT BHO_ASSERT
#elif defined(BHO_INTRUSIVE_INVARIANT_ASSERT_INCLUDE)
   #include BHO_INTRUSIVE_INVARIANT_ASSERT_INCLUDE
#endif

#if !defined(BHO_INTRUSIVE_SAFE_HOOK_DEFAULT_ASSERT)
   #include <asio2/bho/assert.hpp>
   #define BHO_INTRUSIVE_SAFE_HOOK_DEFAULT_ASSERT BHO_ASSERT
#elif defined(BHO_INTRUSIVE_SAFE_HOOK_DEFAULT_ASSERT_INCLUDE)
   #include BHO_INTRUSIVE_SAFE_HOOK_DEFAULT_ASSERT_INCLUDE
#endif

#if !defined(BHO_INTRUSIVE_SAFE_HOOK_DESTRUCTOR_ASSERT)
   #include <asio2/bho/assert.hpp>
   #define BHO_INTRUSIVE_SAFE_HOOK_DESTRUCTOR_ASSERT BHO_ASSERT
#elif defined(BHO_INTRUSIVE_SAFE_HOOK_DESTRUCTOR_ASSERT_INCLUDE)
   #include BHO_INTRUSIVE_SAFE_HOOK_DESTRUCTOR_ASSERT_INCLUDE
#endif

#endif //BHO_INTRUSIVE_DETAIL_ASSERT_HPP
