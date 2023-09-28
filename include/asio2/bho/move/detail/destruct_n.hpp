//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2015-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/move for documentation.
//
//////////////////////////////////////////////////////////////////////////////

//! \file

#ifndef BHO_MOVE_DETAIL_DESTRUCT_N_HPP
#define BHO_MOVE_DETAIL_DESTRUCT_N_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif
#
#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <cstddef>

namespace bho {
namespace movelib{

template<class T, class RandItUninit>
class destruct_n
{
   public:
   explicit destruct_n(RandItUninit raw)
      : m_ptr(raw), m_size()
   {}

   void incr()
   {
      ++m_size;
   }

   void incr(std::size_t n)
   {
      m_size += n;
   }

   void release()
   {
      m_size = 0u;
   }

   ~destruct_n()
   {
      while(m_size--){
         m_ptr[m_size].~T();
      }
   }
   private:
   RandItUninit m_ptr;
   std::size_t m_size;
};

}} //namespace bho {  namespace movelib{

#endif //#ifndef BHO_MOVE_DETAIL_DESTRUCT_N_HPP
