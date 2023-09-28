/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga  2014-2014
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef BHO_INTRUSIVE_DETAIL_TWIN_HPP
#define BHO_INTRUSIVE_DETAIL_TWIN_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

//A tiny utility to avoid pulling std::pair / utility for
//very simple algorithms/types

namespace bho {
namespace intrusive {

template <class T>
struct twin
{
   typedef T type;
   twin()
      : first(), second()
   {}

   twin(const type &f, const type &s)
      : first(f), second(s)
   {}

   T first;
   T second;
};

}  //namespace intrusive{
}  //namespace bho{

#endif //BHO_INTRUSIVE_DETAIL_TWIN_HPP
