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
//! This header implements macros to define movable classes and
//! move-aware functions

#ifndef BHO_MOVE_CORE_HPP
#define BHO_MOVE_CORE_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif
#
#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/move/detail/config_begin.hpp>
#include <asio2/bho/move/detail/workaround.hpp>

// @cond

//bho_move_no_copy_constructor_or_assign typedef
//used to detect noncopyable types for other Boost libraries.
#if defined(BHO_NO_CXX11_DELETED_FUNCTIONS) || defined(BHO_NO_CXX11_RVALUE_REFERENCES)
   #define BHO_MOVE_IMPL_NO_COPY_CTOR_OR_ASSIGN(TYPE) \
      private:\
      TYPE(TYPE &);\
      TYPE& operator=(TYPE &);\
      public:\
      typedef int bho_move_no_copy_constructor_or_assign; \
      private:\
   //
#else
   #define BHO_MOVE_IMPL_NO_COPY_CTOR_OR_ASSIGN(TYPE) \
      public:\
      TYPE(TYPE const &) = delete;\
      TYPE& operator=(TYPE const &) = delete;\
      public:\
      typedef int bho_move_no_copy_constructor_or_assign; \
      private:\
   //
#endif   //BHO_NO_CXX11_DELETED_FUNCTIONS

// @endcond

#if defined(BHO_NO_CXX11_RVALUE_REFERENCES) && !defined(BHO_MOVE_DOXYGEN_INVOKED)

   #include <asio2/bho/move/detail/type_traits.hpp>

   #define BHO_MOVE_TO_RV_CAST(RV_TYPE, ARG) reinterpret_cast<RV_TYPE>(ARG)
   #define BHO_MOVE_TO_LV_CAST(LV_TYPE, ARG) static_cast<LV_TYPE>(ARG)

   //Move emulation rv breaks standard aliasing rules so add workarounds for some compilers
   #if defined(BHO_GCC) && (BHO_GCC >= 40400) && (BHO_GCC < 40500)
   #define BHO_RV_ATTRIBUTE_MAY_ALIAS BHO_MAY_ALIAS
   #else
   #define BHO_RV_ATTRIBUTE_MAY_ALIAS 
   #endif

   namespace bho {

   //////////////////////////////////////////////////////////////////////////////
   //
   //                            struct rv
   //
   //////////////////////////////////////////////////////////////////////////////
   template <class T>
   class BHO_RV_ATTRIBUTE_MAY_ALIAS rv
      : public ::bho::move_detail::if_c
         < ::bho::move_detail::is_class<T>::value
         , T
         , ::bho::move_detail::nat
         >::type
   {
      rv();
      ~rv() throw();
      rv(rv const&);
      void operator=(rv const&);
   };


   //////////////////////////////////////////////////////////////////////////////
   //
   //                            is_rv
   //
   //////////////////////////////////////////////////////////////////////////////

   namespace move_detail {

   template <class T>
   struct is_rv
        //Derive from integral constant because some Boost code assummes it has
        //a "type" internal typedef
      : integral_constant<bool, ::bho::move_detail::is_rv_impl<T>::value >
   {};

   template <class T>
   struct is_not_rv
   {
      static const bool value = !is_rv<T>::value;
   };

   }  //namespace move_detail {

   //////////////////////////////////////////////////////////////////////////////
   //
   //                               has_move_emulation_enabled
   //
   //////////////////////////////////////////////////////////////////////////////
   template<class T>
   struct has_move_emulation_enabled
      : ::bho::move_detail::has_move_emulation_enabled_impl<T>
   {};

   template<class T>
   struct has_move_emulation_disabled
   {
      static const bool value = !::bho::move_detail::has_move_emulation_enabled_impl<T>::value;
   };

   }  //namespace bho {

   #define BHO_RV_REF(TYPE)\
      ::bho::rv< TYPE >& \
   //

   #define BHO_RV_REF_2_TEMPL_ARGS(TYPE, ARG1, ARG2)\
      ::bho::rv< TYPE<ARG1, ARG2> >& \
   //

   #define BHO_RV_REF_3_TEMPL_ARGS(TYPE, ARG1, ARG2, ARG3)\
      ::bho::rv< TYPE<ARG1, ARG2, ARG3> >& \
   //

   #define BHO_RV_REF_BEG\
      ::bho::rv<   \
   //

   #define BHO_RV_REF_END\
      >& \
   //

   #define BHO_RV_REF_BEG_IF_CXX11 \
      \
   //

   #define BHO_RV_REF_END_IF_CXX11 \
      \
   //

   #define BHO_FWD_REF(TYPE)\
      const TYPE & \
   //

   #define BHO_COPY_ASSIGN_REF(TYPE)\
      const ::bho::rv< TYPE >& \
   //

   #define BHO_COPY_ASSIGN_REF_BEG \
      const ::bho::rv<  \
   //

   #define BHO_COPY_ASSIGN_REF_END \
      >& \
   //

   #define BHO_COPY_ASSIGN_REF_2_TEMPL_ARGS(TYPE, ARG1, ARG2)\
      const ::bho::rv< TYPE<ARG1, ARG2> >& \
   //

   #define BHO_COPY_ASSIGN_REF_3_TEMPL_ARGS(TYPE, ARG1, ARG2, ARG3)\
      const ::bho::rv< TYPE<ARG1, ARG2, ARG3> >& \
   //

   #define BHO_CATCH_CONST_RLVALUE(TYPE)\
      const ::bho::rv< TYPE >& \
   //

   namespace bho {
   namespace move_detail {

   template <class Ret, class T>
   BHO_MOVE_FORCEINLINE typename ::bho::move_detail::enable_if_c
      <  ::bho::move_detail::is_lvalue_reference<Ret>::value ||
        !::bho::has_move_emulation_enabled<T>::value
      , T&>::type
         move_return(T& x) BHO_NOEXCEPT
   {
      return x;
   }

   template <class Ret, class T>
   BHO_MOVE_FORCEINLINE typename ::bho::move_detail::enable_if_c
      < !::bho::move_detail::is_lvalue_reference<Ret>::value &&
         ::bho::has_move_emulation_enabled<T>::value
      , ::bho::rv<T>&>::type
         move_return(T& x) BHO_NOEXCEPT
   {
      return *BHO_MOVE_TO_RV_CAST(::bho::rv<T>*, ::bho::move_detail::addressof(x));
   }

   template <class Ret, class T>
   BHO_MOVE_FORCEINLINE typename ::bho::move_detail::enable_if_c
      < !::bho::move_detail::is_lvalue_reference<Ret>::value &&
         ::bho::has_move_emulation_enabled<T>::value
      , ::bho::rv<T>&>::type
         move_return(::bho::rv<T>& x) BHO_NOEXCEPT
   {
      return x;
   }

   template <class T>
   BHO_MOVE_FORCEINLINE T& unrv(::bho::rv<T> &rv) BHO_NOEXCEPT
   {  return BHO_MOVE_TO_LV_CAST(T&, rv);   }

   }  //namespace move_detail {
   }  //namespace bho {

   #define BHO_MOVE_RET(RET_TYPE, REF)\
      bho::move_detail::move_return< RET_TYPE >(REF)
   //

   #define BHO_MOVE_BASE(BASE_TYPE, ARG) \
      ::bho::move((BASE_TYPE&)(ARG))
   //

   #define BHO_MOVE_TO_LV(ARG) \
      ::bho::move_detail::unrv(ARG)
   //


   //////////////////////////////////////////////////////////////////////////////
   //
   //                         BHO_MOVABLE_BUT_NOT_COPYABLE
   //
   //////////////////////////////////////////////////////////////////////////////
   #define BHO_MOVABLE_BUT_NOT_COPYABLE(TYPE)\
      BHO_MOVE_IMPL_NO_COPY_CTOR_OR_ASSIGN(TYPE)\
      public:\
      BHO_MOVE_FORCEINLINE operator ::bho::rv<TYPE>&() \
      {  return *BHO_MOVE_TO_RV_CAST(::bho::rv<TYPE>*, this);  }\
      BHO_MOVE_FORCEINLINE operator const ::bho::rv<TYPE>&() const \
      {  return *BHO_MOVE_TO_RV_CAST(const ::bho::rv<TYPE>*, this);  }\
      private:\
   //

   //////////////////////////////////////////////////////////////////////////////
   //
   //                         BHO_COPYABLE_AND_MOVABLE
   //
   //////////////////////////////////////////////////////////////////////////////

   #define BHO_COPYABLE_AND_MOVABLE(TYPE)\
      public:\
      BHO_MOVE_FORCEINLINE TYPE& operator=(TYPE &t)\
      {  this->operator=(const_cast<const TYPE&>(t)); return *this;}\
      public:\
      BHO_MOVE_FORCEINLINE operator ::bho::rv<TYPE>&() \
      {  return *BHO_MOVE_TO_RV_CAST(::bho::rv<TYPE>*, this);  }\
      BHO_MOVE_FORCEINLINE operator const ::bho::rv<TYPE>&() const \
      {  return *BHO_MOVE_TO_RV_CAST(const ::bho::rv<TYPE>*, this);  }\
      private:\
   //

   #define BHO_COPYABLE_AND_MOVABLE_ALT(TYPE)\
      public:\
      BHO_MOVE_FORCEINLINE operator ::bho::rv<TYPE>&() \
      {  return *BHO_MOVE_TO_RV_CAST(::bho::rv<TYPE>*, this);  }\
      BHO_MOVE_FORCEINLINE operator const ::bho::rv<TYPE>&() const \
      {  return *BHO_MOVE_TO_RV_CAST(const ::bho::rv<TYPE>*, this);  }\
      private:\
   //

   namespace bho{
   namespace move_detail{

   template< class T>
   struct forward_type
   { typedef const T &type; };

   template< class T>
   struct forward_type< bho::rv<T> >
   { typedef T type; };

   }}

#else    //BHO_NO_CXX11_RVALUE_REFERENCES

   //! This macro marks a type as movable but not copyable, disabling copy construction
   //! and assignment. The user will need to write a move constructor/assignment as explained
   //! in the documentation to fully write a movable but not copyable class.
   #define BHO_MOVABLE_BUT_NOT_COPYABLE(TYPE)\
      BHO_MOVE_IMPL_NO_COPY_CTOR_OR_ASSIGN(TYPE)\
      public:\
      typedef int bho_move_emulation_t;\
      private:\
   //

   //! This macro marks a type as copyable and movable.
   //! The user will need to write a move constructor/assignment and a copy assignment
   //! as explained in the documentation to fully write a copyable and movable class.
   #define BHO_COPYABLE_AND_MOVABLE(TYPE)\
   //

   #if !defined(BHO_MOVE_DOXYGEN_INVOKED)
   #define BHO_COPYABLE_AND_MOVABLE_ALT(TYPE)\
   //
   #endif   //#if !defined(BHO_MOVE_DOXYGEN_INVOKED)

   namespace bho {

   //!This trait yields to a compile-time true boolean if T was marked as
   //!BHO_MOVABLE_BUT_NOT_COPYABLE or BHO_COPYABLE_AND_MOVABLE and
   //!rvalue references are not available on the platform. False otherwise.
   template<class T>
   struct has_move_emulation_enabled
   {
      static const bool value = false;
   };

   template<class T>
   struct has_move_emulation_disabled
   {
      static const bool value = true;
   };

   }  //namespace bho{

   //!This macro is used to achieve portable syntax in move
   //!constructors and assignments for classes marked as
   //!BHO_COPYABLE_AND_MOVABLE or BHO_MOVABLE_BUT_NOT_COPYABLE
   #define BHO_RV_REF(TYPE)\
      TYPE && \
   //

   //!This macro is used to achieve portable syntax in move
   //!constructors and assignments for template classes marked as
   //!BHO_COPYABLE_AND_MOVABLE or BHO_MOVABLE_BUT_NOT_COPYABLE.
   //!As macros have problems with comma-separated template arguments,
   //!the template argument must be preceded with BHO_RV_REF_BEG
   //!and ended with BHO_RV_REF_END
   #define BHO_RV_REF_BEG\
         \
   //

   //!This macro is used to achieve portable syntax in move
   //!constructors and assignments for template classes marked as
   //!BHO_COPYABLE_AND_MOVABLE or BHO_MOVABLE_BUT_NOT_COPYABLE.
   //!As macros have problems with comma-separated template arguments,
   //!the template argument must be preceded with BHO_RV_REF_BEG
   //!and ended with BHO_RV_REF_END
   #define BHO_RV_REF_END\
      && \
   //

   //!This macro expands to BHO_RV_REF_BEG if BHO_NO_CXX11_RVALUE_REFERENCES
   //!is not defined, empty otherwise
   #define BHO_RV_REF_BEG_IF_CXX11 \
      BHO_RV_REF_BEG \
   //

   //!This macro expands to BHO_RV_REF_END if BHO_NO_CXX11_RVALUE_REFERENCES
   //!is not defined, empty otherwise
   #define BHO_RV_REF_END_IF_CXX11 \
      BHO_RV_REF_END \
   //

   //!This macro is used to achieve portable syntax in copy
   //!assignment for classes marked as BHO_COPYABLE_AND_MOVABLE.
   #define BHO_COPY_ASSIGN_REF(TYPE)\
      const TYPE & \
   //

   //! This macro is used to implement portable perfect forwarding
   //! as explained in the documentation.
   #define BHO_FWD_REF(TYPE)\
      TYPE && \
   //

   #if !defined(BHO_MOVE_DOXYGEN_INVOKED)

   #define BHO_RV_REF_2_TEMPL_ARGS(TYPE, ARG1, ARG2)\
      TYPE<ARG1, ARG2> && \
   //

   #define BHO_RV_REF_3_TEMPL_ARGS(TYPE, ARG1, ARG2, ARG3)\
      TYPE<ARG1, ARG2, ARG3> && \
   //

   #define BHO_COPY_ASSIGN_REF_BEG \
      const \
   //

   #define BHO_COPY_ASSIGN_REF_END \
      & \
   //

   #define BHO_COPY_ASSIGN_REF_2_TEMPL_ARGS(TYPE, ARG1, ARG2)\
      const TYPE<ARG1, ARG2> & \
   //

   #define BHO_COPY_ASSIGN_REF_3_TEMPL_ARGS(TYPE, ARG1, ARG2, ARG3)\
      const TYPE<ARG1, ARG2, ARG3>& \
   //

   #define BHO_CATCH_CONST_RLVALUE(TYPE)\
      const TYPE & \
   //

   #endif   //#if !defined(BHO_MOVE_DOXYGEN_INVOKED)

   #if !defined(BHO_MOVE_MSVC_AUTO_MOVE_RETURN_BUG) || defined(BHO_MOVE_DOXYGEN_INVOKED)

      //!This macro is used to achieve portable move return semantics.
      //!The C++11 Standard allows implicit move returns when the object to be returned
      //!is designated by a lvalue and:
      //!   - The criteria for elision of a copy operation are met OR
      //!   - The criteria would be met save for the fact that the source object is a function parameter
      //!
      //!For C++11 conforming compilers this macros only yields to REF:
      //! <code>return BHO_MOVE_RET(RET_TYPE, REF);</code> -> <code>return REF;</code>
      //!
      //!For compilers without rvalue references
      //!this macro does an explicit move if the move emulation is activated
      //!and the return type (RET_TYPE) is not a reference.
      //!
      //!For non-conforming compilers with rvalue references like Visual 2010 & 2012,
      //!an explicit move is performed if RET_TYPE is not a reference.
      //!
      //! <b>Caution</b>: When using this macro in non-conforming or C++03
      //!compilers, a move will be performed even if the C++11 standard does not allow it
      //!(e.g. returning a static variable). The user is responsible for using this macro
      //!only to return local objects that met C++11 criteria.
      #define BHO_MOVE_RET(RET_TYPE, REF)\
         REF
      //

   #else //!defined(BHO_MOVE_MSVC_AUTO_MOVE_RETURN_BUG) || defined(BHO_MOVE_DOXYGEN_INVOKED)

      #include <asio2/bho/move/detail/meta_utils.hpp>

      namespace bho {
      namespace move_detail {

      template <class Ret, class T>
      BHO_MOVE_FORCEINLINE typename ::bho::move_detail::enable_if_c
         <  ::bho::move_detail::is_lvalue_reference<Ret>::value
         , T&>::type
            move_return(T& x) BHO_NOEXCEPT
      {
         return x;
      }

      template <class Ret, class T>
      BHO_MOVE_FORCEINLINE typename ::bho::move_detail::enable_if_c
         < !::bho::move_detail::is_lvalue_reference<Ret>::value
         , Ret && >::type
            move_return(T&& t) BHO_NOEXCEPT
      {
         return static_cast< Ret&& >(t);
      }

      }  //namespace move_detail {
      }  //namespace bho {

      #define BHO_MOVE_RET(RET_TYPE, REF)\
         bho::move_detail::move_return< RET_TYPE >(REF)
      //

   #endif   //!defined(BHO_MOVE_MSVC_AUTO_MOVE_RETURN_BUG) || defined(BHO_MOVE_DOXYGEN_INVOKED)

   //!This macro is used to achieve portable optimal move constructors.
   //!
   //!When implementing the move constructor, in C++03 compilers the moved-from argument must be
   //!cast to the base type before calling `::bho::move()` due to rvalue reference limitations.
   //!
   //!In C++11 compilers the cast from a rvalue reference of a derived type to a rvalue reference of
   //!a base type is implicit.
   #define BHO_MOVE_BASE(BASE_TYPE, ARG) \
      ::bho::move((BASE_TYPE&)(ARG))
   //

   //!This macro is used to achieve portable optimal move constructors.
   //!
   //!In C++03 mode, when accessing a member of type through a rvalue (implemented as a `rv<T> &` type, where rv<T> derives
   //!from T) triggers a potential UB as the program never creates objects of type rv<T>. This macro casts back `rv<T>` to
   //!`T&` so that access to member types are done through the original type.
   //! 
   //!In C++11 compilers the cast from a rvalue reference of a derived type to a rvalue reference of
   //!a base type is implicit, so it's a no-op.
   #define BHO_MOVE_TO_LV(ARG) ARG
   //

   namespace bho {
   namespace move_detail {

   template< class T> struct forward_type { typedef T type; };

   }}

#endif   //BHO_NO_CXX11_RVALUE_REFERENCES

#include <asio2/bho/move/detail/config_end.hpp>

#endif //#ifndef BHO_MOVE_CORE_HPP
