//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2010-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/move for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BHO_MOVE_MOVE_HELPERS_HPP
#define BHO_MOVE_MOVE_HELPERS_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif
#
#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/move/core.hpp>
#include <asio2/bho/move/utility_core.hpp>
#include <asio2/bho/move/detail/type_traits.hpp>

#if defined(BHO_NO_CXX11_RVALUE_REFERENCES)

#define BHO_MOVE_CATCH_CONST(U)  \
   typename ::bho::move_detail::if_< ::bho::move_detail::is_class<U>, BHO_CATCH_CONST_RLVALUE(U), const U &>::type
#define BHO_MOVE_CATCH_RVALUE(U)\
   typename ::bho::move_detail::if_< ::bho::move_detail::is_class<U>, BHO_RV_REF(U), ::bho::move_detail::nat>::type
#define BHO_MOVE_CATCH_FWD(U) BHO_FWD_REF(U)
#else
#define BHO_MOVE_CATCH_CONST(U)  const U &
#define BHO_MOVE_CATCH_RVALUE(U) U &&
#define BHO_MOVE_CATCH_FWD(U)    U &&
#endif

////////////////////////////////////////
//
// BHO_MOVE_CONVERSION_AWARE_CATCH
//
////////////////////////////////////////

#ifdef BHO_NO_CXX11_RVALUE_REFERENCES

   template<class RETURN_VALUE, class BHO_MOVE_TEMPL_PARAM, class TYPE>
   struct bho_move_conversion_aware_catch_1
      : public ::bho::move_detail::enable_if_and
                        < RETURN_VALUE
                        , ::bho::move_detail::is_same<TYPE, BHO_MOVE_TEMPL_PARAM>
                        , ::bho::move_detail::is_class<TYPE>
                        , ::bho::has_move_emulation_disabled<BHO_MOVE_TEMPL_PARAM>
                        >
   {};

   template<class RETURN_VALUE, class BHO_MOVE_TEMPL_PARAM, class TYPE>
   struct bho_move_conversion_aware_catch_2
      : public ::bho::move_detail::disable_if_or
                        < RETURN_VALUE
                        , ::bho::move_detail::is_same<TYPE, BHO_MOVE_TEMPL_PARAM> 
                        , ::bho::move_detail::is_rv_impl<BHO_MOVE_TEMPL_PARAM>
                        , ::bho::move_detail::and_
                                    < ::bho::move_detail::is_rv_impl<BHO_MOVE_TEMPL_PARAM>
                                    , ::bho::move_detail::is_class<BHO_MOVE_TEMPL_PARAM>
                                    >
                        >
   {};

   #define BHO_MOVE_CONVERSION_AWARE_CATCH_COMMON(PUB_FUNCTION, TYPE, RETURN_VALUE, FWD_FUNCTION)\
      BHO_MOVE_FORCEINLINE RETURN_VALUE PUB_FUNCTION(BHO_MOVE_CATCH_CONST(TYPE) x)\
      {  return FWD_FUNCTION(static_cast<const TYPE&>(x)); }\
      \
      BHO_MOVE_FORCEINLINE RETURN_VALUE PUB_FUNCTION(BHO_MOVE_CATCH_RVALUE(TYPE) x) \
      {  return FWD_FUNCTION(::bho::move(x));  }\
      \
      BHO_MOVE_FORCEINLINE RETURN_VALUE PUB_FUNCTION(TYPE &x)\
      {  return FWD_FUNCTION(const_cast<const TYPE &>(x)); }\
   //
   #if defined(BHO_MOVE_HELPERS_RETURN_SFINAE_BROKEN)
      #define BHO_MOVE_CONVERSION_AWARE_CATCH(PUB_FUNCTION, TYPE, RETURN_VALUE, FWD_FUNCTION)\
         BHO_MOVE_CONVERSION_AWARE_CATCH_COMMON(PUB_FUNCTION, TYPE, RETURN_VALUE, FWD_FUNCTION)\
         \
         template<class BHO_MOVE_TEMPL_PARAM>\
         BHO_MOVE_FORCEINLINE RETURN_VALUE PUB_FUNCTION(const BHO_MOVE_TEMPL_PARAM &u,\
            typename bho_move_conversion_aware_catch_1< ::bho::move_detail::nat, BHO_MOVE_TEMPL_PARAM, TYPE>::type* = 0)\
         { return FWD_FUNCTION(u); }\
         \
         template<class BHO_MOVE_TEMPL_PARAM>\
         BHO_MOVE_FORCEINLINE RETURN_VALUE PUB_FUNCTION(const BHO_MOVE_TEMPL_PARAM &u,\
            typename bho_move_conversion_aware_catch_2< ::bho::move_detail::nat, BHO_MOVE_TEMPL_PARAM, TYPE>::type* = 0)\
         {\
            TYPE t((u));\
            return FWD_FUNCTION(::bho::move(t));\
         }\
      //
   #else
      #define BHO_MOVE_CONVERSION_AWARE_CATCH(PUB_FUNCTION, TYPE, RETURN_VALUE, FWD_FUNCTION)\
         BHO_MOVE_CONVERSION_AWARE_CATCH_COMMON(PUB_FUNCTION, TYPE, RETURN_VALUE, FWD_FUNCTION)\
         \
         template<class BHO_MOVE_TEMPL_PARAM>\
         BHO_MOVE_FORCEINLINE typename bho_move_conversion_aware_catch_1<RETURN_VALUE, BHO_MOVE_TEMPL_PARAM, TYPE>::type\
            PUB_FUNCTION(const BHO_MOVE_TEMPL_PARAM &u)\
         { return FWD_FUNCTION(u); }\
         \
         template<class BHO_MOVE_TEMPL_PARAM>\
         BHO_MOVE_FORCEINLINE typename bho_move_conversion_aware_catch_2<RETURN_VALUE, BHO_MOVE_TEMPL_PARAM, TYPE>::type\
            PUB_FUNCTION(const BHO_MOVE_TEMPL_PARAM &u)\
         {\
            TYPE t((u));\
            return FWD_FUNCTION(::bho::move(t));\
         }\
      //
   #endif
#elif (defined(_MSC_VER) && (_MSC_VER == 1600))

   #define BHO_MOVE_CONVERSION_AWARE_CATCH(PUB_FUNCTION, TYPE, RETURN_VALUE, FWD_FUNCTION)\
      BHO_MOVE_FORCEINLINE RETURN_VALUE PUB_FUNCTION(BHO_MOVE_CATCH_CONST(TYPE) x)\
      {  return FWD_FUNCTION(static_cast<const TYPE&>(x)); }\
      \
      BHO_MOVE_FORCEINLINE RETURN_VALUE PUB_FUNCTION(BHO_MOVE_CATCH_RVALUE(TYPE) x) \
      {  return FWD_FUNCTION(::bho::move(x));  }\
      \
      template<class BHO_MOVE_TEMPL_PARAM>\
      BHO_MOVE_FORCEINLINE typename ::bho::move_detail::enable_if_c\
                        < !::bho::move_detail::is_same<TYPE, BHO_MOVE_TEMPL_PARAM>::value\
                        , RETURN_VALUE >::type\
      PUB_FUNCTION(const BHO_MOVE_TEMPL_PARAM &u)\
      {\
         TYPE t((u));\
         return FWD_FUNCTION(::bho::move(t));\
      }\
   //

#else    //BHO_NO_CXX11_RVALUE_REFERENCES

   #define BHO_MOVE_CONVERSION_AWARE_CATCH(PUB_FUNCTION, TYPE, RETURN_VALUE, FWD_FUNCTION)\
      BHO_MOVE_FORCEINLINE RETURN_VALUE PUB_FUNCTION(BHO_MOVE_CATCH_CONST(TYPE) x)\
      {  return FWD_FUNCTION(x); }\
      \
      BHO_MOVE_FORCEINLINE RETURN_VALUE PUB_FUNCTION(BHO_MOVE_CATCH_RVALUE(TYPE) x) \
      {  return FWD_FUNCTION(::bho::move(x));  }\
   //

#endif   //BHO_NO_CXX11_RVALUE_REFERENCES

////////////////////////////////////////
//
// BHO_MOVE_CONVERSION_AWARE_CATCH_1ARG
//
////////////////////////////////////////

#ifdef BHO_NO_CXX11_RVALUE_REFERENCES

   template<class RETURN_VALUE, class BHO_MOVE_TEMPL_PARAM, class UNLESS_CONVERTIBLE_TO, class TYPE>
   struct bho_move_conversion_aware_catch_1arg_1
      : public ::bho::move_detail::enable_if_and
                        < RETURN_VALUE
                        , ::bho::move_detail::not_< ::bho::move_detail::is_same_or_convertible<BHO_MOVE_TEMPL_PARAM, UNLESS_CONVERTIBLE_TO> >
                        , ::bho::move_detail::is_same<TYPE, BHO_MOVE_TEMPL_PARAM>
                        , ::bho::has_move_emulation_disabled<BHO_MOVE_TEMPL_PARAM>
                        >
   {};

   template<class RETURN_VALUE, class BHO_MOVE_TEMPL_PARAM, class UNLESS_CONVERTIBLE_TO, class TYPE>
   struct bho_move_conversion_aware_catch_1arg_2
      : public ::bho::move_detail::disable_if_or
                        < RETURN_VALUE
                        , ::bho::move_detail::is_same_or_convertible< BHO_MOVE_TEMPL_PARAM, UNLESS_CONVERTIBLE_TO>
                        , ::bho::move_detail::is_rv_impl<BHO_MOVE_TEMPL_PARAM>
                        , ::bho::move_detail::is_same<TYPE, BHO_MOVE_TEMPL_PARAM>
                        >
   {};

   #define BHO_MOVE_CONVERSION_AWARE_CATCH_1ARG_COMMON(PUB_FUNCTION, TYPE, RETURN_VALUE, FWD_FUNCTION, ARG1, UNLESS_CONVERTIBLE_TO)\
      BHO_MOVE_FORCEINLINE RETURN_VALUE PUB_FUNCTION(ARG1 arg1, BHO_MOVE_CATCH_CONST(TYPE) x)\
      {  return FWD_FUNCTION(arg1, static_cast<const TYPE&>(x)); }\
      \
      BHO_MOVE_FORCEINLINE RETURN_VALUE PUB_FUNCTION(ARG1 arg1, BHO_MOVE_CATCH_RVALUE(TYPE) x) \
      {  return FWD_FUNCTION(arg1, ::bho::move(x));  }\
      \
      BHO_MOVE_FORCEINLINE RETURN_VALUE PUB_FUNCTION(ARG1 arg1, TYPE &x)\
      {  return FWD_FUNCTION(arg1, const_cast<const TYPE &>(x)); }\
   //
   #if defined(BHO_MOVE_HELPERS_RETURN_SFINAE_BROKEN)
      #define BHO_MOVE_CONVERSION_AWARE_CATCH_1ARG(PUB_FUNCTION, TYPE, RETURN_VALUE, FWD_FUNCTION, ARG1, UNLESS_CONVERTIBLE_TO)\
         BHO_MOVE_CONVERSION_AWARE_CATCH_1ARG_COMMON(PUB_FUNCTION, TYPE, RETURN_VALUE, FWD_FUNCTION, ARG1, UNLESS_CONVERTIBLE_TO)\
         \
         template<class BHO_MOVE_TEMPL_PARAM>\
         BHO_MOVE_FORCEINLINE RETURN_VALUE PUB_FUNCTION(ARG1 arg1, const BHO_MOVE_TEMPL_PARAM &u,\
            typename bho_move_conversion_aware_catch_1arg_1<void, BHO_MOVE_TEMPL_PARAM, UNLESS_CONVERTIBLE_TO, TYPE>::type* = 0)\
         { return FWD_FUNCTION(arg1, u); }\
         \
         template<class BHO_MOVE_TEMPL_PARAM>\
         BHO_MOVE_FORCEINLINE RETURN_VALUE PUB_FUNCTION(ARG1 arg1, const BHO_MOVE_TEMPL_PARAM &u,\
            typename bho_move_conversion_aware_catch_1arg_2<void, BHO_MOVE_TEMPL_PARAM, UNLESS_CONVERTIBLE_TO, TYPE>::type* = 0)\
         {\
            TYPE t((u));\
            return FWD_FUNCTION(arg1, ::bho::move(t));\
         }\
      //
   #else
      #define BHO_MOVE_CONVERSION_AWARE_CATCH_1ARG(PUB_FUNCTION, TYPE, RETURN_VALUE, FWD_FUNCTION, ARG1, UNLESS_CONVERTIBLE_TO)\
         BHO_MOVE_CONVERSION_AWARE_CATCH_1ARG_COMMON(PUB_FUNCTION, TYPE, RETURN_VALUE, FWD_FUNCTION, ARG1, UNLESS_CONVERTIBLE_TO)\
         \
         template<class BHO_MOVE_TEMPL_PARAM>\
         BHO_MOVE_FORCEINLINE typename bho_move_conversion_aware_catch_1arg_1<RETURN_VALUE, BHO_MOVE_TEMPL_PARAM, UNLESS_CONVERTIBLE_TO, TYPE>::type\
            PUB_FUNCTION(ARG1 arg1, const BHO_MOVE_TEMPL_PARAM &u)\
         { return FWD_FUNCTION(arg1, u); }\
         \
         template<class BHO_MOVE_TEMPL_PARAM>\
         BHO_MOVE_FORCEINLINE typename bho_move_conversion_aware_catch_1arg_2<RETURN_VALUE, BHO_MOVE_TEMPL_PARAM, UNLESS_CONVERTIBLE_TO, TYPE>::type\
            PUB_FUNCTION(ARG1 arg1, const BHO_MOVE_TEMPL_PARAM &u)\
         {\
            TYPE t((u));\
            return FWD_FUNCTION(arg1, ::bho::move(t));\
         }\
      //
   #endif

#elif (defined(_MSC_VER) && (_MSC_VER == 1600))

   #define BHO_MOVE_CONVERSION_AWARE_CATCH_1ARG(PUB_FUNCTION, TYPE, RETURN_VALUE, FWD_FUNCTION, ARG1, UNLESS_CONVERTIBLE_TO)\
      BHO_MOVE_FORCEINLINE RETURN_VALUE PUB_FUNCTION(ARG1 arg1, BHO_MOVE_CATCH_CONST(TYPE) x)\
      {  return FWD_FUNCTION(arg1, static_cast<const TYPE&>(x)); }\
      \
      BHO_MOVE_FORCEINLINE RETURN_VALUE PUB_FUNCTION(ARG1 arg1, BHO_MOVE_CATCH_RVALUE(TYPE) x) \
      {  return FWD_FUNCTION(arg1, ::bho::move(x));  }\
      \
      template<class BHO_MOVE_TEMPL_PARAM>\
      BHO_MOVE_FORCEINLINE typename ::bho::move_detail::disable_if_or\
                        < RETURN_VALUE \
                        , ::bho::move_detail::is_same<TYPE, BHO_MOVE_TEMPL_PARAM> \
                        , ::bho::move_detail::is_same_or_convertible<BHO_MOVE_TEMPL_PARAM, UNLESS_CONVERTIBLE_TO> \
                        >::type\
      PUB_FUNCTION(ARG1 arg1, const BHO_MOVE_TEMPL_PARAM &u)\
      {\
         TYPE t((u));\
         return FWD_FUNCTION(arg1, ::bho::move(t));\
      }\
   //

#else

   #define BHO_MOVE_CONVERSION_AWARE_CATCH_1ARG(PUB_FUNCTION, TYPE, RETURN_VALUE, FWD_FUNCTION, ARG1, UNLESS_CONVERTIBLE_TO)\
      BHO_MOVE_FORCEINLINE RETURN_VALUE PUB_FUNCTION(ARG1 arg1, BHO_MOVE_CATCH_CONST(TYPE) x)\
      {  return FWD_FUNCTION(arg1, static_cast<const TYPE&>(x)); }\
      \
      BHO_MOVE_FORCEINLINE RETURN_VALUE PUB_FUNCTION(ARG1 arg1, BHO_MOVE_CATCH_RVALUE(TYPE) x) \
      {  return FWD_FUNCTION(arg1, ::bho::move(x));  }\
   //

#endif

#endif //#ifndef BHO_MOVE_MOVE_HELPERS_HPP
