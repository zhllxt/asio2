//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright 2007, 2008 Steven Watanabe, Joseph Gauterin, Niels Dekker
// (C) Copyright Ion Gaztanaga 2005-2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BHO_MOVE_ADL_MOVE_SWAP_HPP
#define BHO_MOVE_ADL_MOVE_SWAP_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif
#
#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

//Based on BHO.Core's swap.
//Many thanks to Steven Watanabe, Joseph Gauterin and Niels Dekker.
#include <cstddef> //for std::size_t
#include <asio2/bho/move/detail/workaround.hpp>  //forceinline

//Try to avoid including <algorithm>, as it's quite big
#if defined(_MSC_VER) && defined(BHO_DINKUMWARE_STDLIB)
   #include <utility>   //Dinkum libraries define std::swap in utility which is lighter than algorithm
#elif defined(BHO_GNU_STDLIB)
   //For non-GCC compilers, where GNUC version is not very reliable, or old GCC versions
   //use the good old stl_algobase header, which is quite lightweight
   #if !defined(BHO_GCC) || ((__GNUC__ < 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ < 3)))
      #include <bits/stl_algobase.h>
   #elif (__GNUC__ == 4) && (__GNUC_MINOR__ == 3)
      //In GCC 4.3 a tiny stl_move.h was created with swap and move utilities
      #include <bits/stl_move.h>
   #else
      //In GCC 4.4 stl_move.h was renamed to move.h
      #include <bits/move.h>
   #endif
#elif defined(_LIBCPP_VERSION)
   #include <type_traits>  //The initial import of libc++ defines std::swap and still there
#elif __cplusplus >= 201103L
   #include <utility>    //Fallback for C++ >= 2011
#else
   #include <algorithm>  //Fallback for C++98/03
#endif

#include <asio2/bho/move/utility_core.hpp> //for bho::move

#if !defined(BHO_MOVE_DOXYGEN_INVOKED)

#if defined(BHO_NO_CXX11_RVALUE_REFERENCES)
namespace bho_move_member_swap {

struct dont_care
{
   dont_care(...);
};

struct private_type
{
   static private_type p;
   private_type const &operator,(int) const;
};

typedef char yes_type;            
struct no_type{ char dummy[2]; }; 

template<typename T>
no_type is_private_type(T const &);

yes_type is_private_type(private_type const &);

template <typename Type>
class has_member_function_named_swap
{
   struct BaseMixin
   {
      void swap();
   };

   struct Base : public Type, public BaseMixin { Base(); };
   template <typename T, T t> class Helper{};

   template <typename U>
   static no_type deduce(U*, Helper<void (BaseMixin::*)(), &U::swap>* = 0);
   static yes_type deduce(...);

   public:
   static const bool value = sizeof(yes_type) == sizeof(deduce((Base*)(0)));
};

template<typename Fun, bool HasFunc>
struct has_member_swap_impl
{
   static const bool value = false;
};

template<typename Fun>
struct has_member_swap_impl<Fun, true>
{
   struct FunWrap : Fun
   {
      FunWrap();

      using Fun::swap;
      private_type swap(dont_care) const;
   };

   static Fun &declval_fun();
   static FunWrap declval_wrap();

   static bool const value =
      sizeof(no_type) == sizeof(is_private_type( (declval_wrap().swap(declval_fun()), 0)) );
};

template<typename Fun>
struct has_member_swap : public has_member_swap_impl
      <Fun, has_member_function_named_swap<Fun>::value>
{};

}  //namespace bho_move_member_swap

namespace bho_move_adl_swap{

template<class P1, class P2, bool = P1::value>
struct and_op_impl
{  static const bool value = false; };

template<class P1, class P2>
struct and_op_impl<P1, P2, true>
{  static const bool value = P2::value;   };

template<class P1, class P2>
struct and_op
   : and_op_impl<P1, P2>
{};

//////

template<class P1, class P2, bool = P1::value>
struct and_op_not_impl
{  static const bool value = false; };

template<class P1, class P2>
struct and_op_not_impl<P1, P2, true>
{  static const bool value = !P2::value;   };

template<class P1, class P2>
struct and_op_not
   : and_op_not_impl<P1, P2>
{};

template<class T>
BHO_MOVE_FORCEINLINE void swap_proxy(T& x, T& y, typename bho::move_detail::enable_if_c<!bho::move_detail::has_move_emulation_enabled_impl<T>::value>::type* = 0)
{
   //use std::swap if argument dependent lookup fails
   //Use using directive ("using namespace xxx;") instead as some older compilers
   //don't do ADL with using declarations ("using ns::func;").
   using namespace std;
   swap(x, y);
}

template<class T>
BHO_MOVE_FORCEINLINE void swap_proxy(T& x, T& y
               , typename bho::move_detail::enable_if< and_op_not_impl<bho::move_detail::has_move_emulation_enabled_impl<T>
                                                                        , bho_move_member_swap::has_member_swap<T> >
                                                       >::type* = 0)
{  T t(::bho::move(x)); x = ::bho::move(y); y = ::bho::move(t);  }

template<class T>
BHO_MOVE_FORCEINLINE void swap_proxy(T& x, T& y
               , typename bho::move_detail::enable_if< and_op_impl< bho::move_detail::has_move_emulation_enabled_impl<T>
                                                                    , bho_move_member_swap::has_member_swap<T> >
                                                       >::type* = 0)
{  x.swap(y);  }

}  //namespace bho_move_adl_swap{

#else

namespace bho_move_adl_swap{

template<class T>
BHO_MOVE_FORCEINLINE void swap_proxy(T& x, T& y)
{
   using std::swap;
   swap(x, y);
}

}  //namespace bho_move_adl_swap{

#endif   //#if defined(BHO_NO_CXX11_RVALUE_REFERENCES)

namespace bho_move_adl_swap{

template<class T, std::size_t N>
void swap_proxy(T (& x)[N], T (& y)[N])
{
   for (std::size_t i = 0; i < N; ++i){
      ::bho_move_adl_swap::swap_proxy(x[i], y[i]);
   }
}

}  //namespace bho_move_adl_swap {

#endif   //!defined(BHO_MOVE_DOXYGEN_INVOKED)

namespace bho{

//! Exchanges the values of a and b, using Argument Dependent Lookup (ADL) to select a
//! specialized swap function if available. If no specialized swap function is available,
//! std::swap is used.
//!
//! <b>Exception</b>: If T uses BHO.Move's move emulation and the compiler has
//! no rvalue references then:
//!
//!   -  If T has a <code>T::swap(T&)</code> member, that member is called.
//!   -  Otherwise a move-based swap is called, equivalent to: 
//!      <code>T t(::bho::move(x)); x = ::bho::move(y); y = ::bho::move(t);</code>.
template<class T>
BHO_MOVE_FORCEINLINE void adl_move_swap(T& x, T& y)
{
   ::bho_move_adl_swap::swap_proxy(x, y);
}

//! Exchanges elements between range [first1, last1) and another range starting at first2
//! using bho::adl_move_swap.
//! 
//! Parameters:
//!   first1, last1   -   the first range of elements to swap
//!   first2   -   beginning of the second range of elements to swap
//!
//! Type requirements:
//!   - ForwardIt1, ForwardIt2 must meet the requirements of ForwardIterator.
//!   - The types of dereferenced ForwardIt1 and ForwardIt2 must meet the
//!     requirements of Swappable
//!
//! Return value: Iterator to the element past the last element exchanged in the range
//! beginning with first2.
template<class ForwardIt1, class ForwardIt2>
ForwardIt2 adl_move_swap_ranges(ForwardIt1 first1, ForwardIt1 last1, ForwardIt2 first2)
{
    while (first1 != last1) {
      ::bho::adl_move_swap(*first1, *first2);
      ++first1;
      ++first2;
    }
   return first2;
}

template<class BidirIt1, class BidirIt2>
BidirIt2 adl_move_swap_ranges_backward(BidirIt1 first1, BidirIt1 last1, BidirIt2 last2)
{
   while (first1 != last1) {
      ::bho::adl_move_swap(*(--last1), *(--last2));
   }
   return last2;
}

template<class ForwardIt1, class ForwardIt2>
void adl_move_iter_swap(ForwardIt1 a, ForwardIt2 b)
{
   bho::adl_move_swap(*a, *b); 
}

}  //namespace bho{

#endif   //#ifndef BHO_MOVE_ADL_MOVE_SWAP_HPP
