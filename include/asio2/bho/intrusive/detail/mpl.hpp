/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga        2006-2014
// (C) Copyright Microsoft Corporation  2014
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef BHO_INTRUSIVE_DETAIL_MPL_HPP
#define BHO_INTRUSIVE_DETAIL_MPL_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/intrusive/detail/config_begin.hpp>
#include <asio2/bho/move/detail/type_traits.hpp>
#include <cstddef>

namespace bho {
namespace intrusive {
namespace detail {
   
using bho::move_detail::is_same;
using bho::move_detail::add_const;
using bho::move_detail::remove_const;
using bho::move_detail::remove_cv;
using bho::move_detail::remove_reference;
using bho::move_detail::add_reference;
using bho::move_detail::remove_pointer;
using bho::move_detail::add_pointer;
using bho::move_detail::true_type;
using bho::move_detail::false_type;
using bho::move_detail::voider;
using bho::move_detail::enable_if_c;
using bho::move_detail::enable_if;
using bho::move_detail::disable_if_c;
using bho::move_detail::disable_if;
using bho::move_detail::is_convertible;
using bho::move_detail::if_c;
using bho::move_detail::if_;
using bho::move_detail::is_const;
using bho::move_detail::identity;
using bho::move_detail::alignment_of;
using bho::move_detail::is_empty;
using bho::move_detail::addressof;
using bho::move_detail::integral_constant;
using bho::move_detail::enable_if_convertible;
using bho::move_detail::disable_if_convertible;
using bho::move_detail::bool_;
using bho::move_detail::true_;
using bho::move_detail::false_;
using bho::move_detail::yes_type;
using bho::move_detail::no_type;
using bho::move_detail::apply;
using bho::move_detail::eval_if_c;
using bho::move_detail::eval_if;
using bho::move_detail::unvoid_ref;
using bho::move_detail::add_const_if_c;

template<std::size_t S>
struct ls_zeros
{
   static const std::size_t value = (S & std::size_t(1)) ? 0 : (1 + ls_zeros<(S>>1u)>::value);
};

template<>
struct ls_zeros<0>
{
   static const std::size_t value = 0;
};

template<>
struct ls_zeros<1>
{
   static const std::size_t value = 0;
};

// Infrastructure for providing a default type for T::TNAME if absent.
#define BHO_INTRUSIVE_INSTANTIATE_DEFAULT_TYPE_TMPLT(TNAME)     \
   template <typename T>                                          \
   struct bho_intrusive_has_type_ ## TNAME                      \
   {                                                              \
      template <typename X>                                       \
      static char test(int, typename X::TNAME*);                  \
                                                                  \
      template <typename X>                                       \
      static int test(...);                                       \
                                                                  \
      static const bool value = (1 == sizeof(test<T>(0, 0)));     \
   };                                                             \
                                                                  \
   template <typename T, typename DefaultType>                    \
   struct bho_intrusive_default_type_ ## TNAME                  \
   {                                                              \
      struct DefaultWrap { typedef DefaultType TNAME; };          \
                                                                  \
      typedef typename                                            \
         ::bho::intrusive::detail::if_c                         \
            < bho_intrusive_has_type_ ## TNAME<T>::value        \
            , T, DefaultWrap>::type::TNAME type;                  \
   };                                                             \
   //

#define BHO_INTRUSIVE_OBTAIN_TYPE_WITH_DEFAULT(INSTANTIATION_NS_PREFIX, T, TNAME, TIMPL)   \
      typename INSTANTIATION_NS_PREFIX                                                       \
         bho_intrusive_default_type_ ## TNAME< T, TIMPL >::type                            \
//

#define BHO_INTRUSIVE_HAS_TYPE(INSTANTIATION_NS_PREFIX, T, TNAME)  \
      INSTANTIATION_NS_PREFIX                                        \
         bho_intrusive_has_type_ ## TNAME< T >::value              \
//

#define BHO_INTRUSIVE_INSTANTIATE_EVAL_DEFAULT_TYPE_TMPLT(TNAME)\
   template <typename T, typename DefaultType>                    \
   struct bho_intrusive_eval_default_type_ ## TNAME             \
   {                                                              \
      template <typename X>                                       \
      static char test(int, typename X::TNAME*);                  \
                                                                  \
      template <typename X>                                       \
      static int test(...);                                       \
                                                                  \
      struct DefaultWrap                                          \
      { typedef typename DefaultType::type TNAME; };              \
                                                                  \
      static const bool value = (1 == sizeof(test<T>(0, 0)));     \
                                                                  \
      typedef typename                                            \
         ::bho::intrusive::detail::eval_if_c                    \
            < value                                               \
            , ::bho::intrusive::detail::identity<T>             \
            , ::bho::intrusive::detail::identity<DefaultWrap>   \
            >::type::TNAME type;                                  \
   };                                                             \
//

#define BHO_INTRUSIVE_OBTAIN_TYPE_WITH_EVAL_DEFAULT(INSTANTIATION_NS_PREFIX, T, TNAME, TIMPL) \
      typename INSTANTIATION_NS_PREFIX                                                          \
         bho_intrusive_eval_default_type_ ## TNAME< T, TIMPL >::type                          \
//

#define BHO_INTRUSIVE_INTERNAL_STATIC_BOOL_IS_TRUE(TRAITS_PREFIX, TYPEDEF_TO_FIND) \
template <class T>\
struct TRAITS_PREFIX##_bool\
{\
   template<bool Add>\
   struct two_or_three {yes_type _[2u + (unsigned)Add];};\
   template <class U> static yes_type test(...);\
   template <class U> static two_or_three<U::TYPEDEF_TO_FIND> test (int);\
   static const std::size_t value = sizeof(test<T>(0));\
};\
\
template <class T>\
struct TRAITS_PREFIX##_bool_is_true\
{\
   static const bool value = TRAITS_PREFIX##_bool<T>::value > sizeof(yes_type)*2;\
};\
//

#define BHO_INTRUSIVE_HAS_STATIC_MEMBER_FUNC_SIGNATURE(TRAITS_NAME, FUNC_NAME) \
  template <typename U, typename Signature> \
  class TRAITS_NAME \
  { \
  private: \
  template<Signature> struct helper;\
  template<typename T> \
  static ::bho::intrusive::detail::yes_type test(helper<&T::FUNC_NAME>*); \
  template<typename T> static ::bho::intrusive::detail::no_type test(...); \
  public: \
  static const bool value = sizeof(test<U>(0)) == sizeof(::bho::intrusive::detail::yes_type); \
  }; \
//

#define BHO_INTRUSIVE_HAS_MEMBER_FUNC_CALLED(TRAITS_NAME, FUNC_NAME) \
template <typename Type> \
struct TRAITS_NAME \
{ \
   struct BaseMixin \
   { \
      void FUNC_NAME(); \
   }; \
   struct Base : public Type, public BaseMixin { Base(); }; \
   template <typename T, T t> class Helper{}; \
   template <typename U> \
   static ::bho::intrusive::detail::no_type  test(U*, Helper<void (BaseMixin::*)(), &U::FUNC_NAME>* = 0); \
   static ::bho::intrusive::detail::yes_type test(...); \
   static const bool value = sizeof(::bho::intrusive::detail::yes_type) == sizeof(test((Base*)(0))); \
};\
//

#define BHO_INTRUSIVE_HAS_MEMBER_FUNC_CALLED_IGNORE_SIGNATURE(TRAITS_NAME, FUNC_NAME) \
BHO_INTRUSIVE_HAS_MEMBER_FUNC_CALLED(TRAITS_NAME##_ignore_signature, FUNC_NAME) \
\
template <typename Type, class> \
struct TRAITS_NAME \
   : public TRAITS_NAME##_ignore_signature<Type> \
{};\
//

} //namespace detail
} //namespace intrusive
} //namespace bho

#include <asio2/bho/intrusive/detail/config_end.hpp>

#endif //BHO_INTRUSIVE_DETAIL_MPL_HPP
