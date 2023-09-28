//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2005-2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/interprocess for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BHO_INTRUSIVE_DETAIL_WORKAROUND_HPP
#define BHO_INTRUSIVE_DETAIL_WORKAROUND_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#ifndef BHO_CONFIG_HPP
#include <asio2/bho/config.hpp>
#endif

// MSVC-12 ICEs when variadic templates are enabled.
#if    !defined(BHO_NO_CXX11_VARIADIC_TEMPLATES) && (!defined(BHO_MSVC) || BHO_MSVC >= 1900)
   #define BHO_INTRUSIVE_VARIADIC_TEMPLATES
#endif

#if    !defined(BHO_NO_CXX11_RVALUE_REFERENCES) && !defined(BHO_NO_CXX11_VARIADIC_TEMPLATES)
   #define BHO_INTRUSIVE_PERFECT_FORWARDING
#endif

//Macros for documentation purposes. For code, expands to the argument
#define BHO_INTRUSIVE_IMPDEF(TYPE) TYPE
#define BHO_INTRUSIVE_SEEDOC(TYPE) TYPE
#define BHO_INTRUSIVE_DOC1ST(TYPE1, TYPE2) TYPE2
#define BHO_INTRUSIVE_I ,
#define BHO_INTRUSIVE_DOCIGN(T1) T1

//#define BHO_INTRUSIVE_DISABLE_FORCEINLINE

#if defined(BHO_INTRUSIVE_DISABLE_FORCEINLINE)
   #define BHO_INTRUSIVE_FORCEINLINE inline
#elif defined(BHO_INTRUSIVE_FORCEINLINE_IS_BHO_FORCELINE)
   #define BHO_INTRUSIVE_FORCEINLINE BHO_FORCEINLINE
#elif defined(BHO_MSVC) && defined(_DEBUG)
   //"__forceinline" and MSVC seems to have some bugs in debug mode
   #define BHO_INTRUSIVE_FORCEINLINE inline
#elif defined(__GNUC__) && ((__GNUC__ < 4) || (__GNUC__ == 4 && (__GNUC_MINOR__ < 5)))
   //Older GCCs have problems with forceinline
   #define BHO_INTRUSIVE_FORCEINLINE inline
#else
   #define BHO_INTRUSIVE_FORCEINLINE BHO_FORCEINLINE
#endif

#endif   //#ifndef BHO_INTRUSIVE_DETAIL_WORKAROUND_HPP
