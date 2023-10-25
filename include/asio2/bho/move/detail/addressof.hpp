//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2014-2015. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BHO_MOVE_DETAIL_ADDRESSOF_HPP
#define BHO_MOVE_DETAIL_ADDRESSOF_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/move/detail/workaround.hpp>

namespace bho {
namespace move_detail {

#if defined(BHO_MSVC_FULL_VER) && BHO_MSVC_FULL_VER >= 190024215
#define BHO_MOVE_HAS_BUILTIN_ADDRESSOF
#elif defined(BHO_GCC) && BHO_GCC >= 70000
#define BHO_MOVE_HAS_BUILTIN_ADDRESSOF
#elif defined(__has_builtin)
#if __has_builtin(__builtin_addressof)
#define BHO_MOVE_HAS_BUILTIN_ADDRESSOF
#endif
#endif

#ifdef BHO_MOVE_HAS_BUILTIN_ADDRESSOF

template<class T>
BHO_MOVE_FORCEINLINE T *addressof( T & v ) BHO_NOEXCEPT
{
   return __builtin_addressof(v);
}

#else //BHO_MOVE_HAS_BUILTIN_ADDRESSOF

template <typename T>
BHO_MOVE_FORCEINLINE T* addressof(T& obj)
{
   return static_cast<T*>(
      static_cast<void*>(
         const_cast<char*>(
            &reinterpret_cast<const volatile char&>(obj)
   )));
}

#endif   //BHO_MOVE_HAS_BUILTIN_ADDRESSOF

}  //namespace move_detail {
}  //namespace bho {

#endif   //#ifndef BHO_MOVE_DETAIL_ADDRESSOF_HPP
