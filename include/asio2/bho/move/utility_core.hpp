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
//! This header defines core utilities to ease the development
//! of move-aware functions. This header minimizes dependencies
//! from other libraries.

#ifndef BHO_MOVE_MOVE_UTILITY_CORE_HPP
#define BHO_MOVE_MOVE_UTILITY_CORE_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif
#
#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/move/detail/config_begin.hpp>
#include <asio2/bho/move/detail/workaround.hpp>  //forceinline
#include <asio2/bho/move/core.hpp>
#include <asio2/bho/move/detail/meta_utils.hpp>

#if defined(BHO_NO_CXX11_RVALUE_REFERENCES) && !defined(BHO_MOVE_DOXYGEN_INVOKED)

   namespace bho {

   template<class T>
   struct enable_move_utility_emulation
   {
      static const bool value = true;
   };
    
   //////////////////////////////////////////////////////////////////////////////
   //
   //                            move()
   //
   //////////////////////////////////////////////////////////////////////////////

   template <class T>
   BHO_MOVE_FORCEINLINE typename ::bho::move_detail::enable_if_and
      < T &
      , enable_move_utility_emulation<T>
      , has_move_emulation_disabled<T>
      >::type
         move(T& x) BHO_NOEXCEPT
   {
      return x;
   }

   template <class T>
   BHO_MOVE_FORCEINLINE typename ::bho::move_detail::enable_if_and
      < rv<T>&
      , enable_move_utility_emulation<T>
      , has_move_emulation_enabled<T>
      >::type
         move(T& x) BHO_NOEXCEPT
   {
      return *BHO_MOVE_TO_RV_CAST(::bho::rv<T>*, ::bho::move_detail::addressof(x) );
   }

   template <class T>
   BHO_MOVE_FORCEINLINE typename ::bho::move_detail::enable_if_and
      < rv<T>&
      , enable_move_utility_emulation<T>
      , has_move_emulation_enabled<T>
      >::type
         move(rv<T>& x) BHO_NOEXCEPT
   {
      return x;
   }

   //////////////////////////////////////////////////////////////////////////////
   //
   //                            forward()
   //
   //////////////////////////////////////////////////////////////////////////////

   template <class T>
   BHO_MOVE_FORCEINLINE typename ::bho::move_detail::enable_if_and
      < T &
      , enable_move_utility_emulation<T>
      , ::bho::move_detail::is_rv<T>
      >::type
         forward(const typename ::bho::move_detail::identity<T>::type &x) BHO_NOEXCEPT
   {
      return const_cast<T&>(x);
   }

   template <class T>
   BHO_MOVE_FORCEINLINE typename ::bho::move_detail::enable_if_and
      < const T &
      , enable_move_utility_emulation<T>
      , ::bho::move_detail::is_not_rv<T>
      >::type
         forward(const typename ::bho::move_detail::identity<T>::type &x) BHO_NOEXCEPT
   {
      return x;
   }

   //////////////////////////////////////////////////////////////////////////////
   //
   //                        move_if_not_lvalue_reference()
   //
   //////////////////////////////////////////////////////////////////////////////

   template <class T>
   BHO_MOVE_FORCEINLINE typename ::bho::move_detail::enable_if_and
      < T &
      , enable_move_utility_emulation<T>
      , ::bho::move_detail::is_rv<T>
      >::type
         move_if_not_lvalue_reference(const typename ::bho::move_detail::identity<T>::type &x) BHO_NOEXCEPT
   {
      return const_cast<T&>(x);
   }

   template <class T>
   BHO_MOVE_FORCEINLINE typename ::bho::move_detail::enable_if_and
      < typename ::bho::move_detail::add_lvalue_reference<T>::type
      , enable_move_utility_emulation<T>
      , ::bho::move_detail::is_not_rv<T>
      , ::bho::move_detail::or_
         < ::bho::move_detail::is_lvalue_reference<T>
         , has_move_emulation_disabled<T>
         >
      >::type
         move_if_not_lvalue_reference(typename ::bho::move_detail::remove_reference<T>::type &x) BHO_NOEXCEPT
   {
      return x;
   }

   template <class T>
   BHO_MOVE_FORCEINLINE typename ::bho::move_detail::enable_if_and
      < rv<T>&
      , enable_move_utility_emulation<T>
      , ::bho::move_detail::is_not_rv<T>
      , ::bho::move_detail::and_
         < ::bho::move_detail::not_< ::bho::move_detail::is_lvalue_reference<T> >
         , has_move_emulation_enabled<T>
         >
      >::type
         move_if_not_lvalue_reference(typename ::bho::move_detail::remove_reference<T>::type &x) BHO_NOEXCEPT
   {
      return move(x);
   }

   }  //namespace bho

#else    //#if defined(BHO_NO_CXX11_RVALUE_REFERENCES) && !defined(BHO_MOVE_DOXYGEN_INVOKED)

   #if defined(BHO_MOVE_USE_STANDARD_LIBRARY_MOVE)
      #include <utility>

      namespace bho{

      using ::std::move;
      using ::std::forward;

      }  //namespace bho

   #else //!BHO_MOVE_USE_STANDARD_LIBRARY_MOVE

      namespace bho {

      //! This trait's internal boolean `value` is false in compilers with rvalue references
      //! and true in compilers without rvalue references.
      //!
      //! A user can specialize this trait for a type T to false to SFINAE out `move` and `forward`
      //! so that the user can define a different move emulation for that type in namespace bho
      //! (e.g. another Boost library for its types) and avoid any overload ambiguity.
      template<class T>
      struct enable_move_utility_emulation
      {
         static const bool value = false;
      };

      //////////////////////////////////////////////////////////////////////////////
      //
      //                                  move
      //
      //////////////////////////////////////////////////////////////////////////////

      #if defined(BHO_MOVE_DOXYGEN_INVOKED)
         //! This function provides a way to convert a reference into a rvalue reference
         //! in compilers with rvalue references. For other compilers if `T` is BHO.Move
         //! enabled type then it converts `T&` into <tt>::bho::rv<T> &</tt> so that
         //! move emulation is activated, else it returns `T &`.
         template <class T>
         rvalue_reference move(input_reference) noexcept;

      #elif defined(BHO_MOVE_OLD_RVALUE_REF_BINDING_RULES)

         //Old move approach, lvalues could bind to rvalue references
         template <class T>
         BHO_MOVE_FORCEINLINE typename ::bho::move_detail::remove_reference<T>::type && move(T&& t) BHO_NOEXCEPT
         {  return t;   }

      #else //BHO_MOVE_OLD_RVALUE_REF_BINDING_RULES

         template <class T>
         BHO_MOVE_INTRINSIC_CAST
         typename ::bho::move_detail::remove_reference<T>::type && move(T&& t) BHO_NOEXCEPT
         { return static_cast<typename ::bho::move_detail::remove_reference<T>::type &&>(t); }

      #endif   //BHO_MOVE_OLD_RVALUE_REF_BINDING_RULES

      //////////////////////////////////////////////////////////////////////////////
      //
      //                                  forward
      //
      //////////////////////////////////////////////////////////////////////////////


      #if defined(BHO_MOVE_DOXYGEN_INVOKED)
         //! This function provides limited form of forwarding that is usually enough for
         //! in-place construction and avoids the exponential overloading for
         //! achieve the limited forwarding in C++03.
         //!
         //! For compilers with rvalue references this function provides perfect forwarding.
         //!
         //! Otherwise:
         //! * If input_reference binds to const ::bho::rv<T> & then it output_reference is
         //!   ::bho::rv<T> &
         //!
         //! * Else, output_reference is equal to input_reference.
         template <class T> output_reference forward(input_reference) noexcept;
      #elif defined(BHO_MOVE_OLD_RVALUE_REF_BINDING_RULES)

         //Old move approach, lvalues could bind to rvalue references

         template <class T>
         BHO_MOVE_FORCEINLINE T&& forward(typename ::bho::move_detail::identity<T>::type&& t) BHO_NOEXCEPT
         {  return t;   }

      #else //Old move

         template <class T>
         BHO_MOVE_INTRINSIC_CAST
         T&& forward(typename ::bho::move_detail::remove_reference<T>::type& t) BHO_NOEXCEPT
         {  return static_cast<T&&>(t);   }

         template <class T>
         BHO_MOVE_INTRINSIC_CAST
         T&& forward(typename ::bho::move_detail::remove_reference<T>::type&& t) BHO_NOEXCEPT
         {
            //"bho::forward<T> error: 'T' is a lvalue reference, can't forward as rvalue.";
            BHO_MOVE_STATIC_ASSERT(!bho::move_detail::is_lvalue_reference<T>::value);
            return static_cast<T&&>(t);
         }

      #endif   //BHO_MOVE_DOXYGEN_INVOKED

      }  //namespace bho {

   #endif   //BHO_MOVE_USE_STANDARD_LIBRARY_MOVE

   //////////////////////////////////////////////////////////////////////////////
   //
   //                         move_if_not_lvalue_reference
   //
   //////////////////////////////////////////////////////////////////////////////

   namespace bho {

   #if defined(BHO_MOVE_DOXYGEN_INVOKED)
      //! <b>Effects</b>: Calls `bho::move` if `input_reference` is not a lvalue reference.
      //!   Otherwise returns the reference
      template <class T> output_reference move_if_not_lvalue_reference(input_reference) noexcept;
   #elif defined(BHO_MOVE_OLD_RVALUE_REF_BINDING_RULES)

      //Old move approach, lvalues could bind to rvalue references

      template <class T>
      BHO_MOVE_FORCEINLINE T&& move_if_not_lvalue_reference(typename ::bho::move_detail::identity<T>::type&& t) BHO_NOEXCEPT
      {  return t;   }

   #else //Old move

      template <class T>
      BHO_MOVE_FORCEINLINE T&& move_if_not_lvalue_reference(typename ::bho::move_detail::remove_reference<T>::type& t) BHO_NOEXCEPT
      {  return static_cast<T&&>(t);   }

      template <class T>
      BHO_MOVE_FORCEINLINE T&& move_if_not_lvalue_reference(typename ::bho::move_detail::remove_reference<T>::type&& t) BHO_NOEXCEPT
      {
         //"bho::forward<T> error: 'T' is a lvalue reference, can't forward as rvalue.";
         BHO_MOVE_STATIC_ASSERT(!bho::move_detail::is_lvalue_reference<T>::value);
         return static_cast<T&&>(t);
      }

   #endif   //BHO_MOVE_DOXYGEN_INVOKED

   }  //namespace bho {

#endif   //BHO_NO_CXX11_RVALUE_REFERENCES

#if !defined(BHO_MOVE_DOXYGEN_INVOKED)

namespace bho{
namespace move_detail{

template <typename T>
typename bho::move_detail::add_rvalue_reference<T>::type declval();

}  //namespace move_detail{
}  //namespace bho{

#endif   //#if !defined(BHO_MOVE_DOXYGEN_INVOKED)


#include <asio2/bho/move/detail/config_end.hpp>

#endif //#ifndef BHO_MOVE_MOVE_UTILITY_CORE_HPP
