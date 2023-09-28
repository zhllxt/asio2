/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Joaquin M Lopez Munoz  2006-2013
// (C) Copyright Ion Gaztanaga          2014-2014
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef BHO_INTRUSIVE_DETAIL_EBO_HOLDER_HPP
#define BHO_INTRUSIVE_DETAIL_EBO_HOLDER_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/intrusive/detail/workaround.hpp>
#include <asio2/bho/move/utility_core.hpp>

namespace bho {
namespace intrusive {
namespace detail {

#if defined(BHO_MSVC) || defined(__BORLANDC_)
#define BHO_INTRUSIVE_TT_DECL __cdecl
#else
#define BHO_INTRUSIVE_TT_DECL
#endif

#if defined(_MSC_EXTENSIONS) && !defined(__BORLAND__) && !defined(_WIN64) && !defined(_M_ARM) && !defined(_M_ARM64) && !defined(UNDER_CE)
#define BHO_INTRUSIVE_TT_TEST_MSC_FUNC_SIGS
#endif

template <typename T>
struct is_unary_or_binary_function_impl
{  static const bool value = false; };

// see boost ticket #4094
// avoid duplicate definitions of is_unary_or_binary_function_impl
#ifndef BHO_INTRUSIVE_TT_TEST_MSC_FUNC_SIGS

template <typename R>
struct is_unary_or_binary_function_impl<R (*)()>
{  static const bool value = true;  };

template <typename R>
struct is_unary_or_binary_function_impl<R (*)(...)>
{  static const bool value = true;  };

#else // BHO_INTRUSIVE_TT_TEST_MSC_FUNC_SIGS

template <typename R>
struct is_unary_or_binary_function_impl<R (__stdcall*)()>
{  static const bool value = true;  };

#ifndef _MANAGED

template <typename R>
struct is_unary_or_binary_function_impl<R (__fastcall*)()>
{  static const bool value = true;  };

#endif

template <typename R>
struct is_unary_or_binary_function_impl<R (__cdecl*)()>
{  static const bool value = true;  };

template <typename R>
struct is_unary_or_binary_function_impl<R (__cdecl*)(...)>
{  static const bool value = true;  };

#endif

// see boost ticket #4094
// avoid duplicate definitions of is_unary_or_binary_function_impl
#ifndef BHO_INTRUSIVE_TT_TEST_MSC_FUNC_SIGS

template <typename R, class T0>
struct is_unary_or_binary_function_impl<R (*)(T0)>
{  static const bool value = true;  };

template <typename R, class T0>
struct is_unary_or_binary_function_impl<R (*)(T0...)>
{  static const bool value = true;  };

#else // BHO_INTRUSIVE_TT_TEST_MSC_FUNC_SIGS

template <typename R, class T0>
struct is_unary_or_binary_function_impl<R (__stdcall*)(T0)>
{  static const bool value = true;  };

#ifndef _MANAGED

template <typename R, class T0>
struct is_unary_or_binary_function_impl<R (__fastcall*)(T0)>
{  static const bool value = true;  };

#endif

template <typename R, class T0>
struct is_unary_or_binary_function_impl<R (__cdecl*)(T0)>
{  static const bool value = true;  };

template <typename R, class T0>
struct is_unary_or_binary_function_impl<R (__cdecl*)(T0...)>
{  static const bool value = true;  };

#endif

// see boost ticket #4094
// avoid duplicate definitions of is_unary_or_binary_function_impl
#ifndef BHO_INTRUSIVE_TT_TEST_MSC_FUNC_SIGS

template <typename R, class T0, class T1>
struct is_unary_or_binary_function_impl<R (*)(T0, T1)>
{  static const bool value = true;  };

template <typename R, class T0, class T1>
struct is_unary_or_binary_function_impl<R (*)(T0, T1...)>
{  static const bool value = true;  };

#else // BHO_INTRUSIVE_TT_TEST_MSC_FUNC_SIGS

template <typename R, class T0, class T1>
struct is_unary_or_binary_function_impl<R (__stdcall*)(T0, T1)>
{  static const bool value = true;  };

#ifndef _MANAGED

template <typename R, class T0, class T1>
struct is_unary_or_binary_function_impl<R (__fastcall*)(T0, T1)>
{  static const bool value = true;  };

#endif

template <typename R, class T0, class T1>
struct is_unary_or_binary_function_impl<R (__cdecl*)(T0, T1)>
{  static const bool value = true;  };

template <typename R, class T0, class T1>
struct is_unary_or_binary_function_impl<R (__cdecl*)(T0, T1...)>
{  static const bool value = true;  };
#endif

template <typename T>
struct is_unary_or_binary_function_impl<T&>
{  static const bool value = false; };

template<typename T>
struct is_unary_or_binary_function : is_unary_or_binary_function_impl<T>
{};

template<typename T, typename Tag = void, bool = is_unary_or_binary_function<T>::value>
class ebo_functor_holder
{
   BHO_COPYABLE_AND_MOVABLE(ebo_functor_holder)

   public:
   typedef T functor_type;

   BHO_INTRUSIVE_FORCEINLINE ebo_functor_holder()
      : t_()
   {}

   BHO_INTRUSIVE_FORCEINLINE explicit ebo_functor_holder(const T &t)
      : t_(t)
   {}

   BHO_INTRUSIVE_FORCEINLINE explicit ebo_functor_holder(BHO_RV_REF(T) t)
      : t_(::bho::move(t))
   {}

   template<class Arg1, class Arg2>
   BHO_INTRUSIVE_FORCEINLINE ebo_functor_holder(BHO_FWD_REF(Arg1) arg1, BHO_FWD_REF(Arg2) arg2)
      : t_(::bho::forward<Arg1>(arg1), ::bho::forward<Arg2>(arg2))
   {}

   BHO_INTRUSIVE_FORCEINLINE ebo_functor_holder(const ebo_functor_holder &x)
      : t_(x.t_)
   {}

   BHO_INTRUSIVE_FORCEINLINE ebo_functor_holder(BHO_RV_REF(ebo_functor_holder) x)
      : t_(x.t_)
   {}

   BHO_INTRUSIVE_FORCEINLINE ebo_functor_holder& operator=(BHO_COPY_ASSIGN_REF(ebo_functor_holder) x)
   {
      this->get() = x.get();
      return *this;
   }

   BHO_INTRUSIVE_FORCEINLINE ebo_functor_holder& operator=(BHO_RV_REF(ebo_functor_holder) x)
   {
      this->get() = ::bho::move(x.get());
      return *this;
   }

   BHO_INTRUSIVE_FORCEINLINE ebo_functor_holder& operator=(const T &x)
   {
      this->get() = x;
      return *this;
   }

   BHO_INTRUSIVE_FORCEINLINE ebo_functor_holder& operator=(BHO_RV_REF(T) x)
   {
      this->get() = ::bho::move(x);
      return *this;
   }

   BHO_INTRUSIVE_FORCEINLINE T&       get(){return t_;}
   BHO_INTRUSIVE_FORCEINLINE const T& get()const{return t_;}

   private:
   T t_;
};

template<typename T, typename Tag>
class ebo_functor_holder<T, Tag, false>
   :  public T
{
   BHO_COPYABLE_AND_MOVABLE(ebo_functor_holder)

   public:
   typedef T functor_type;

   BHO_INTRUSIVE_FORCEINLINE ebo_functor_holder()
      : T()
   {}

   BHO_INTRUSIVE_FORCEINLINE explicit ebo_functor_holder(const T &t)
      : T(t)
   {}

   BHO_INTRUSIVE_FORCEINLINE explicit ebo_functor_holder(BHO_RV_REF(T) t)
      : T(::bho::move(t))
   {}

   template<class Arg1, class Arg2>
   BHO_INTRUSIVE_FORCEINLINE ebo_functor_holder(BHO_FWD_REF(Arg1) arg1, BHO_FWD_REF(Arg2) arg2)
      : T(::bho::forward<Arg1>(arg1), ::bho::forward<Arg2>(arg2))
   {}

   BHO_INTRUSIVE_FORCEINLINE ebo_functor_holder(const ebo_functor_holder &x)
      : T(static_cast<const T&>(x))
   {}

   BHO_INTRUSIVE_FORCEINLINE ebo_functor_holder(BHO_RV_REF(ebo_functor_holder) x)
      : T(BHO_MOVE_BASE(T, x))
   {}

   BHO_INTRUSIVE_FORCEINLINE ebo_functor_holder& operator=(BHO_COPY_ASSIGN_REF(ebo_functor_holder) x)
   {
      const ebo_functor_holder&r = x;
      this->get() = r;
      return *this;
   }

   BHO_INTRUSIVE_FORCEINLINE ebo_functor_holder& operator=(BHO_RV_REF(ebo_functor_holder) x)
   {
      this->get() = ::bho::move(x.get());
      return *this;
   }

   BHO_INTRUSIVE_FORCEINLINE ebo_functor_holder& operator=(const T &x)
   {
      this->get() = x;
      return *this;
   }

   BHO_INTRUSIVE_FORCEINLINE ebo_functor_holder& operator=(BHO_RV_REF(T) x)
   {
      this->get() = ::bho::move(x);
      return *this;
   }

   BHO_INTRUSIVE_FORCEINLINE T&       get(){return *this;}
   BHO_INTRUSIVE_FORCEINLINE const T& get()const{return *this;}
};

}  //namespace detail {
}  //namespace intrusive {
}  //namespace bho {

#endif   //#ifndef BHO_INTRUSIVE_DETAIL_EBO_HOLDER_HPP
