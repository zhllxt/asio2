//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Pablo Halpern 2009. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2011-2014. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BHO_INTRUSIVE_POINTER_TRAITS_HPP
#define BHO_INTRUSIVE_POINTER_TRAITS_HPP

#include <asio2/bho/intrusive/detail/config_begin.hpp>
#include <asio2/bho/intrusive/detail/workaround.hpp>
#include <asio2/bho/intrusive/pointer_rebind.hpp>
#include <asio2/bho/move/detail/pointer_element.hpp>
#include <asio2/bho/intrusive/detail/mpl.hpp>
#include <cstddef>

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

namespace bho {
namespace intrusive {
namespace detail {

#if !defined(BHO_MSVC) || (BHO_MSVC > 1310)
BHO_INTRUSIVE_HAS_STATIC_MEMBER_FUNC_SIGNATURE(has_member_function_callable_with_pointer_to, pointer_to)
BHO_INTRUSIVE_HAS_STATIC_MEMBER_FUNC_SIGNATURE(has_member_function_callable_with_dynamic_cast_from, dynamic_cast_from)
BHO_INTRUSIVE_HAS_STATIC_MEMBER_FUNC_SIGNATURE(has_member_function_callable_with_static_cast_from, static_cast_from)
BHO_INTRUSIVE_HAS_STATIC_MEMBER_FUNC_SIGNATURE(has_member_function_callable_with_const_cast_from, const_cast_from)
#else
BHO_INTRUSIVE_HAS_MEMBER_FUNC_CALLED_IGNORE_SIGNATURE(has_member_function_callable_with_pointer_to, pointer_to)
BHO_INTRUSIVE_HAS_MEMBER_FUNC_CALLED_IGNORE_SIGNATURE(has_member_function_callable_with_dynamic_cast_from, dynamic_cast_from)
BHO_INTRUSIVE_HAS_MEMBER_FUNC_CALLED_IGNORE_SIGNATURE(has_member_function_callable_with_static_cast_from, static_cast_from)
BHO_INTRUSIVE_HAS_MEMBER_FUNC_CALLED_IGNORE_SIGNATURE(has_member_function_callable_with_const_cast_from, const_cast_from)
#endif

BHO_INTRUSIVE_INSTANTIATE_EVAL_DEFAULT_TYPE_TMPLT(element_type)
BHO_INTRUSIVE_INSTANTIATE_DEFAULT_TYPE_TMPLT(difference_type)
BHO_INTRUSIVE_INSTANTIATE_DEFAULT_TYPE_TMPLT(size_type)
BHO_INTRUSIVE_INSTANTIATE_DEFAULT_TYPE_TMPLT(reference)
BHO_INTRUSIVE_INSTANTIATE_DEFAULT_TYPE_TMPLT(value_traits_ptr)

}  //namespace detail {


//! pointer_traits is the implementation of C++11 std::pointer_traits class with some
//! extensions like castings.
//!
//! pointer_traits supplies a uniform interface to certain attributes of pointer-like types.
//!
//! <b>Note</b>: When defining a custom family of pointers or references to be used with BI
//! library, make sure the public static conversion functions accessed through
//! the `pointer_traits` interface (`*_cast_from` and `pointer_to`) can
//! properly convert between const and nonconst referred member types
//! <b>without the use of implicit constructor calls</b>. It is suggested these
//! conversions be implemented as function templates, where the template
//! argument is the type of the object being converted from.
template <typename Ptr>
struct pointer_traits
{
   #ifdef BHO_INTRUSIVE_DOXYGEN_INVOKED
      //!The pointer type
      //!queried by this pointer_traits instantiation
      typedef Ptr             pointer;

      //!Ptr::element_type if such a type exists; otherwise, T if Ptr is a class
      //!template instantiation of the form SomePointer<T, Args>, where Args is zero or
      //!more type arguments ; otherwise , the specialization is ill-formed.
      typedef unspecified_type element_type;

      //!Ptr::difference_type if such a type exists; otherwise,
      //!std::ptrdiff_t.
      typedef unspecified_type difference_type;

      //!Ptr::rebind<U> if such a type exists; otherwise, SomePointer<U, Args> if Ptr is
      //!a class template instantiation of the form SomePointer<T, Args>, where Args is zero or
      //!more type arguments ; otherwise, the instantiation of rebind is ill-formed.
      //!
      //!For portable code for C++03 and C++11, <pre>typename rebind_pointer<U>::type</pre>
      //!shall be used instead of rebind<U> to obtain a pointer to U.
      template <class U> using rebind = unspecified;

      //!Ptr::reference if such a type exists (non-standard extension); otherwise, element_type &
      //!
      typedef unspecified_type reference;
   #else
      typedef Ptr                                                             pointer;
      //
      typedef BHO_INTRUSIVE_OBTAIN_TYPE_WITH_EVAL_DEFAULT
         ( bho::intrusive::detail::, Ptr, element_type
         , bho::movelib::detail::first_param<Ptr>)                          element_type;
      //
      typedef BHO_INTRUSIVE_OBTAIN_TYPE_WITH_DEFAULT
         (bho::intrusive::detail::, Ptr, difference_type, std::ptrdiff_t)   difference_type;

      typedef BHO_INTRUSIVE_OBTAIN_TYPE_WITH_DEFAULT
         ( bho::intrusive::detail::, Ptr, size_type
         , typename bho::move_detail::
               make_unsigned<difference_type>::type)                          size_type;

      typedef BHO_INTRUSIVE_OBTAIN_TYPE_WITH_DEFAULT
         ( bho::intrusive::detail::, Ptr, reference
         , typename bho::intrusive::detail::unvoid_ref<element_type>::type) reference;
      //
      template <class U> struct rebind_pointer
      {
         typedef typename bho::intrusive::pointer_rebind<Ptr, U>::type  type;
      };

      #if !defined(BHO_NO_CXX11_TEMPLATE_ALIASES)
         template <class U> using rebind = typename bho::intrusive::pointer_rebind<Ptr, U>::type;
      #endif
   #endif   //#if !defined(BHO_NO_CXX11_TEMPLATE_ALIASES)

   //! <b>Remark</b>: If element_type is (possibly cv-qualified) void, r type is unspecified; otherwise,
   //!   it is element_type &.
   //!
   //! <b>Returns</b>: A dereferenceable pointer to r obtained by calling Ptr::pointer_to(reference).
   //!   Non-standard extension: If such function does not exist, returns pointer(addressof(r));
   //!
   //! <b>Note</b>: For non-conforming compilers only the existence of a member function called
   //!   <code>pointer_to</code> is checked.
   BHO_INTRUSIVE_FORCEINLINE static pointer pointer_to(reference r) BHO_NOEXCEPT
   {
      //Non-standard extension, it does not require Ptr::pointer_to. If not present
      //tries to converts &r to pointer.
      const bool value = bho::intrusive::detail::
         has_member_function_callable_with_pointer_to
            <Ptr, Ptr (*)(reference)>::value;
      bho::intrusive::detail::bool_<value> flag;
      return pointer_traits::priv_pointer_to(flag, r);
   }

   //! <b>Remark</b>: Non-standard extension.
   //!
   //! <b>Returns</b>: A dereferenceable pointer to r obtained by calling the static template function
   //!   Ptr::static_cast_from(UPpr/const UPpr &).
   //!   If such function does not exist, returns pointer_to(static_cast<element_type&>(*uptr))
   //!
   //! <b>Note</b>: For non-conforming compilers only the existence of a member function called
   //!   <code>static_cast_from</code> is checked.
   template<class UPtr>
   BHO_INTRUSIVE_FORCEINLINE static pointer static_cast_from(const UPtr &uptr) BHO_NOEXCEPT
   {
      typedef const UPtr &RefArg;
      const bool value = bho::intrusive::detail::
         has_member_function_callable_with_static_cast_from
            <pointer, pointer(*)(RefArg)>::value
         || bho::intrusive::detail::
               has_member_function_callable_with_static_cast_from
                  <pointer, pointer(*)(UPtr)>::value;
      return pointer_traits::priv_static_cast_from(bho::intrusive::detail::bool_<value>(), uptr);
   }

   //! <b>Remark</b>: Non-standard extension.
   //!
   //! <b>Returns</b>: A dereferenceable pointer to r obtained by calling the static template function
   //!   Ptr::const_cast_from<UPtr>(UPpr/const UPpr &).
   //!   If such function does not exist, returns pointer_to(const_cast<element_type&>(*uptr))
   //!
   //! <b>Note</b>: For non-conforming compilers only the existence of a member function called
   //!   <code>const_cast_from</code> is checked.
   template<class UPtr>
   BHO_INTRUSIVE_FORCEINLINE static pointer const_cast_from(const UPtr &uptr) BHO_NOEXCEPT
   {
      typedef const UPtr &RefArg;
      const bool value = bho::intrusive::detail::
         has_member_function_callable_with_const_cast_from
            <pointer, pointer(*)(RefArg)>::value
         || bho::intrusive::detail::
               has_member_function_callable_with_const_cast_from
                  <pointer, pointer(*)(UPtr)>::value;
      return pointer_traits::priv_const_cast_from(bho::intrusive::detail::bool_<value>(), uptr);
   }

   //! <b>Remark</b>: Non-standard extension.
   //!
   //! <b>Returns</b>: A dereferenceable pointer to r obtained by calling the static template function
   //!   Ptr::dynamic_cast_from<UPtr>(UPpr/const UPpr &).
   //!   If such function does not exist, returns pointer_to(*dynamic_cast<element_type*>(&*uptr))
   //!
   //! <b>Note</b>: For non-conforming compilers only the existence of a member function called
   //!   <code>dynamic_cast_from</code> is checked.
   template<class UPtr>
   BHO_INTRUSIVE_FORCEINLINE static pointer dynamic_cast_from(const UPtr &uptr) BHO_NOEXCEPT
   {
      typedef const UPtr &RefArg;
      const bool value = bho::intrusive::detail::
         has_member_function_callable_with_dynamic_cast_from
            <pointer, pointer(*)(RefArg)>::value
         || bho::intrusive::detail::
               has_member_function_callable_with_dynamic_cast_from
                  <pointer, pointer(*)(UPtr)>::value;
      return pointer_traits::priv_dynamic_cast_from(bho::intrusive::detail::bool_<value>(), uptr);
   }

   ///@cond
   private:
   //priv_to_raw_pointer
   template <class T>
   BHO_INTRUSIVE_FORCEINLINE static T* to_raw_pointer(T* p) BHO_NOEXCEPT
   {  return p; }

   template <class Pointer>
   BHO_INTRUSIVE_FORCEINLINE static typename pointer_traits<Pointer>::element_type*
      to_raw_pointer(const Pointer &p) BHO_NOEXCEPT
   {  return pointer_traits::to_raw_pointer(p.operator->());  }

   //priv_pointer_to
   BHO_INTRUSIVE_FORCEINLINE static pointer priv_pointer_to(bho::intrusive::detail::true_, reference r) BHO_NOEXCEPT
   { return Ptr::pointer_to(r); }

   BHO_INTRUSIVE_FORCEINLINE static pointer priv_pointer_to(bho::intrusive::detail::false_, reference r) BHO_NOEXCEPT
   { return pointer(bho::intrusive::detail::addressof(r)); }

   //priv_static_cast_from
   template<class UPtr>
   BHO_INTRUSIVE_FORCEINLINE static pointer priv_static_cast_from(bho::intrusive::detail::true_, const UPtr &uptr) BHO_NOEXCEPT
   { return Ptr::static_cast_from(uptr); }

   template<class UPtr>
   BHO_INTRUSIVE_FORCEINLINE static pointer priv_static_cast_from(bho::intrusive::detail::false_, const UPtr &uptr) BHO_NOEXCEPT
   {  return uptr ? pointer_to(*static_cast<element_type*>(to_raw_pointer(uptr))) : pointer();  }

   //priv_const_cast_from
   template<class UPtr>
   BHO_INTRUSIVE_FORCEINLINE static pointer priv_const_cast_from(bho::intrusive::detail::true_, const UPtr &uptr) BHO_NOEXCEPT
   { return Ptr::const_cast_from(uptr); }

   template<class UPtr>
   BHO_INTRUSIVE_FORCEINLINE static pointer priv_const_cast_from(bho::intrusive::detail::false_, const UPtr &uptr) BHO_NOEXCEPT
   {  return uptr ? pointer_to(const_cast<element_type&>(*uptr)) : pointer();  }

   //priv_dynamic_cast_from
   template<class UPtr>
   BHO_INTRUSIVE_FORCEINLINE static pointer priv_dynamic_cast_from(bho::intrusive::detail::true_, const UPtr &uptr) BHO_NOEXCEPT
   { return Ptr::dynamic_cast_from(uptr); }

   template<class UPtr>
   BHO_INTRUSIVE_FORCEINLINE static pointer priv_dynamic_cast_from(bho::intrusive::detail::false_, const UPtr &uptr) BHO_NOEXCEPT
   {  return uptr ? pointer_to(dynamic_cast<element_type&>(*uptr)) : pointer();  }
   ///@endcond
};

///@cond

// Remove cv qualification from Ptr parameter to pointer_traits:
template <typename Ptr>
struct pointer_traits<const Ptr> : pointer_traits<Ptr> {};
template <typename Ptr>
struct pointer_traits<volatile Ptr> : pointer_traits<Ptr> { };
template <typename Ptr>
struct pointer_traits<const volatile Ptr> : pointer_traits<Ptr> { };
// Remove reference from Ptr parameter to pointer_traits:
template <typename Ptr>
struct pointer_traits<Ptr&> : pointer_traits<Ptr> { };

///@endcond

//! Specialization of pointer_traits for raw pointers
//!
template <typename T>
struct pointer_traits<T*>
{
   typedef T               element_type;
   typedef T*              pointer;
   typedef std::ptrdiff_t  difference_type;
   typedef std::size_t     size_type;

   #ifdef BHO_INTRUSIVE_DOXYGEN_INVOKED
      typedef T &          reference;
      //!typedef for <pre>U *</pre>
      //!
      //!For portable code for C++03 and C++11, <pre>typename rebind_pointer<U>::type</pre>
      //!shall be used instead of rebind<U> to obtain a pointer to U.
      template <class U> using rebind = U*;
   #else
      typedef typename bho::intrusive::detail::unvoid_ref<element_type>::type reference;
      #if !defined(BHO_NO_CXX11_TEMPLATE_ALIASES)
         template <class U> using rebind = U*;
      #endif
   #endif

   template <class U> struct rebind_pointer
   {  typedef U* type;  };

   //! <b>Returns</b>: addressof(r)
   //!
   BHO_INTRUSIVE_FORCEINLINE static pointer pointer_to(reference r) BHO_NOEXCEPT
   { return bho::intrusive::detail::addressof(r); }

   //! <b>Returns</b>: static_cast<pointer>(uptr)
   //!
   template<class U>
   BHO_INTRUSIVE_FORCEINLINE static pointer static_cast_from(U *uptr) BHO_NOEXCEPT
   {  return static_cast<pointer>(uptr);  }

   //! <b>Returns</b>: const_cast<pointer>(uptr)
   //!
   template<class U>
   BHO_INTRUSIVE_FORCEINLINE static pointer const_cast_from(U *uptr) BHO_NOEXCEPT
   {  return const_cast<pointer>(uptr);  }

   //! <b>Returns</b>: dynamic_cast<pointer>(uptr)
   //!
   template<class U>
   BHO_INTRUSIVE_FORCEINLINE static pointer dynamic_cast_from(U *uptr) BHO_NOEXCEPT
   {  return dynamic_cast<pointer>(uptr);  }
};

}  //namespace container {
}  //namespace bho {

#include <asio2/bho/intrusive/detail/config_end.hpp>

#endif // ! defined(BHO_INTRUSIVE_POINTER_TRAITS_HPP)
