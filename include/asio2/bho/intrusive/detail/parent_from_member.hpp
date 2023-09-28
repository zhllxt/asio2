/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga  2007-2013
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////
#ifndef BHO_INTRUSIVE_DETAIL_PARENT_FROM_MEMBER_HPP
#define BHO_INTRUSIVE_DETAIL_PARENT_FROM_MEMBER_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/intrusive/detail/config_begin.hpp>
#include <asio2/bho/intrusive/detail/workaround.hpp>
#include <cstddef>

#if defined(_MSC_VER)
   #define BHO_INTRUSIVE_MSVC_ABI_PTR_TO_MEMBER
   #include <asio2/bho/static_assert.hpp>
#endif

namespace bho {
namespace intrusive {
namespace detail {

template<class Parent, class Member>
BHO_INTRUSIVE_FORCEINLINE std::ptrdiff_t offset_from_pointer_to_member(const Member Parent::* ptr_to_member)
{
   //The implementation of a pointer to member is compiler dependent.
   #if defined(BHO_INTRUSIVE_MSVC_ABI_PTR_TO_MEMBER)

   //MSVC compliant compilers use their the first 32 bits as offset (even in 64 bit mode)
   union caster_union
   {
      const Member Parent::* ptr_to_member;
      int offset;
   } caster;

   //MSVC ABI can use up to 3 int32 to represent pointer to member data
   //with virtual base classes, in those cases there is no simple to
   //obtain the address of the parent. So static assert to avoid runtime errors
   BHO_STATIC_ASSERT( sizeof(caster) == sizeof(int) );

   caster.ptr_to_member = ptr_to_member;
   return std::ptrdiff_t(caster.offset);
   //Additional info on MSVC behaviour for the future. For 2/3 int ptr-to-member
   //types dereference seems to be:
   //
   // vboffset = [compile_time_offset if 2-int ptr2memb] /
   //            [ptr2memb.i32[2] if 3-int ptr2memb].
   // vbtable = *(this + vboffset);
   // adj = vbtable[ptr2memb.i32[1]];
   // var = adj + (this + vboffset) + ptr2memb.i32[0];
   //
   //To reverse the operation we need to
   // - obtain vboffset (in 2-int ptr2memb implementation only)
   // - Go to Parent's vbtable and obtain adjustment at index ptr2memb.i32[1]
   // - parent = member - adj - vboffset - ptr2memb.i32[0]
   //
   //Even accessing to RTTI we might not be able to obtain this information
   //so anyone who thinks it's possible, please send a patch.

   //This works with gcc, msvc, ac++, ibmcpp
   #elif defined(__GNUC__)   || defined(__HP_aCC) || defined(BHO_INTEL) || \
         defined(__IBMCPP__) || defined(__DECCXX)
   const Parent * const parent = 0;
   const char *const member = static_cast<const char*>(static_cast<const void*>(&(parent->*ptr_to_member)));
   return std::ptrdiff_t(member - static_cast<const char*>(static_cast<const void*>(parent)));
   #else
   //This is the traditional C-front approach: __MWERKS__, __DMC__, __SUNPRO_CC
   union caster_union
   {
      const Member Parent::* ptr_to_member;
      std::ptrdiff_t offset;
   } caster;
   caster.ptr_to_member = ptr_to_member;
   return caster.offset - 1;
   #endif
}

template<class Parent, class Member>
BHO_INTRUSIVE_FORCEINLINE Parent *parent_from_member(Member *member, const Member Parent::* ptr_to_member)
{
   return static_cast<Parent*>
      (
         static_cast<void*>
         (
            static_cast<char*>(static_cast<void*>(member)) - offset_from_pointer_to_member(ptr_to_member)
         )
      );
}

template<class Parent, class Member>
BHO_INTRUSIVE_FORCEINLINE const Parent *parent_from_member(const Member *member, const Member Parent::* ptr_to_member)
{
   return static_cast<const Parent*>
      (
         static_cast<const void*>
         (
            static_cast<const char*>(static_cast<const void*>(member)) - offset_from_pointer_to_member(ptr_to_member)
         )
      );
}

}  //namespace detail {
}  //namespace intrusive {
}  //namespace bho {

#include <asio2/bho/intrusive/detail/config_end.hpp>

#endif   //#ifndef BHO_INTRUSIVE_DETAIL_PARENT_FROM_MEMBER_HPP
