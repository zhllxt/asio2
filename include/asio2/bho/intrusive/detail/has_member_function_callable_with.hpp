//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2014-2014. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_CALLABLE_WITH_HPP
#define BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_CALLABLE_WITH_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

//In case no decltype and no variadics, mark that we don't support 0 arg calls due to
//compiler ICE in GCC 3.4/4.0/4.1 and, wrong SFINAE for GCC 4.2/4.3/MSVC10/MSVC11
#if defined(BHO_NO_CXX11_DECLTYPE) && defined(BHO_NO_CXX11_VARIADIC_TEMPLATES)
#  if defined(BHO_GCC) && (BHO_GCC < 40400)
#     define BHO_INTRUSIVE_DETAIL_HAS_MEMBER_FUNCTION_CALLABLE_WITH_0_ARGS_UNSUPPORTED
#  elif defined(BHO_INTEL) && (BHO_INTEL < 1200)
#     define BHO_INTRUSIVE_DETAIL_HAS_MEMBER_FUNCTION_CALLABLE_WITH_0_ARGS_UNSUPPORTED
#  elif defined(BHO_MSVC) && (BHO_MSVC < 1800)
#     define BHO_INTRUSIVE_DETAIL_HAS_MEMBER_FUNCTION_CALLABLE_WITH_0_ARGS_UNSUPPORTED
#  endif
#endif   //#if defined(BHO_NO_CXX11_DECLTYPE) && defined(BHO_NO_CXX11_VARIADIC_TEMPLATES)

#include <cstddef>
#include <asio2/bho/move/utility_core.hpp>
#include <asio2/bho/move/detail/fwd_macros.hpp>

namespace bho_intrusive_hmfcw {

typedef char yes_type;
struct no_type{ char dummy[2]; };

struct dont_care
{
   dont_care(...);
};

#if defined(BHO_NO_CXX11_DECLTYPE)

#if !defined(BHO_NO_CXX11_VARIADIC_TEMPLATES)

template<class T>
struct make_dontcare
{
   typedef dont_care type;
};

#endif

struct private_type
{
   static private_type p;
   private_type const &operator,(int) const;
};

template<typename T>
no_type is_private_type(T const &);
yes_type is_private_type(private_type const &);

#endif   //#if defined(BHO_NO_CXX11_DECLTYPE)

#if defined(BHO_NO_CXX11_VARIADIC_TEMPLATES) || defined(BHO_NO_CXX11_DECLTYPE)

template<typename T> struct remove_cv                    {  typedef T type;   };
template<typename T> struct remove_cv<const T>           {  typedef T type;   };
template<typename T> struct remove_cv<const volatile T>  {  typedef T type;   };
template<typename T> struct remove_cv<volatile T>        {  typedef T type;   };

#endif

}  //namespace bho_intrusive_hmfcw {

#endif  //BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_CALLABLE_WITH_HPP

#ifndef BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME
   #error "You MUST define BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME before including this header!"
#endif

#ifndef BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_MIN
   #error "You MUST define BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_MIN before including this header!"
#endif

#ifndef BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_MAX
   #error "You MUST define BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_MAX before including this header!"
#endif

#if BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_MAX < BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_MIN
   #error "BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_MAX value MUST be greater or equal than BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_MIN!"
#endif

#if BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_MAX == 0
   #define BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_COMMA_IF
#else
   #define BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_COMMA_IF ,
#endif

#ifndef  BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_NS_BEG
   #error "BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_NS_BEG not defined!"
#endif

#ifndef  BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_NS_END
   #error "BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_NS_END not defined!"
#endif

BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_NS_BEG

#if !defined(BHO_NO_CXX11_VARIADIC_TEMPLATES) && !defined(BHO_NO_CXX11_DECLTYPE)
   //With decltype and variadic templaes, things are pretty easy
   template<typename Fun, class ...Args>
   struct BHO_MOVE_CAT(has_member_function_callable_with_,BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME)
   {
      template<class U>
      static decltype(bho::move_detail::declval<U>().
         BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME(::bho::move_detail::declval<Args>()...)
            , bho_intrusive_hmfcw::yes_type()) Test(U* f);
      template<class U>
      static bho_intrusive_hmfcw::no_type Test(...);
      static const bool value = sizeof(Test<Fun>((Fun*)0)) == sizeof(bho_intrusive_hmfcw::yes_type);
   };

#else //defined(BHO_NO_CXX11_VARIADIC_TEMPLATES) || defined(BHO_NO_CXX11_DECLTYPE)

   /////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////
   //
   //    has_member_function_callable_with_impl_XXX
   //    declaration, special case and 0 arg specializaton
   //
   /////////////////////////////////////////////////////////

   template <typename Type>
   class BHO_MOVE_CAT(has_member_function_named_, BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME)
   {
      struct BaseMixin
      {
         void BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME()
         {} //Some compilers require the definition or linker errors happen
      };

      struct Base
         : public bho_intrusive_hmfcw::remove_cv<Type>::type, public BaseMixin
      {  //Declare the unneeded default constructor as some old compilers wrongly require it with is_convertible
         Base(){}
      };
      template <typename T, T t> class Helper{};

      template <typename U>
      static bho_intrusive_hmfcw::no_type  deduce
         (U*, Helper<void (BaseMixin::*)(), &U::BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME>* = 0);
      static bho_intrusive_hmfcw::yes_type deduce(...);

      public:
      static const bool value = sizeof(bho_intrusive_hmfcw::yes_type) == sizeof(deduce((Base*)0));
   };

   #if !defined(BHO_NO_CXX11_VARIADIC_TEMPLATES)
      /////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////
      //
      //    has_member_function_callable_with_impl_XXX for 1 to N arguments
      //
      /////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////

      //defined(BHO_NO_CXX11_DECLTYPE) must be true
      template<class Fun>
      struct FunWrapTmpl : Fun
      {
         using Fun::BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME;
         FunWrapTmpl();
         template<class ...DontCares>
         bho_intrusive_hmfcw::private_type BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME(DontCares...) const;
      };

      template<typename Fun, bool HasFunc, class ...Args>
      struct BHO_MOVE_CAT(has_member_function_callable_with_impl_, BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME);

      //No BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME member specialization
      template<typename Fun, class ...Args>
      struct BHO_MOVE_CAT(has_member_function_callable_with_impl_, BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME)
         <Fun, false, Args...>
      {
         static const bool value = false;
      };

      template<typename Fun, class ...Args>
      struct BHO_MOVE_CAT(has_member_function_callable_with_impl_,BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME)<Fun, true, Args...>
      {
         static bool const value = (sizeof(bho_intrusive_hmfcw::no_type) == sizeof(bho_intrusive_hmfcw::is_private_type
                                             ( (::bho::move_detail::declval
                                                   < FunWrapTmpl<Fun> >().
                                                   BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME(::bho::move_detail::declval<Args>()...), 0) )
                                          )
                                    );
      };

      template<typename Fun, class ...Args>
      struct BHO_MOVE_CAT(has_member_function_callable_with_, BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME)
         : public BHO_MOVE_CAT(has_member_function_callable_with_impl_, BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME)
            <Fun
            , BHO_MOVE_CAT(has_member_function_named_, BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME)<Fun>::value
            , Args...>
      {};
   #else //defined(BHO_NO_CXX11_VARIADIC_TEMPLATES)

      /////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////
      //
      //    has_member_function_callable_with_impl_XXX specializations
      //
      /////////////////////////////////////////////////////////

      template<typename Fun, bool HasFunc BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_COMMA_IF BHO_MOVE_CAT(BHO_MOVE_CLASSDFLT,BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_MAX)>
      struct BHO_MOVE_CAT(has_member_function_callable_with_impl_, BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME);

      //No BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME member specialization
      template<typename Fun BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_COMMA_IF BHO_MOVE_CAT(BHO_MOVE_CLASS,BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_MAX)>
      struct BHO_MOVE_CAT(has_member_function_callable_with_impl_, BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME)
         <Fun, false BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_COMMA_IF BHO_MOVE_CAT(BHO_MOVE_TARG,BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_MAX)>
      {
         static const bool value = false;
      };

      #if BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_MIN == 0
         //0 arg specialization when BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME is present
         #if !defined(BHO_NO_CXX11_DECLTYPE)

            template<typename Fun>
            struct BHO_MOVE_CAT(has_member_function_callable_with_impl_, BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME)<Fun, true>
            {
               template<class U>
               static decltype(bho::move_detail::declval<U>().BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME()
                  , bho_intrusive_hmfcw::yes_type()) Test(U* f);

               template<class U>
               static bho_intrusive_hmfcw::no_type Test(...);
               static const bool value = sizeof(Test<Fun>((Fun*)0)) == sizeof(bho_intrusive_hmfcw::yes_type);
            };

         #else //defined(BHO_NO_CXX11_DECLTYPE)

            #if !defined(BHO_INTRUSIVE_DETAIL_HAS_MEMBER_FUNCTION_CALLABLE_WITH_0_ARGS_UNSUPPORTED)

               template<class F, std::size_t N = sizeof(bho::move_detail::declval<F>().BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME(), 0)>
               struct BHO_MOVE_CAT(zeroarg_checker_, BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME)
               {  bho_intrusive_hmfcw::yes_type dummy[N ? 1 : 2];   };

               template<typename Fun>
               struct BHO_MOVE_CAT(has_member_function_callable_with_impl_, BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME)<Fun, true>
               {
                  template<class U> static BHO_MOVE_CAT(zeroarg_checker_, BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME)<U>
                     Test(BHO_MOVE_CAT(zeroarg_checker_, BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME)<U>*);
                  template<class U> static bho_intrusive_hmfcw::no_type Test(...);
                  static const bool value = sizeof(Test< Fun >(0)) == sizeof(bho_intrusive_hmfcw::yes_type);
               };

            #else //defined(BHO_INTRUSIVE_DETAIL_HAS_MEMBER_FUNCTION_CALLABLE_WITH_0_ARGS_UNSUPPORTED)

               template<typename Fun>
               struct BHO_MOVE_CAT(has_member_function_callable_with_impl_, BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME)<Fun, true>
               {  //Some compilers gives ICE when instantiating the 0 arg version so it is not supported.
                  static const bool value = true;
               };

            #endif//!defined(BHO_INTRUSIVE_DETAIL_HAS_MEMBER_FUNCTION_CALLABLE_WITH_0_ARGS_UNSUPPORTED)
         #endif   //!defined(BHO_NO_CXX11_DECLTYPE)
      #endif   //#if BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_MIN == 0

      #if BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_MAX > 0
         //1 to N arg specialization when BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME is present
         //Declare some unneeded default constructor as some old compilers wrongly require it with is_convertible
         #if defined(BHO_NO_CXX11_DECLTYPE)
            #define BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_ITERATION(N)\
            \
            template<class Fun>\
            struct BHO_MOVE_CAT(FunWrap##N, BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME)\
               : Fun\
            {\
               using Fun::BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME;\
               BHO_MOVE_CAT(FunWrap##N, BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME)();\
               bho_intrusive_hmfcw::private_type BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME\
                  (BHO_MOVE_REPEAT##N(bho_intrusive_hmfcw::dont_care)) const;\
            };\
            \
            template<typename Fun, BHO_MOVE_CLASS##N>\
            struct BHO_MOVE_CAT(has_member_function_callable_with_impl_, BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME)<Fun, true, BHO_MOVE_TARG##N>\
            {\
               static bool const value = (sizeof(bho_intrusive_hmfcw::no_type) == sizeof(bho_intrusive_hmfcw::is_private_type\
                                                   ( (::bho::move_detail::declval\
                                                         < BHO_MOVE_CAT(FunWrap##N, BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME)<Fun> >().\
                                                      BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME(BHO_MOVE_DECLVAL##N), 0) )\
                                                )\
                                          );\
            };\
            //
         #else
            #define BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_ITERATION(N)\
            template<typename Fun, BHO_MOVE_CLASS##N>\
            struct BHO_MOVE_CAT(has_member_function_callable_with_impl_, BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME)\
               <Fun, true, BHO_MOVE_TARG##N>\
            {\
               template<class U>\
               static decltype(bho::move_detail::declval<U>().\
                  BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME(BHO_MOVE_DECLVAL##N)\
                     , bho_intrusive_hmfcw::yes_type()) Test(U* f);\
               template<class U>\
               static bho_intrusive_hmfcw::no_type Test(...);\
               static const bool value = sizeof(Test<Fun>((Fun*)0)) == sizeof(bho_intrusive_hmfcw::yes_type);\
            };\
            //
         #endif
         ////////////////////////////////////
         // Build and invoke BHO_MOVE_ITERATE_NTOM macrofunction, note that N has to be at least 1
         ////////////////////////////////////
         #if BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_MIN == 0
            #define BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_ITERATE_MIN 1
         #else
            #define BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_ITERATE_MIN BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_MIN
         #endif
         BHO_MOVE_CAT
            (BHO_MOVE_CAT(BHO_MOVE_CAT(BHO_MOVE_ITERATE_, BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_ITERATE_MIN), TO)
            ,BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_MAX)
               (BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_ITERATION)
         #undef BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_ITERATION
         #undef BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_ITERATE_MIN
         ////////////////////////////////////
         // End of BHO_MOVE_ITERATE_NTOM
         ////////////////////////////////////
      #endif   //BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_MAX > 0

      /////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////
      //
      //       has_member_function_callable_with_FUNC
      //
      /////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////

      //Otherwise use the preprocessor
      template<typename Fun BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_COMMA_IF BHO_MOVE_CAT(BHO_MOVE_CLASSDFLT,BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_MAX)>
      struct BHO_MOVE_CAT(has_member_function_callable_with_, BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME)
         : public BHO_MOVE_CAT(has_member_function_callable_with_impl_, BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME)
            <Fun
            , BHO_MOVE_CAT(has_member_function_named_, BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME)<Fun>::value
            BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_COMMA_IF BHO_MOVE_CAT(BHO_MOVE_TARG,BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_MAX)>
      {};
   #endif   //defined(BHO_NO_CXX11_VARIADIC_TEMPLATES)
#endif

BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_NS_END

//Undef local macros
#undef BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_COMMA_IF

//Undef user defined macros
#undef BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME
#undef BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_MIN
#undef BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_MAX
#undef BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_NS_BEG
#undef BHO_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_NS_END
