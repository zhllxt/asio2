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

#ifndef BHO_INTRUSIVE_DETAIL_EXCEPTION_DISPOSER_HPP
#define BHO_INTRUSIVE_DETAIL_EXCEPTION_DISPOSER_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/intrusive/detail/workaround.hpp>

namespace bho {
namespace intrusive {
namespace detail {

template<class Container, class Disposer>
class exception_disposer
{
   Container *cont_;
   Disposer  &disp_;

   exception_disposer(const exception_disposer&);
   exception_disposer &operator=(const exception_disposer&);

   public:
   exception_disposer(Container &cont, Disposer &disp)
      :  cont_(&cont), disp_(disp)
   {}

   BHO_INTRUSIVE_FORCEINLINE void release()
   {  cont_ = 0;  }

   ~exception_disposer()
   {
      if(cont_){
         cont_->clear_and_dispose(disp_);
      }
   }
};

}  //namespace detail{
}  //namespace intrusive{
}  //namespace bho{

#endif //BHO_INTRUSIVE_DETAIL_EXCEPTION_DISPOSER_HPP
