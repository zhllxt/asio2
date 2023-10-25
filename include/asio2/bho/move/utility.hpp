//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2012-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/move for documentation.
//
//////////////////////////////////////////////////////////////////////////////

//! \file
//! This header includes core utilities from <tt><asio2/bho/move/utility_core.hpp></tt> and defines
//! some more advanced utilities such as:

#ifndef BHO_MOVE_MOVE_UTILITY_HPP
#define BHO_MOVE_MOVE_UTILITY_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif
#
#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/move/detail/config_begin.hpp>
#include <asio2/bho/move/detail/workaround.hpp>  //forceinline
#include <asio2/bho/move/utility_core.hpp>
#include <asio2/bho/move/traits.hpp>

#if defined(BHO_NO_CXX11_RVALUE_REFERENCES) && !defined(BHO_MOVE_DOXYGEN_INVOKED)

   namespace bho {

   //////////////////////////////////////////////////////////////////////////////
   //
   //                            move_if_noexcept()
   //
   //////////////////////////////////////////////////////////////////////////////

   template <class T>
   BHO_MOVE_FORCEINLINE typename ::bho::move_detail::enable_if_c
      < enable_move_utility_emulation<T>::value && !has_move_emulation_enabled<T>::value
      , typename ::bho::move_detail::add_const<T>::type &
      >::type
         move_if_noexcept(T& x) BHO_NOEXCEPT
   {
      return x;
   }

   template <class T>
   BHO_MOVE_FORCEINLINE typename ::bho::move_detail::enable_if_c
      < enable_move_utility_emulation<T>::value && has_move_emulation_enabled<T>::value
            && ::bho::move_detail::is_nothrow_move_constructible_or_uncopyable<T>::value, rv<T>&>::type
         move_if_noexcept(T& x) BHO_NOEXCEPT
   {
      return *static_cast<rv<T>* >(::bho::move_detail::addressof(x));
   }

   template <class T>
   BHO_MOVE_FORCEINLINE typename ::bho::move_detail::enable_if_c
      < enable_move_utility_emulation<T>::value && has_move_emulation_enabled<T>::value
            && ::bho::move_detail::is_nothrow_move_constructible_or_uncopyable<T>::value
      , rv<T>&
      >::type
         move_if_noexcept(rv<T>& x) BHO_NOEXCEPT
   {
      return x;
   }

   template <class T>
   BHO_MOVE_FORCEINLINE typename ::bho::move_detail::enable_if_c
      < enable_move_utility_emulation<T>::value && has_move_emulation_enabled<T>::value
            && !::bho::move_detail::is_nothrow_move_constructible_or_uncopyable<T>::value
      , typename ::bho::move_detail::add_const<T>::type &
      >::type
         move_if_noexcept(T& x) BHO_NOEXCEPT
   {
      return x;
   }

   template <class T>
   BHO_MOVE_FORCEINLINE typename ::bho::move_detail::enable_if_c
      < enable_move_utility_emulation<T>::value && has_move_emulation_enabled<T>::value
            && !::bho::move_detail::is_nothrow_move_constructible_or_uncopyable<T>::value
      , typename ::bho::move_detail::add_const<T>::type &
      >::type
         move_if_noexcept(rv<T>& x) BHO_NOEXCEPT
   {
      return x;
   }

   }  //namespace bho

#else    //#if defined(BHO_NO_CXX11_RVALUE_REFERENCES) && !defined(BHO_MOVE_DOXYGEN_INVOKED)

   #if defined(BHO_MOVE_USE_STANDARD_LIBRARY_MOVE)
      #include <utility>

      namespace bho{

      using ::std::move_if_noexcept;

      }  //namespace bho

   #else //!BHO_MOVE_USE_STANDARD_LIBRARY_MOVE

      namespace bho {

      //////////////////////////////////////////////////////////////////////////////
      //
      //                            move_if_noexcept()
      //
      //////////////////////////////////////////////////////////////////////////////
      #if defined(BHO_MOVE_DOXYGEN_INVOKED)
         //! This function provides a way to convert a reference into a rvalue reference
         //! in compilers with rvalue references. For other compilers converts T & into
         //! <i>::bho::rv<T> &</i> so that move emulation is activated. Reference
         //! would be converted to rvalue reference only if input type is nothrow move
         //! constructible or if it has no copy constructor. In all other cases const
         //! reference would be returned
         template <class T>
         rvalue_reference_or_const_lvalue_reference move_if_noexcept(input_reference) noexcept;

      #else //BHO_MOVE_DOXYGEN_INVOKED

         template <class T>
         BHO_MOVE_INTRINSIC_CAST typename ::bho::move_detail::enable_if_c
            < ::bho::move_detail::is_nothrow_move_constructible_or_uncopyable<T>::value, T&&>::type
               move_if_noexcept(T& x) BHO_NOEXCEPT
         {  return ::bho::move(x);   }

         template <class T>
         BHO_MOVE_INTRINSIC_CAST typename ::bho::move_detail::enable_if_c
            < !::bho::move_detail::is_nothrow_move_constructible_or_uncopyable<T>::value, const T&>::type
               move_if_noexcept(T& x) BHO_NOEXCEPT
         {  return x;  }

      #endif //BHO_MOVE_DOXYGEN_INVOKED

      }  //namespace bho {

   #endif   //#if defined(BHO_MOVE_USE_STANDARD_LIBRARY_MOVE)

#endif   //BHO_NO_CXX11_RVALUE_REFERENCES

#include <asio2/bho/move/detail/config_end.hpp>

#endif //#ifndef BHO_MOVE_MOVE_UTILITY_HPP
