//////////////////////////////////////////////////////////////////////////////
// (C) Copyright John Maddock 2000.
// (C) Copyright Ion Gaztanaga 2005-2015.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/move for documentation.
//
// The alignment and Type traits implementation comes from
// John Maddock's TypeTraits library.
//
// Some other tricks come from Howard Hinnant's papers and StackOverflow replies
//////////////////////////////////////////////////////////////////////////////
#ifndef BHO_MOVE_DETAIL_TYPE_TRAITS_HPP
#define BHO_MOVE_DETAIL_TYPE_TRAITS_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif
#
#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/move/detail/config_begin.hpp>
#include <asio2/bho/move/detail/workaround.hpp>

// move/detail
#include <asio2/bho/move/detail/meta_utils.hpp>
// other
#include <cassert>
// std
#include <cstddef>

//Use of BHO.TypeTraits leads to long preprocessed source code due to
//MPL dependencies. We'll use intrinsics directly and make or own
//simplified version of TypeTraits.
//If someday BHO.TypeTraits dependencies are minimized, we should
//revisit this file redirecting code to BHO.TypeTraits traits.

//These traits don't care about volatile, reference or other checks
//made by BHO.TypeTraits because no volatile or reference types
//can be hold in BHO.Containers. This helps to avoid any BHO.TypeTraits
//dependency.

// Helper macros for builtin compiler support.
// If your compiler has builtin support for any of the following
// traits concepts, then redefine the appropriate macros to pick
// up on the compiler support:
//
// (these should largely ignore cv-qualifiers)
// BHO_MOVE_IS_POD(T) should evaluate to true if T is a POD type
// BHO_MOVE_HAS_TRIVIAL_CONSTRUCTOR(T) should evaluate to true if "T x;" has no effect
// BHO_MOVE_HAS_TRIVIAL_COPY(T) should evaluate to true if T(t) <==> memcpy
// (Note: this trait does not guarantee T is copy constructible, the copy constructor could be deleted but still be trivial)
// BHO_MOVE_HAS_TRIVIAL_MOVE_CONSTRUCTOR(T) should evaluate to true if T(bho::move(t)) <==> memcpy
// BHO_MOVE_HAS_TRIVIAL_ASSIGN(T) should evaluate to true if t = u <==> memcpy
// (Note: this trait does not guarantee T is assignable , the copy assignmen could be deleted but still be trivial)
// BHO_MOVE_HAS_TRIVIAL_MOVE_ASSIGN(T) should evaluate to true if t = bho::move(u) <==> memcpy
// BHO_MOVE_HAS_TRIVIAL_DESTRUCTOR(T) should evaluate to true if ~T() has no effect
// BHO_MOVE_HAS_NOTHROW_CONSTRUCTOR(T) should evaluate to true if "T x;" can not throw
// BHO_MOVE_HAS_NOTHROW_COPY(T) should evaluate to true if T(t) can not throw
// BHO_MOVE_HAS_NOTHROW_ASSIGN(T) should evaluate to true if t = u can not throw
// BHO_MOVE_IS_ENUM(T) should evaluate to true it t is a union type.
// BHO_MOVE_HAS_NOTHROW_MOVE_CONSTRUCTOR(T) should evaluate to true if T has a non-throwing move constructor.
// BHO_MOVE_HAS_NOTHROW_MOVE_ASSIGN(T) should evaluate to true if T has a non-throwing move assignment operator.
//
// The following can also be defined: when detected our implementation is greatly simplified.
//
// BHO_ALIGNMENT_OF(T) should evaluate to the alignment requirements of type T.

#if defined(__MSL_CPP__) && (__MSL_CPP__ >= 0x8000)
    // Metrowerks compiler is acquiring intrinsic type traits support
    // post version 8.  We hook into the published interface to pick up
    // user defined specializations as well as compiler intrinsics as
    // and when they become available:
#   include <msl_utility>
#   define BHO_MOVE_IS_UNION(T) BHO_STD_EXTENSION_NAMESPACE::is_union<T>::value
#   define BHO_MOVE_IS_POD(T) BHO_STD_EXTENSION_NAMESPACE::is_POD<T>::value
#   define BHO_MOVE_HAS_TRIVIAL_CONSTRUCTOR(T) BHO_STD_EXTENSION_NAMESPACE::has_trivial_default_ctor<T>::value
#   define BHO_MOVE_HAS_TRIVIAL_COPY(T) BHO_STD_EXTENSION_NAMESPACE::has_trivial_copy_ctor<T>::value
#   define BHO_MOVE_HAS_TRIVIAL_ASSIGN(T) BHO_STD_EXTENSION_NAMESPACE::has_trivial_assignment<T>::value
#   define BHO_MOVE_HAS_TRIVIAL_DESTRUCTOR(T) BHO_STD_EXTENSION_NAMESPACE::has_trivial_dtor<T>::value
#endif

#if (defined(BHO_MSVC) && defined(BHO_MSVC_FULL_VER) && (BHO_MSVC_FULL_VER >=140050215))\
         || (defined(BHO_INTEL) && defined(_MSC_VER) && (_MSC_VER >= 1500))
#   define BHO_MOVE_IS_UNION(T) __is_union(T)
#   define BHO_MOVE_IS_POD(T)                    (__is_pod(T) && __has_trivial_constructor(T))
#   define BHO_MOVE_IS_EMPTY(T)                  __is_empty(T)
#   define BHO_MOVE_HAS_TRIVIAL_CONSTRUCTOR(T)   __has_trivial_constructor(T)
#   define BHO_MOVE_HAS_TRIVIAL_COPY(T)          (__has_trivial_copy(T)|| ::bho::move_detail::is_pod<T>::value)
#   define BHO_MOVE_HAS_TRIVIAL_ASSIGN(T)        (__has_trivial_assign(T) || ::bho::move_detail::is_pod<T>::value)
#   define BHO_MOVE_HAS_TRIVIAL_DESTRUCTOR(T)    (__has_trivial_destructor(T) || ::bho::move_detail::is_pod<T>::value)
#   define BHO_MOVE_HAS_NOTHROW_CONSTRUCTOR(T)   (__has_nothrow_constructor(T) || ::bho::move_detail::is_trivially_default_constructible<T>::value)
#   define BHO_MOVE_HAS_NOTHROW_COPY(T)          (__has_nothrow_copy(T) || ::bho::move_detail::is_trivially_copy_constructible<T>::value)
#   define BHO_MOVE_HAS_NOTHROW_ASSIGN(T)        (__has_nothrow_assign(T) || ::bho::move_detail::is_trivially_copy_assignable<T>::value)

#   define BHO_MOVE_IS_ENUM(T) __is_enum(T)
#   if defined(_MSC_VER) && (_MSC_VER >= 1700)
#       define BHO_MOVE_HAS_TRIVIAL_MOVE_CONSTRUCTOR(T)   (__has_trivial_move_constructor(T) || ::bho::move_detail::is_pod<T>::value)
#       define BHO_MOVE_HAS_TRIVIAL_MOVE_ASSIGN(T)        (__has_trivial_move_assign(T) || ::bho::move_detail::is_pod<T>::value)
#   endif
#  if _MSC_FULL_VER >= 180020827
#     define BHO_MOVE_HAS_NOTHROW_MOVE_ASSIGN(T) (__is_nothrow_assignable(T&, T&&))
#     define BHO_MOVE_HAS_NOTHROW_MOVE_CONSTRUCTOR(T) (__is_nothrow_constructible(T, T&&))
#  endif
#endif

#if defined(BHO_CLANG)
//    BHO_MOVE_HAS_TRAIT
#   if defined __is_identifier
#       define BHO_MOVE_HAS_TRAIT(T) (__has_extension(T) || !__is_identifier(__##T))
#   elif defined(__has_extension)
#     define BHO_MOVE_HAS_TRAIT(T) __has_extension(T)
#   else
#     define BHO_MOVE_HAS_TRAIT(T) 0
#   endif

//    BHO_MOVE_IS_UNION
#   if BHO_MOVE_HAS_TRAIT(is_union)
#     define BHO_MOVE_IS_UNION(T) __is_union(T)
#   endif

//    BHO_MOVE_IS_ENUM
#   if BHO_MOVE_HAS_TRAIT(is_enum)
#     define BHO_MOVE_IS_ENUM(T) __is_enum(T)
#   endif

//    BHO_MOVE_IS_POD
#   if (!defined(__GLIBCXX__) || (__GLIBCXX__ >= 20080306 && __GLIBCXX__ != 20080519)) && BHO_MOVE_HAS_TRAIT(is_pod)
#     define BHO_MOVE_IS_POD(T) __is_pod(T)
#   endif

//    BHO_MOVE_IS_EMPTY
#   if (!defined(__GLIBCXX__) || (__GLIBCXX__ >= 20080306 && __GLIBCXX__ != 20080519)) && BHO_MOVE_HAS_TRAIT(is_empty)
#     define BHO_MOVE_IS_EMPTY(T) __is_empty(T)
#   endif

//    BHO_MOVE_HAS_TRIVIAL_CONSTRUCTOR
#   if BHO_MOVE_HAS_TRAIT(is_constructible) && BHO_MOVE_HAS_TRAIT(is_trivially_constructible)
#     define BHO_MOVE_HAS_TRIVIAL_CONSTRUCTOR(T) __is_trivially_constructible(T)
#   elif BHO_MOVE_HAS_TRAIT(has_trivial_constructor)
#     define BHO_MOVE_HAS_TRIVIAL_CONSTRUCTOR(T) __has_trivial_constructor(T)
#   endif

//    BHO_MOVE_HAS_TRIVIAL_COPY
#   if BHO_MOVE_HAS_TRAIT(is_constructible) && BHO_MOVE_HAS_TRAIT(is_trivially_constructible)
#     define BHO_MOVE_HAS_TRIVIAL_COPY(T) (__is_constructible(T, const T &) && __is_trivially_constructible(T, const T &))
#   elif BHO_MOVE_HAS_TRAIT(has_trivial_copy)
#     define BHO_MOVE_HAS_TRIVIAL_COPY(T) __has_trivial_copy(T)
#   endif

//    BHO_MOVE_HAS_TRIVIAL_ASSIGN
#   if BHO_MOVE_HAS_TRAIT(is_assignable) && BHO_MOVE_HAS_TRAIT(is_trivially_assignable)
#     define BHO_MOVE_HAS_TRIVIAL_ASSIGN(T) (__is_assignable(T, const T &) && __is_trivially_assignable(T, const T &))
#   elif BHO_MOVE_HAS_TRAIT(has_trivial_copy)
#     define BHO_MOVE_HAS_TRIVIAL_ASSIGN(T) __has_trivial_assign(T)
#   endif

//    BHO_MOVE_HAS_TRIVIAL_DESTRUCTOR
#   if BHO_MOVE_HAS_TRAIT(is_trivially_destructible)
#     define BHO_MOVE_HAS_TRIVIAL_DESTRUCTOR(T) __is_trivially_destructible(T)
#   elif BHO_MOVE_HAS_TRAIT(has_trivial_destructor)
#     define BHO_MOVE_HAS_TRIVIAL_DESTRUCTOR(T) __has_trivial_destructor(T)
#   endif

//    BHO_MOVE_HAS_NOTHROW_CONSTRUCTOR
#   if BHO_MOVE_HAS_TRAIT(is_nothrow_constructible)
#     define BHO_MOVE_HAS_NOTHROW_CONSTRUCTOR(T) __is_nothrow_constructible(T)
#   elif BHO_MOVE_HAS_TRAIT(has_nothrow_constructor)
#     define BHO_MOVE_HAS_NOTHROW_CONSTRUCTOR(T) __has_nothrow_constructor(T)
#   endif

//    BHO_MOVE_HAS_NOTHROW_COPY
#   if BHO_MOVE_HAS_TRAIT(is_constructible) && BHO_MOVE_HAS_TRAIT(is_nothrow_constructible)
#     define BHO_MOVE_HAS_NOTHROW_COPY(T) (__is_constructible(T, const T &) && __is_nothrow_constructible(T, const T &))
#   elif BHO_MOVE_HAS_TRAIT(has_nothrow_copy)
#     define BHO_MOVE_HAS_NOTHROW_COPY(T) (__has_nothrow_copy(T))
#   endif

//    BHO_MOVE_HAS_NOTHROW_ASSIGN
#   if BHO_MOVE_HAS_TRAIT(is_assignable) && BHO_MOVE_HAS_TRAIT(is_nothrow_assignable)
#     define BHO_MOVE_HAS_NOTHROW_ASSIGN(T) (__is_assignable(T, const T &) && __is_nothrow_assignable(T, const T &))
#   elif BHO_MOVE_HAS_TRAIT(has_nothrow_assign)
#     define BHO_MOVE_HAS_NOTHROW_ASSIGN(T) (__has_nothrow_assign(T))
#   endif

//    BHO_MOVE_HAS_TRIVIAL_MOVE_CONSTRUCTOR
#   if !defined(BHO_NO_CXX11_RVALUE_REFERENCES) 

#   if BHO_MOVE_HAS_TRAIT(is_constructible) && BHO_MOVE_HAS_TRAIT(is_trivially_constructible)
#     define BHO_MOVE_HAS_TRIVIAL_MOVE_CONSTRUCTOR(T) (__is_constructible(T, T&&) && __is_trivially_constructible(T, T&&))
#   elif BHO_MOVE_HAS_TRAIT(has_trivial_move_constructor)
#     define BHO_MOVE_HAS_TRIVIAL_MOVE_CONSTRUCTOR(T) __has_trivial_move_constructor(T)
#   endif

//    BHO_MOVE_HAS_TRIVIAL_MOVE_ASSIGN
#   if BHO_MOVE_HAS_TRAIT(is_assignable) && BHO_MOVE_HAS_TRAIT(is_trivially_assignable)
#     define BHO_MOVE_HAS_TRIVIAL_MOVE_ASSIGN(T) (__is_assignable(T, T&&) && __is_trivially_assignable(T, T&&))
#   elif BHO_MOVE_HAS_TRAIT(has_trivial_move_assign)
#     define BHO_MOVE_HAS_TRIVIAL_MOVE_ASSIGN(T) __has_trivial_move_assign(T)
#   endif

//    BHO_MOVE_HAS_NOTHROW_MOVE_CONSTRUCTOR
#   if BHO_MOVE_HAS_TRAIT(is_constructible) && BHO_MOVE_HAS_TRAIT(is_nothrow_constructible)
#     define BHO_MOVE_HAS_NOTHROW_MOVE_CONSTRUCTOR(T) (__is_constructible(T, T&&) && __is_nothrow_constructible(T, T&&))
#   elif BHO_MOVE_HAS_TRAIT(has_nothrow_move_constructor)
#     define BHO_MOVE_HAS_NOTHROW_MOVE_CONSTRUCTOR(T) __has_nothrow_move_constructor(T)
#   endif

//    BHO_MOVE_HAS_NOTHROW_MOVE_ASSIGN
#   if BHO_MOVE_HAS_TRAIT(is_assignable) && BHO_MOVE_HAS_TRAIT(is_nothrow_assignable)
#     define BHO_MOVE_HAS_NOTHROW_MOVE_ASSIGN(T) (__is_assignable(T, T&&) && __is_nothrow_assignable(T, T&&))
#   elif BHO_MOVE_HAS_TRAIT(has_nothrow_move_assign)
#     define BHO_MOVE_HAS_NOTHROW_MOVE_ASSIGN(T) __has_nothrow_move_assign(T)
#   endif

#   endif   //BHO_NO_CXX11_RVALUE_REFERENCES

//    BHO_MOVE_ALIGNMENT_OF
#   define BHO_MOVE_ALIGNMENT_OF(T) __alignof(T)

#endif   //#if defined(BHO_CLANG)

#if defined(__GNUC__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3) && !defined(__GCCXML__))) && !defined(BHO_CLANG)

#ifdef BHO_INTEL
#  define BHO_MOVE_INTEL_TT_OPTS || ::bho::move_detail::is_pod<T>::value
#else
#  define BHO_MOVE_INTEL_TT_OPTS
#endif

#   define BHO_MOVE_IS_UNION(T) __is_union(T)
#   define BHO_MOVE_IS_POD(T) __is_pod(T)
#   define BHO_MOVE_IS_EMPTY(T) __is_empty(T)
#   define BHO_MOVE_HAS_TRIVIAL_CONSTRUCTOR(T) ((__has_trivial_constructor(T) BHO_MOVE_INTEL_TT_OPTS))

#   if defined(BHO_GCC) && (BHO_GCC > 50000)
#     define BHO_MOVE_HAS_TRIVIAL_COPY(T) (__is_trivially_constructible(T, const T &))
#     define BHO_MOVE_HAS_TRIVIAL_ASSIGN(T) (__is_trivially_assignable(T, const T &))
#   else
#     define BHO_MOVE_HAS_TRIVIAL_COPY(T) ((__has_trivial_copy(T) BHO_MOVE_INTEL_TT_OPTS))
#     define BHO_MOVE_HAS_TRIVIAL_ASSIGN(T) ((__has_trivial_assign(T) BHO_MOVE_INTEL_TT_OPTS) )
#   endif

#   define BHO_MOVE_HAS_TRIVIAL_DESTRUCTOR(T) (__has_trivial_destructor(T) BHO_MOVE_INTEL_TT_OPTS)
#   define BHO_MOVE_HAS_NOTHROW_CONSTRUCTOR(T) (__has_nothrow_constructor(T) BHO_MOVE_INTEL_TT_OPTS)
#   define BHO_MOVE_HAS_NOTHROW_COPY(T) ((__has_nothrow_copy(T) BHO_MOVE_INTEL_TT_OPTS))
#   define BHO_MOVE_HAS_NOTHROW_ASSIGN(T) ((__has_nothrow_assign(T) BHO_MOVE_INTEL_TT_OPTS))

#if !defined(BHO_NO_CXX11_RVALUE_REFERENCES) && !defined(BHO_NO_CXX11_SFINAE_EXPR)

   template <typename T>
   T && bho_move_tt_declval() BHO_NOEXCEPT;

#  if defined(BHO_GCC) && (BHO_GCC >= 80000)
// __is_assignable / __is_constructible implemented
#     define BHO_MOVE_IS_ASSIGNABLE(T, U)     __is_assignable(T, U)
#     define BHO_MOVE_IS_CONSTRUCTIBLE(T, U)  __is_constructible(T, U)
#  else

   template<typename Tt, typename Ut>
   class bho_move_tt_is_assignable
   {
      struct twochar {  char dummy[2]; };
      template < class T
               , class U
               , class = decltype(bho_move_tt_declval<T>() = bho_move_tt_declval<U>())
               > static char test(int);

      template<class, class> static twochar test(...);

      public:
      static const bool value = sizeof(test<Tt, Ut>(0)) == sizeof(char);
   };

   template<typename Tt, typename Ut>
   class bho_move_tt_is_constructible
   {
      struct twochar {  char dummy[2]; };
      template < class T
               , class U
               , class = decltype(T(bho_move_tt_declval<U>()))
               > static char test(int);

      template<class, class> static twochar test(...);

      public:
      static const bool value = sizeof(test<Tt, Ut>(0)) == sizeof(char);
   };

#     define BHO_MOVE_IS_ASSIGNABLE(T, U)     bho_move_tt_is_assignable<T,U>::value
#     define BHO_MOVE_IS_CONSTRUCTIBLE(T, U)  bho_move_tt_is_constructible<T, U>::value

#  endif

   template <typename T, typename U, bool = BHO_MOVE_IS_ASSIGNABLE(T, U)>
   struct bho_move_tt_is_nothrow_assignable
   {
      static const bool value = false;
   };

   template <typename T, typename U>
   struct bho_move_tt_is_nothrow_assignable<T, U, true>
   {
      #if !defined(BHO_NO_CXX11_NOEXCEPT)
      static const bool value = noexcept(bho_move_tt_declval<T>() = bho_move_tt_declval<U>());
      #else
      static const bool value = false;
      #endif
   };

   template <typename T, typename U, bool = BHO_MOVE_IS_CONSTRUCTIBLE(T, U)>
   struct bho_move_tt_is_nothrow_constructible
   {
      static const bool value = false;
   };

   template <typename T, typename U>
   struct bho_move_tt_is_nothrow_constructible<T, U, true>
   {
      #if !defined(BHO_NO_CXX11_NOEXCEPT)
      static const bool value = noexcept(T(bho_move_tt_declval<U>()));
      #else
      static const bool value = false;
      #endif
   };

#     define BHO_MOVE_HAS_NOTHROW_MOVE_ASSIGN(T)       bho_move_tt_is_nothrow_assignable<T, T&&>::value
#     define BHO_MOVE_HAS_NOTHROW_MOVE_CONSTRUCTOR(T)  bho_move_tt_is_nothrow_constructible<T, T&&>::value

#  endif

#   define BHO_MOVE_IS_ENUM(T) __is_enum(T)

// BHO_MOVE_ALIGNMENT_OF
#   if (!defined(unix) && !defined(__unix__)) || defined(__LP64__)
      // GCC sometimes lies about alignment requirements
      // of type double on 32-bit unix platforms, use the
      // old implementation instead in that case:
#     define BHO_MOVE_ALIGNMENT_OF(T) __alignof__(T)
#   endif
#endif

#if defined(__ghs__) && (__GHS_VERSION_NUMBER >= 600)

#   define BHO_MOVE_IS_UNION(T) __is_union(T)
#   define BHO_MOVE_IS_POD(T) __is_pod(T)
#   define BHO_MOVE_IS_EMPTY(T) __is_empty(T)
#   define BHO_MOVE_HAS_TRIVIAL_CONSTRUCTOR(T) __has_trivial_constructor(T)
#   define BHO_MOVE_HAS_TRIVIAL_COPY(T) (__has_trivial_copy(T))
#   define BHO_MOVE_HAS_TRIVIAL_ASSIGN(T) (__has_trivial_assign(T))
#   define BHO_MOVE_HAS_TRIVIAL_DESTRUCTOR(T) __has_trivial_destructor(T)
#   define BHO_MOVE_HAS_NOTHROW_CONSTRUCTOR(T) __has_nothrow_constructor(T)
#   define BHO_MOVE_HAS_NOTHROW_COPY(T) (__has_nothrow_copy(T))
#   define BHO_MOVE_HAS_NOTHROW_ASSIGN(T) (__has_nothrow_assign(T))

#   define BHO_MOVE_IS_ENUM(T) __is_enum(T)
#   define BHO_MOVE_ALIGNMENT_OF(T) __alignof__(T)
#endif

# if defined(BHO_CODEGEARC)
#   define BHO_MOVE_IS_UNION(T) __is_union(T)
#   define BHO_MOVE_IS_POD(T) __is_pod(T)
#   define BHO_MOVE_IS_EMPTY(T) __is_empty(T)
#   define BHO_MOVE_HAS_TRIVIAL_CONSTRUCTOR(T) (__has_trivial_default_constructor(T))
#   define BHO_MOVE_HAS_TRIVIAL_COPY(T) (__has_trivial_copy_constructor(T))
#   define BHO_MOVE_HAS_TRIVIAL_ASSIGN(T) (__has_trivial_assign(T))
#   define BHO_MOVE_HAS_TRIVIAL_DESTRUCTOR(T) (__has_trivial_destructor(T))
#   define BHO_MOVE_HAS_NOTHROW_CONSTRUCTOR(T) (__has_nothrow_default_constructor(T))
#   define BHO_MOVE_HAS_NOTHROW_COPY(T) (__has_nothrow_copy_constructor(T))
#   define BHO_MOVE_HAS_NOTHROW_ASSIGN(T) (__has_nothrow_assign(T))

#   define BHO_MOVE_IS_ENUM(T) __is_enum(T)
#   define BHO_MOVE_ALIGNMENT_OF(T) alignof(T)

#endif

//Fallback definitions

#ifdef BHO_MOVE_IS_UNION
   #define BHO_MOVE_IS_UNION_IMPL(T) BHO_MOVE_IS_UNION(T)
#else
   #define BHO_MOVE_IS_UNION_IMPL(T) false
#endif

#ifdef BHO_MOVE_IS_POD
   //in some compilers the intrinsic is limited to class types so add scalar and void
   #define BHO_MOVE_IS_POD_IMPL(T) (::bho::move_detail::is_scalar<T>::value ||\
                                      ::bho::move_detail::is_void<T>::value   ||\
                                       BHO_MOVE_IS_POD(T))
#else
   #define BHO_MOVE_IS_POD_IMPL(T) \
      (::bho::move_detail::is_scalar<T>::value || ::bho::move_detail::is_void<T>::value)
#endif

#ifdef BHO_MOVE_IS_EMPTY
   #define BHO_MOVE_IS_EMPTY_IMPL(T) BHO_MOVE_IS_EMPTY(T)
#else
   #define BHO_MOVE_IS_EMPTY_IMPL(T)    ::bho::move_detail::is_empty_nonintrinsic<T>::value
#endif

#ifdef BHO_MOVE_HAS_TRIVIAL_COPY
   #define BHO_MOVE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(T)   ::bho::move_detail::is_pod<T>::value ||\
                                                          (::bho::move_detail::is_copy_constructible<T>::value &&\
                                                           BHO_MOVE_HAS_TRIVIAL_COPY(T))
#else
   #define BHO_MOVE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(T)   ::bho::move_detail::is_pod<T>::value
#endif

#ifdef BHO_MOVE_HAS_TRIVIAL_CONSTRUCTOR
   #define BHO_MOVE_IS_TRIVIALLY_DEFAULT_CONSTRUCTIBLE(T)  BHO_MOVE_HAS_TRIVIAL_CONSTRUCTOR(T) || ::bho::move_detail::is_pod<T>::value
#else
   #define BHO_MOVE_IS_TRIVIALLY_DEFAULT_CONSTRUCTIBLE(T)  ::bho::move_detail::is_pod<T>::value
#endif

#ifdef BHO_MOVE_HAS_TRIVIAL_MOVE_CONSTRUCTOR
   #define BHO_MOVE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(T)   BHO_MOVE_HAS_TRIVIAL_MOVE_CONSTRUCTOR(T) || ::bho::move_detail::is_pod<T>::value
#else
   #define BHO_MOVE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(T)   ::bho::move_detail::is_pod<T>::value
#endif

#ifdef BHO_MOVE_HAS_TRIVIAL_ASSIGN
   #define BHO_MOVE_IS_TRIVIALLY_COPY_ASSIGNABLE(T) ::bho::move_detail::is_pod<T>::value ||\
                                                      ( ::bho::move_detail::is_copy_assignable<T>::value &&\
                                                         BHO_MOVE_HAS_TRIVIAL_ASSIGN(T))
#else
   #define BHO_MOVE_IS_TRIVIALLY_COPY_ASSIGNABLE(T) ::bho::move_detail::is_pod<T>::value
#endif

#ifdef BHO_MOVE_HAS_TRIVIAL_MOVE_ASSIGN
   #define BHO_MOVE_IS_TRIVIALLY_MOVE_ASSIGNABLE(T)  BHO_MOVE_HAS_TRIVIAL_MOVE_ASSIGN(T) || ::bho::move_detail::is_pod<T>::value
#else
   #define BHO_MOVE_IS_TRIVIALLY_MOVE_ASSIGNABLE(T)  ::bho::move_detail::is_pod<T>::value
#endif

#ifdef BHO_MOVE_HAS_TRIVIAL_DESTRUCTOR
   #define BHO_MOVE_IS_TRIVIALLY_DESTRUCTIBLE(T)   BHO_MOVE_HAS_TRIVIAL_DESTRUCTOR(T) || ::bho::move_detail::is_pod<T>::value
#else
   #define BHO_MOVE_IS_TRIVIALLY_DESTRUCTIBLE(T)   ::bho::move_detail::is_pod<T>::value
#endif

#ifdef BHO_MOVE_HAS_NOTHROW_CONSTRUCTOR
   #define BHO_MOVE_IS_NOTHROW_DEFAULT_CONSTRUCTIBLE(T)  BHO_MOVE_HAS_NOTHROW_CONSTRUCTOR(T) || ::bho::move_detail::is_pod<T>::value
#else
   #define BHO_MOVE_IS_NOTHROW_DEFAULT_CONSTRUCTIBLE(T)  ::bho::move_detail::is_pod<T>::value
#endif

#ifdef BHO_MOVE_HAS_NOTHROW_COPY
   #define BHO_MOVE_IS_NOTHROW_COPY_CONSTRUCTIBLE(T)   BHO_MOVE_HAS_NOTHROW_COPY(T) || ::bho::move_detail::is_pod<T>::value
#else
   #define BHO_MOVE_IS_NOTHROW_COPY_CONSTRUCTIBLE(T)   BHO_MOVE_IS_TRIVIALLY_COPY_ASSIGNABLE(T)
#endif

#ifdef BHO_MOVE_HAS_NOTHROW_ASSIGN
   #define BHO_MOVE_IS_NOTHROW_COPY_ASSIGNABLE(T) BHO_MOVE_HAS_NOTHROW_ASSIGN(T) || ::bho::move_detail::is_pod<T>::value
#else
   #define BHO_MOVE_IS_NOTHROW_COPY_ASSIGNABLE(T) BHO_MOVE_IS_TRIVIALLY_COPY_ASSIGNABLE(T)
#endif

#ifdef BHO_MOVE_HAS_NOTHROW_MOVE_CONSTRUCTOR
   #define BHO_MOVE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(T)   BHO_MOVE_HAS_NOTHROW_MOVE_CONSTRUCTOR(T) || ::bho::move_detail::is_pod<T>::value
#else
   #define BHO_MOVE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(T)   BHO_MOVE_IS_TRIVIALLY_MOVE_ASSIGNABLE(T)
#endif

#ifdef BHO_MOVE_HAS_NOTHROW_MOVE_ASSIGN
   #define BHO_MOVE_IS_NOTHROW_MOVE_ASSIGNABLE(T) BHO_MOVE_HAS_NOTHROW_MOVE_ASSIGN(T) || ::bho::move_detail::is_pod<T>::value
#else
   #define BHO_MOVE_IS_NOTHROW_MOVE_ASSIGNABLE(T) BHO_MOVE_IS_TRIVIALLY_MOVE_ASSIGNABLE(T)
#endif

#ifdef BHO_MOVE_IS_ENUM
   #define BHO_MOVE_IS_ENUM_IMPL(T)   BHO_MOVE_IS_ENUM(T)
#else
   #define BHO_MOVE_IS_ENUM_IMPL(T)   ::bho::move_detail::is_enum_nonintrinsic<T>::value
#endif

namespace bho {
namespace move_detail {

//////////////////////////
//    is_reference
//////////////////////////
template<class T>
struct is_reference
{  static const bool value = false; };

template<class T>
struct is_reference<T&>
{  static const bool value = true; };

#if !defined(BHO_NO_CXX11_RVALUE_REFERENCES)
template<class T>
struct is_reference<T&&>
{  static const bool value = true; };
#endif

//////////////////////////
//    is_pointer
//////////////////////////
template<class T>
struct is_pointer
{  static const bool value = false; };

template<class T>
struct is_pointer<T*>
{  static const bool value = true; };

//////////////////////////
//       is_const
//////////////////////////
template<class T>
struct is_const
{  static const bool value = false; };

template<class T>
struct is_const<const T>
{  static const bool value = true; };

//////////////////////////
//       unvoid_ref
//////////////////////////
template <typename T> struct unvoid_ref : add_lvalue_reference<T>{};
template <> struct unvoid_ref<void>                { typedef unvoid_ref & type; };
template <> struct unvoid_ref<const void>          { typedef unvoid_ref & type; };
template <> struct unvoid_ref<volatile void>       { typedef unvoid_ref & type; };
template <> struct unvoid_ref<const volatile void> { typedef unvoid_ref & type; };

template <typename T>
struct add_reference : add_lvalue_reference<T>
{};

//////////////////////////
//    add_const_reference
//////////////////////////
template <class T>
struct add_const_reference
{  typedef const T &type;   };

template <class T>
struct add_const_reference<T&>
{  typedef T& type;   };

//////////////////////////
//    add_const_if_c
//////////////////////////
template<class T, bool Add>
struct add_const_if_c
   : if_c<Add, typename add_const<T>::type, T>
{};

//////////////////////////
//    remove_const
//////////////////////////
template<class T>
struct remove_const
{  typedef T type;   };

template<class T>
struct remove_const< const T>
{  typedef T type;   };

//////////////////////////
//    remove_cv
//////////////////////////
template<typename T> struct remove_cv                    {  typedef T type;   };
template<typename T> struct remove_cv<const T>           {  typedef T type;   };
template<typename T> struct remove_cv<const volatile T>  {  typedef T type;   };
template<typename T> struct remove_cv<volatile T>        {  typedef T type;   };

//////////////////////////
//    remove_cvref
//////////////////////////
template<class T>
struct remove_cvref
   : remove_cv<typename remove_reference<T>::type>
{
};

//////////////////////////
//    make_unsigned
//////////////////////////
template <class T>
struct make_unsigned_impl                                         {  typedef T type;   };
template <> struct make_unsigned_impl<signed char>                {  typedef unsigned char  type; };
template <> struct make_unsigned_impl<signed short>               {  typedef unsigned short type; };
template <> struct make_unsigned_impl<signed int>                 {  typedef unsigned int   type; };
template <> struct make_unsigned_impl<signed long>                {  typedef unsigned long  type; };
#ifdef BHO_HAS_LONG_LONG
template <> struct make_unsigned_impl< ::bho::long_long_type >  {  typedef ::bho::ulong_long_type type; };
#endif

template <class T>
struct make_unsigned
   : make_unsigned_impl<typename remove_cv<T>::type>
{};

//////////////////////////
//    is_floating_point
//////////////////////////
template<class T> struct is_floating_point_cv               {  static const bool value = false; };
template<>        struct is_floating_point_cv<float>        {  static const bool value = true; };
template<>        struct is_floating_point_cv<double>       {  static const bool value = true; };
template<>        struct is_floating_point_cv<long double>  {  static const bool value = true; };

template<class T>
struct is_floating_point
   : is_floating_point_cv<typename remove_cv<T>::type>
{};

//////////////////////////
//    is_integral
//////////////////////////
template<class T> struct is_integral_cv                    {  static const bool value = false; };
template<> struct is_integral_cv<                     bool>{  static const bool value = true; };
template<> struct is_integral_cv<                     char>{  static const bool value = true; };
template<> struct is_integral_cv<            unsigned char>{  static const bool value = true; };
template<> struct is_integral_cv<              signed char>{  static const bool value = true; };
#ifndef BHO_NO_CXX11_CHAR16_T
template<> struct is_integral_cv<                 char16_t>{  static const bool value = true; };
#endif
#ifndef BHO_NO_CXX11_CHAR32_T
template<> struct is_integral_cv<                 char32_t>{  static const bool value = true; };
#endif
#ifndef BHO_NO_INTRINSIC_WCHAR_T
template<> struct is_integral_cv<                  wchar_t>{  static const bool value = true; };
#endif
template<> struct is_integral_cv<                    short>{  static const bool value = true; };
template<> struct is_integral_cv<           unsigned short>{  static const bool value = true; };
template<> struct is_integral_cv<                      int>{  static const bool value = true; };
template<> struct is_integral_cv<             unsigned int>{  static const bool value = true; };
template<> struct is_integral_cv<                     long>{  static const bool value = true; };
template<> struct is_integral_cv<            unsigned long>{  static const bool value = true; };
#ifdef BHO_HAS_LONG_LONG
template<> struct is_integral_cv< ::bho:: long_long_type>{  static const bool value = true; };
template<> struct is_integral_cv< ::bho::ulong_long_type>{  static const bool value = true; };
#endif

template<class T>
struct is_integral
   : public is_integral_cv<typename remove_cv<T>::type>
{};

//////////////////////////////////////
//          remove_all_extents
//////////////////////////////////////
template <class T>
struct remove_all_extents
{  typedef T type;};

template <class T>
struct remove_all_extents<T[]>
{  typedef typename remove_all_extents<T>::type type; };

template <class T, std::size_t N>
struct remove_all_extents<T[N]>
{  typedef typename remove_all_extents<T>::type type;};

//////////////////////////
//    is_scalar
//////////////////////////
template<class T>
struct is_scalar
{  static const bool value = std::is_integral<T>::value || is_floating_point<T>::value; };

//////////////////////////
//       is_void
//////////////////////////
template<class T>
struct is_void_cv
{  static const bool value = false; };

template<>
struct is_void_cv<void>
{  static const bool value = true; };

template<class T>
struct is_void
   : is_void_cv<typename remove_cv<T>::type>
{};

//////////////////////////////////////
//          is_array
//////////////////////////////////////
template<class T>
struct is_array
{  static const bool value = false; };

template<class T>
struct is_array<T[]>
{  static const bool value = true;  };

template<class T, std::size_t N>
struct is_array<T[N]>
{  static const bool value = true;  };

//////////////////////////////////////
//           is_member_pointer
//////////////////////////////////////
template <class T>         struct is_member_pointer_cv         {  static const bool value = false; };
template <class T, class U>struct is_member_pointer_cv<T U::*> {  static const bool value = true; };

template <class T>
struct is_member_pointer
    : is_member_pointer_cv<typename remove_cv<T>::type>
{};

//////////////////////////////////////
//          is_nullptr_t
//////////////////////////////////////
template <class T>
struct is_nullptr_t_cv
{  static const bool value = false; };

#if !defined(BHO_NO_CXX11_NULLPTR)
template <>
struct is_nullptr_t_cv
   #if !defined(BHO_NO_CXX11_DECLTYPE)
   <decltype(nullptr)>
   #else
   <std::nullptr_t>
   #endif
{  static const bool value = true; };
#endif

template <class T>
struct is_nullptr_t
   : is_nullptr_t_cv<typename remove_cv<T>::type>
{};

//////////////////////////////////////
//          is_function
//////////////////////////////////////
//Inspired by libc++, thanks to Howard Hinnant
//For a function to pointer an lvalue of function type T can be implicitly converted to a prvalue
//pointer to that function. This does not apply to non-static member functions because lvalues
//that refer to non-static member functions do not exist.
template <class T>
struct is_reference_convertible_to_pointer
{
   struct twochar { char dummy[2]; };
   template <class U> static char    test(U*);
   template <class U> static twochar test(...);
   static T& source();
   static const bool value = sizeof(char) == sizeof(test<T>(source()));
};
//Filter out:
// - class types that might have implicit conversions
// - void (to avoid forming a reference to void later)
// - references (e.g.: filtering reference to functions)
// - nullptr_t (convertible to pointer)
template < class T
         , bool Filter = is_class_or_union<T>::value  ||
                         is_void<T>::value            ||
                         is_reference<T>::value       ||
                         is_nullptr_t<T>::value       >
struct is_function_impl
{  static const bool value = is_reference_convertible_to_pointer<T>::value; };

template <class T>
struct is_function_impl<T, true>
{  static const bool value = false; };

template <class T>
struct is_function
   : is_function_impl<T>
{};

//////////////////////////////////////
//       is_union
//////////////////////////////////////
template<class T>
struct is_union_noextents_cv
{  static const bool value = BHO_MOVE_IS_UNION_IMPL(T); };

template<class T>
struct is_union
   : is_union_noextents_cv<typename remove_cv<typename remove_all_extents<T>::type>::type>
{};

//////////////////////////////////////
//             is_class
//////////////////////////////////////
template <class T>
struct is_class
{
   static const bool value = is_class_or_union<T>::value && ! is_union<T>::value;
};


//////////////////////////////////////
//             is_arithmetic
//////////////////////////////////////
template <class T>
struct is_arithmetic
{
   static const bool value = is_floating_point<T>::value ||
                             std::is_integral<T>::value;
};

//////////////////////////////////////
//    is_member_function_pointer
//////////////////////////////////////
template <class T>
struct is_member_function_pointer_cv
{
   static const bool value = false;
};

template <class T, class C>
struct is_member_function_pointer_cv<T C::*>
   : is_function<T>
{};

template <class T>
struct is_member_function_pointer
    : is_member_function_pointer_cv<typename remove_cv<T>::type>
{};

//////////////////////////////////////
//             is_enum
//////////////////////////////////////
#if !defined(BHO_MOVE_IS_ENUM)
//Based on (http://howardhinnant.github.io/TypeHiearchy.pdf)
template <class T>
struct is_enum_nonintrinsic
{
   static const bool value =  !is_arithmetic<T>::value     &&
                              !is_reference<T>::value      &&
                              !is_class_or_union<T>::value &&
                              !is_array<T>::value          &&
                              !is_void<T>::value           &&
                              !is_nullptr_t<T>::value      &&
                              !is_member_pointer<T>::value &&
                              !is_pointer<T>::value        &&
                              !is_function<T>::value;
};
#endif

template <class T>
struct is_enum
{  static const bool value = BHO_MOVE_IS_ENUM_IMPL(T);  };

//////////////////////////////////////
//       is_pod
//////////////////////////////////////
template<class T>
struct is_pod_noextents_cv  //for non-c++11 compilers, a safe fallback
{  static const bool value = BHO_MOVE_IS_POD_IMPL(T); };

template<class T>
struct is_pod
   : is_pod_noextents_cv<typename remove_cv<typename remove_all_extents<T>::type>::type>
{};

//////////////////////////////////////
//             is_empty
//////////////////////////////////////
#if !defined(BHO_MOVE_IS_EMPTY)

template <typename T>
struct empty_helper_t1 : public T
{
   empty_helper_t1();  // hh compiler bug workaround
   int i[256];
   private:

   empty_helper_t1(const empty_helper_t1&);
   empty_helper_t1& operator=(const empty_helper_t1&);
};

struct empty_helper_t2 { int i[256]; };

template <typename T, bool IsClass = std::is_class<T>::value >
struct is_empty_nonintrinsic
{
   static const bool value = false;
};

template <typename T>
struct is_empty_nonintrinsic<T, true>
{
   static const bool value = sizeof(empty_helper_t1<T>) == sizeof(empty_helper_t2);
};
#endif

template <class T>
struct is_empty
{  static const bool value = BHO_MOVE_IS_EMPTY_IMPL(T);  };


template<class T>
struct has_bho_move_no_copy_constructor_or_assign_type
{
   template <class U>
   static yes_type test(typename U::bho_move_no_copy_constructor_or_assign*);

   template <class U>
   static no_type test(...);

   static const bool value = sizeof(test<T>(0)) == sizeof(yes_type);
};

//////////////////////////////////////
//       is_copy_constructible
//////////////////////////////////////
#if !defined(BHO_NO_CXX11_DELETED_FUNCTIONS) && !defined(BHO_NO_CXX11_DECLTYPE) \
   && !defined(BHO_INTEL_CXX_VERSION) && \
      !(defined(BHO_MSVC) && _MSC_VER == 1800)
#define BHO_MOVE_TT_CXX11_IS_COPY_CONSTRUCTIBLE
#endif

template<class T>
struct is_copy_constructible
{
   // Intel compiler has problems with SFINAE for copy constructors and deleted functions:
   //
   // error: function *function_name* cannot be referenced -- it is a deleted function
   // static yes_type test(U&, decltype(U(bho::declval<U&>()))* = 0);
   //                                                        ^ 
   // MSVC 12.0 (Visual 2013) has problems when the copy constructor has been deleted. See:
   // https://connect.microsoft.com/VisualStudio/feedback/details/800328/std-is-copy-constructible-is-broken
   #if defined(BHO_MOVE_TT_CXX11_IS_COPY_CONSTRUCTIBLE)
      template<class U> static typename add_reference<U>::type source();
      static no_type test(...);
      #ifdef BHO_NO_CXX11_DECLTYPE
         template <class U>
         static yes_type test(U&, bool_<sizeof(U(source<U>()))>* = 0);
      #else
         template <class U>
         static yes_type test(U&, decltype(U(source<U>()))* = 0);
      #endif
      static const bool value = sizeof(test(source<T>())) == sizeof(yes_type);
   #else
   static const bool value = !has_bho_move_no_copy_constructor_or_assign_type<T>::value;
   #endif
};


//////////////////////////////////////
//       is_copy_assignable
//////////////////////////////////////
#if !defined(BHO_NO_CXX11_DELETED_FUNCTIONS) && !defined(BHO_NO_CXX11_DECLTYPE) \
   && !defined(BHO_INTEL_CXX_VERSION) && \
      !(defined(BHO_MSVC) && _MSC_VER == 1800)
#define BHO_MOVE_TT_CXX11_IS_COPY_ASSIGNABLE
#endif

template <class T>
struct is_copy_assignable
{
// Intel compiler has problems with SFINAE for copy constructors and deleted functions:
//
// error: function *function_name* cannot be referenced -- it is a deleted function
// static bho::type_traits::yes_type test(T1&, decltype(T1(bho::declval<T1&>()))* = 0);
//                                                        ^ 
//
// MSVC 12.0 (Visual 2013) has problems when the copy constructor has been deleted. See:
// https://connect.microsoft.com/VisualStudio/feedback/details/800328/std-is-copy-constructible-is-broken
#if defined(BHO_MOVE_TT_CXX11_IS_COPY_ASSIGNABLE)
   typedef char yes_type;
   struct no_type { char dummy[2]; };
   
   template <class U>   static typename add_reference<U>::type source();
   template <class U>   static decltype(source<U&>() = source<const U&>(), yes_type() ) test(int);
   template <class>     static no_type test(...);

   static const bool value = sizeof(test<T>(0)) == sizeof(yes_type);
#else
   static const bool value = !has_bho_move_no_copy_constructor_or_assign_type<T>::value;
#endif
};

//////////////////////////////////////
//       is_trivially_destructible
//////////////////////////////////////
template<class T>
struct is_trivially_destructible
{  static const bool value = BHO_MOVE_IS_TRIVIALLY_DESTRUCTIBLE(T); };

//////////////////////////////////////
//       is_trivially_default_constructible
//////////////////////////////////////
template<class T>
struct is_trivially_default_constructible
{  static const bool value = BHO_MOVE_IS_TRIVIALLY_DEFAULT_CONSTRUCTIBLE(T); };

//////////////////////////////////////
//       is_trivially_copy_constructible
//////////////////////////////////////
template<class T>
struct is_trivially_copy_constructible
{
   static const bool value = BHO_MOVE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(T);
};

//////////////////////////////////////
//       is_trivially_move_constructible
//////////////////////////////////////
template<class T>
struct is_trivially_move_constructible
{ static const bool value = BHO_MOVE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(T); };

//////////////////////////////////////
//       is_trivially_copy_assignable
//////////////////////////////////////
template<class T>
struct is_trivially_copy_assignable
{
   static const bool value = BHO_MOVE_IS_TRIVIALLY_COPY_ASSIGNABLE(T);
};                             

//////////////////////////////////////
//       is_trivially_move_assignable
//////////////////////////////////////
template<class T>
struct is_trivially_move_assignable
{  static const bool value = BHO_MOVE_IS_TRIVIALLY_MOVE_ASSIGNABLE(T);  };

//////////////////////////////////////
//       is_nothrow_default_constructible
//////////////////////////////////////
template<class T>
struct is_nothrow_default_constructible
{  static const bool value = BHO_MOVE_IS_NOTHROW_DEFAULT_CONSTRUCTIBLE(T);  };

//////////////////////////////////////
//    is_nothrow_copy_constructible
//////////////////////////////////////
template<class T>
struct is_nothrow_copy_constructible
{  static const bool value = BHO_MOVE_IS_NOTHROW_COPY_CONSTRUCTIBLE(T);  };

//////////////////////////////////////
//    is_nothrow_move_constructible
//////////////////////////////////////
template<class T>
struct is_nothrow_move_constructible
{  static const bool value = BHO_MOVE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(T);  };

//////////////////////////////////////
//       is_nothrow_copy_assignable
//////////////////////////////////////
template<class T>
struct is_nothrow_copy_assignable
{  static const bool value = BHO_MOVE_IS_NOTHROW_COPY_ASSIGNABLE(T);  };

//////////////////////////////////////
//    is_nothrow_move_assignable
//////////////////////////////////////
template<class T>
struct is_nothrow_move_assignable
{  static const bool value = BHO_MOVE_IS_NOTHROW_MOVE_ASSIGNABLE(T);  };

//////////////////////////////////////
//    is_nothrow_swappable
//////////////////////////////////////
template<class T>
struct is_nothrow_swappable
{
   static const bool value = is_empty<T>::value || is_pod<T>::value;
};

//////////////////////////////////////
//       alignment_of
//////////////////////////////////////
template <typename T>
struct alignment_of_hack
{
   T t1;
   char c;
   T t2;
   alignment_of_hack();
   ~alignment_of_hack();
};

template <unsigned A, unsigned S>
struct alignment_logic
{  static const std::size_t value = A < S ? A : S; };

template< typename T >
struct alignment_of_impl
#if defined(BHO_MSVC) && (BHO_MSVC >= 1400)
    // With MSVC both the native __alignof operator
    // and our own logic gets things wrong from time to time :-(
    // Using a combination of the two seems to make the most of a bad job:
   : alignment_logic< sizeof(alignment_of_hack<T>) - 2*sizeof(T), __alignof(T)>
{};
#elif !defined(BHO_MOVE_ALIGNMENT_OF)
   : alignment_logic< sizeof(alignment_of_hack<T>) - 2*sizeof(T), sizeof(T)>
{};
#else
{  static const std::size_t value = BHO_MOVE_ALIGNMENT_OF(T);  };
#endif

template< typename T >
struct alignment_of
   : alignment_of_impl<T>
{};

class alignment_dummy;
typedef void (*function_ptr)();
typedef int (alignment_dummy::*member_ptr);

struct alignment_struct
{  long double dummy[4];  };

/////////////////////////////
//    max_align_t
/////////////////////////////
//This is not standard, but should work with all compilers
union max_align
{
   char        char_;
   short       short_;
   int         int_;
   long        long_;
   #ifdef BHO_HAS_LONG_LONG
   ::bho::long_long_type   long_long_;
   #endif
   float       float_;
   double      double_;
   void *      void_ptr_;
   long double long_double_[4];
   alignment_dummy *unknown_class_ptr_;
   function_ptr function_ptr_;
   alignment_struct alignment_struct_;
};

typedef union max_align max_align_t;

/////////////////////////////
//    aligned_storage
/////////////////////////////

#if defined(_MSC_VER) && defined(_M_IX86)

// Special version for usual alignments on x86 MSVC because it might crash
// when passsing aligned types by value even for 8 byte alignment.
template<std::size_t Align>
struct aligned_struct;

template <> struct aligned_struct<1> { char data; };
template <> struct aligned_struct<2> { short data; };
template <> struct aligned_struct<4> { int data; };
template <> struct aligned_struct<8> { double data; };

#define BHO_MOVE_ALIGNED_STRUCT(x) \
  template <> struct aligned_struct<x> { \
    __declspec(align(x)) char data; \
  }
BHO_MOVE_ALIGNED_STRUCT(16);
BHO_MOVE_ALIGNED_STRUCT(32);
BHO_MOVE_ALIGNED_STRUCT(64);
BHO_MOVE_ALIGNED_STRUCT(128);
BHO_MOVE_ALIGNED_STRUCT(512);
BHO_MOVE_ALIGNED_STRUCT(1024);
BHO_MOVE_ALIGNED_STRUCT(2048);
BHO_MOVE_ALIGNED_STRUCT(4096);

template<std::size_t Len, std::size_t Align>
union aligned_union
{
   typedef aligned_struct<Align> aligner_t;
   aligner_t aligner;
   unsigned char data[Len > sizeof(aligner_t) ? Len : sizeof(aligner_t)];
};

template<std::size_t Len, std::size_t Align>
struct aligned_storage_impl
{
   typedef aligned_union<Len, Align> type;
};

#elif !defined(BHO_NO_ALIGNMENT)

template<std::size_t Len, std::size_t Align>
struct aligned_struct;

#define BHO_MOVE_ALIGNED_STORAGE_WITH_BHO_ALIGNMENT(A)\
template<std::size_t Len>\
struct BHO_ALIGNMENT(A) aligned_struct<Len, A>\
{\
   unsigned char data[Len];\
};\
//

//Up to 4K alignment (typical page size)
BHO_MOVE_ALIGNED_STORAGE_WITH_BHO_ALIGNMENT(0x1)
BHO_MOVE_ALIGNED_STORAGE_WITH_BHO_ALIGNMENT(0x2)
BHO_MOVE_ALIGNED_STORAGE_WITH_BHO_ALIGNMENT(0x4)
BHO_MOVE_ALIGNED_STORAGE_WITH_BHO_ALIGNMENT(0x8)
BHO_MOVE_ALIGNED_STORAGE_WITH_BHO_ALIGNMENT(0x10)
BHO_MOVE_ALIGNED_STORAGE_WITH_BHO_ALIGNMENT(0x20)
BHO_MOVE_ALIGNED_STORAGE_WITH_BHO_ALIGNMENT(0x40)
BHO_MOVE_ALIGNED_STORAGE_WITH_BHO_ALIGNMENT(0x80)
BHO_MOVE_ALIGNED_STORAGE_WITH_BHO_ALIGNMENT(0x100)
BHO_MOVE_ALIGNED_STORAGE_WITH_BHO_ALIGNMENT(0x200)
BHO_MOVE_ALIGNED_STORAGE_WITH_BHO_ALIGNMENT(0x400)
BHO_MOVE_ALIGNED_STORAGE_WITH_BHO_ALIGNMENT(0x800)
BHO_MOVE_ALIGNED_STORAGE_WITH_BHO_ALIGNMENT(0x1000)

#undef BHO_MOVE_ALIGNED_STORAGE_WITH_BHO_ALIGNMENT

// Workaround for bogus [-Wignored-attributes] warning on GCC 6.x/7.x: don't use a type that "directly" carries the alignment attribute.
// See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=82270
template<std::size_t Len, std::size_t Align>
union aligned_struct_wrapper
{
   typedef aligned_struct<Len, Align> aligner_t;
   aligned_struct<Len, Align> aligner;
   unsigned char data[Len > sizeof(aligner_t) ? Len : sizeof(aligner_t)];
};

template<std::size_t Len, std::size_t Align>
struct aligned_storage_impl
{
   typedef aligned_struct_wrapper<Len, Align> type;
};

#else //BHO_NO_ALIGNMENT

template<class T, std::size_t Len>
union aligned_union
{   
   T aligner;
   unsigned char data[Len > sizeof(T) ? Len : sizeof(T)];
};

template<std::size_t Len, std::size_t Align, class T, bool Ok>
struct aligned_next;

template<std::size_t Len, std::size_t Align, class T>
struct aligned_next<Len, Align, T, true>
{
   BHO_MOVE_STATIC_ASSERT((alignment_of<T>::value == Align));
   typedef aligned_union<T, Len> type;
};

//End of search defaults to max_align_t
template<std::size_t Len, std::size_t Align>
struct aligned_next<Len, Align, max_align_t, false>
{   typedef aligned_union<max_align_t, Len> type;   };

//Now define a search list through types
#define BHO_MOVE_ALIGNED_NEXT_STEP(TYPE, NEXT_TYPE)\
   template<std::size_t Len, std::size_t Align>\
   struct aligned_next<Len, Align, TYPE, false>\
      : aligned_next<Len, Align, NEXT_TYPE, Align == alignment_of<NEXT_TYPE>::value>\
   {};\
   //
   BHO_MOVE_ALIGNED_NEXT_STEP(long double, max_align_t)
   BHO_MOVE_ALIGNED_NEXT_STEP(double, long double)
   #ifdef BHO_HAS_LONG_LONG
      BHO_MOVE_ALIGNED_NEXT_STEP(::bho::long_long_type, double)
      BHO_MOVE_ALIGNED_NEXT_STEP(long, ::bho::long_long_type)
   #else
      BHO_MOVE_ALIGNED_NEXT_STEP(long, double)
   #endif
   BHO_MOVE_ALIGNED_NEXT_STEP(int, long)
   BHO_MOVE_ALIGNED_NEXT_STEP(short, int)
   BHO_MOVE_ALIGNED_NEXT_STEP(char, short)
#undef BHO_MOVE_ALIGNED_NEXT_STEP

template<std::size_t Len, std::size_t Align>
struct aligned_storage_impl
   : aligned_next<Len, Align, char, Align == alignment_of<char>::value>
{};

#endif

template<std::size_t Len, std::size_t Align = alignment_of<max_align_t>::value>
struct aligned_storage
{
   //Sanity checks for input parameters
   BHO_MOVE_STATIC_ASSERT(Align > 0);

   //Sanity checks for output type
   typedef typename aligned_storage_impl<Len ? Len : 1, Align>::type type;
   static const std::size_t value = alignment_of<type>::value;
   BHO_MOVE_STATIC_ASSERT(value >= Align);
   BHO_MOVE_STATIC_ASSERT((value % Align) == 0);

   //Just in case someone instantiates aligned_storage
   //instead of aligned_storage::type (typical error).
   private:
   aligned_storage();
};

}  //namespace move_detail {
}  //namespace bho {

#include <asio2/bho/move/detail/config_end.hpp>

#endif   //#ifndef BHO_MOVE_DETAIL_TYPE_TRAITS_HPP
