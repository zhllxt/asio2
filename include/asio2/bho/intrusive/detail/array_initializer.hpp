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

#ifndef BHO_INTRUSIVE_DETAIL_ARRAY_INITIALIZER_HPP
#define BHO_INTRUSIVE_DETAIL_ARRAY_INITIALIZER_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/config.hpp>
#include <asio2/bho/intrusive/detail/workaround.hpp>
#include <asio2/bho/move/detail/placement_new.hpp>
#include <asio2/bho/move/detail/force_ptr.hpp>

namespace bho {
namespace intrusive {
namespace detail {

//This is not standard, but should work with all compilers
union max_align
{
   char        char_;
   short       short_;
   int         int_;
   long        long_;
   #ifdef BHO_HAS_LONG_LONG
   ::bho::long_long_type  long_long_;
   #endif
   float       float_;
   double      double_;
   long double long_double_;
   void *      void_ptr_;
};

template<class T, std::size_t N>
class array_initializer
{
   public:
   template<class CommonInitializer>
   array_initializer(const CommonInitializer &init)
   {
      char *init_buf = (char*)rawbuf;
      std::size_t i = 0;
      BHO_INTRUSIVE_TRY{
         for(; i != N; ++i){
            ::new(init_buf, bho_move_new_t()) T(init);
            init_buf += sizeof(T);
         }
      }
      BHO_INTRUSIVE_CATCH(...){
         while(i--){
            init_buf -= sizeof(T);
            move_detail::force_ptr<T*>(init_buf)->~T();
         }
         BHO_INTRUSIVE_RETHROW;
      }
      BHO_INTRUSIVE_CATCH_END
   }

   operator T* ()
   {  return (T*)(rawbuf);  }

   operator const T*() const
   {  return (const T*)(rawbuf);  }

   ~array_initializer()
   {
      char *init_buf = (char*)rawbuf + N*sizeof(T);
      for(std::size_t i = 0; i != N; ++i){
         init_buf -= sizeof(T);
         move_detail::force_ptr<T*>(init_buf)->~T();
      }
   }

   private:
   detail::max_align rawbuf[(N*sizeof(T)-1)/sizeof(detail::max_align)+1];
};

}  //namespace detail{
}  //namespace intrusive{
}  //namespace bho{

#endif //BHO_INTRUSIVE_DETAIL_ARRAY_INITIALIZER_HPP
