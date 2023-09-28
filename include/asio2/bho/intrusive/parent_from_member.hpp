/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga  2010-2013
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////
#ifndef BHO_INTRUSIVE_PARENT_FROM_MEMBER_HPP
#define BHO_INTRUSIVE_PARENT_FROM_MEMBER_HPP

#include <asio2/bho/intrusive/detail/config_begin.hpp>
#include <asio2/bho/intrusive/detail/workaround.hpp>
#include <asio2/bho/intrusive/intrusive_fwd.hpp>

#include <asio2/bho/intrusive/detail/parent_from_member.hpp>

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

namespace bho {
namespace intrusive {

//! Given a pointer to a member and its corresponding pointer to data member,
//! this function returns the pointer of the parent containing that member.
//! Note: this function does not work with pointer to members that rely on
//! virtual inheritance.
template<class Parent, class Member>
BHO_INTRUSIVE_FORCEINLINE Parent *get_parent_from_member(Member *member, const Member Parent::* ptr_to_member) BHO_NOEXCEPT
{  return ::bho::intrusive::detail::parent_from_member(member, ptr_to_member);  }

//! Given a const pointer to a member and its corresponding const pointer to data member,
//! this function returns the const pointer of the parent containing that member.
//! Note: this function does not work with pointer to members that rely on
//! virtual inheritance.
template<class Parent, class Member>
BHO_INTRUSIVE_FORCEINLINE const Parent *get_parent_from_member(const Member *member, const Member Parent::* ptr_to_member) BHO_NOEXCEPT
{  return ::bho::intrusive::detail::parent_from_member(member, ptr_to_member);  }

}  //namespace intrusive {
}  //namespace bho {

#include <asio2/bho/intrusive/detail/config_end.hpp>

#endif   //#ifndef BHO_INTRUSIVE_PARENT_FROM_MEMBER_HPP
