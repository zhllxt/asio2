/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga  2014-2014
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/move for documentation.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef BHO_MOVE_DETAIL_REVERSE_ITERATOR_HPP
#define BHO_MOVE_DETAIL_REVERSE_ITERATOR_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/move/detail/config_begin.hpp>
#include <asio2/bho/move/detail/iterator_traits.hpp>
#include <asio2/bho/move/detail/meta_utils.hpp>

namespace bho {
namespace movelib {

template<class I>
BHO_MOVE_FORCEINLINE typename iterator_traits<I>::pointer iterator_arrow_result(const I &i)
{  return i.operator->();  }

template<class T>
BHO_MOVE_FORCEINLINE T * iterator_arrow_result(T *p)
{  return p;   }

template<class It>
class reverse_iterator
{
   public:
   typedef typename bho::movelib::iterator_traits<It>::pointer             pointer;
   typedef typename bho::movelib::iterator_traits<It>::reference           reference;
   typedef typename bho::movelib::iterator_traits<It>::difference_type     difference_type;
   typedef typename bho::movelib::iterator_traits<It>::iterator_category   iterator_category;
   typedef typename bho::movelib::iterator_traits<It>::value_type          value_type;


   typedef It iterator_type;

   BHO_MOVE_FORCEINLINE reverse_iterator()
      : m_current()  //Value initialization to achieve "null iterators" (N3644)
   {}

   BHO_MOVE_FORCEINLINE explicit reverse_iterator(It r)
      : m_current(r)
   {}

   BHO_MOVE_FORCEINLINE reverse_iterator(const reverse_iterator& r)
      : m_current(r.base())
   {}

   template<class OtherIt>
   BHO_MOVE_FORCEINLINE
      reverse_iterator( const reverse_iterator<OtherIt>& r
                      , typename bho::move_detail::enable_if_convertible<OtherIt, It>::type* =0
                   )
      : m_current(r.base())
   {}

   BHO_MOVE_FORCEINLINE reverse_iterator & operator=( const reverse_iterator& r)
   {  m_current = r.base();   return *this;  }

   template<class OtherIt>
   BHO_MOVE_FORCEINLINE typename bho::move_detail::enable_if_convertible<OtherIt, It, reverse_iterator &>::type
         operator=( const reverse_iterator<OtherIt>& r)
   {  m_current = r.base();   return *this;  }

   BHO_MOVE_FORCEINLINE It base() const
   {  return m_current;  }

   BHO_MOVE_FORCEINLINE reference operator*() const
   {
      It temp(m_current);
      --temp;
      reference r = *temp;
      return r;
   }

   BHO_MOVE_FORCEINLINE pointer operator->() const
   {
      It temp(m_current);
      --temp;
      return (iterator_arrow_result)(temp);
   }

   BHO_MOVE_FORCEINLINE reference operator[](difference_type off) const
   {
      return this->m_current[difference_type(-off - 1)];
   }

   BHO_MOVE_FORCEINLINE reverse_iterator& operator++()
   {
      --m_current;
      return *this;
   }

   BHO_MOVE_FORCEINLINE reverse_iterator operator++(int)
   {
      reverse_iterator temp((*this));
      --m_current;
      return temp;
   }

   BHO_MOVE_FORCEINLINE reverse_iterator& operator--()
   {
      ++m_current;
      return *this;
   }

   BHO_MOVE_FORCEINLINE reverse_iterator operator--(int)
   {
      reverse_iterator temp((*this));
      ++m_current;
      return temp;
   }

   BHO_MOVE_FORCEINLINE friend bool operator==(const reverse_iterator& l, const reverse_iterator& r)
   {  return l.m_current == r.m_current;  }

   BHO_MOVE_FORCEINLINE friend bool operator!=(const reverse_iterator& l, const reverse_iterator& r)
   {  return l.m_current != r.m_current;  }

   BHO_MOVE_FORCEINLINE friend bool operator<(const reverse_iterator& l, const reverse_iterator& r)
   {  return l.m_current > r.m_current;  }

   BHO_MOVE_FORCEINLINE friend bool operator<=(const reverse_iterator& l, const reverse_iterator& r)
   {  return l.m_current >= r.m_current;  }

   BHO_MOVE_FORCEINLINE friend bool operator>(const reverse_iterator& l, const reverse_iterator& r)
   {  return l.m_current < r.m_current;  }

   BHO_MOVE_FORCEINLINE friend bool operator>=(const reverse_iterator& l, const reverse_iterator& r)
   {  return l.m_current <= r.m_current;  }

   BHO_MOVE_FORCEINLINE reverse_iterator& operator+=(difference_type off)
   {  m_current -= off; return *this;  }

   BHO_MOVE_FORCEINLINE reverse_iterator& operator-=(difference_type off)
   {  m_current += off; return *this;  }

   BHO_MOVE_FORCEINLINE friend reverse_iterator operator+(reverse_iterator l, difference_type off)
   {  return (l += off);  }

   BHO_MOVE_FORCEINLINE friend reverse_iterator operator+(difference_type off, reverse_iterator r)
   {  return (r += off);   }

   BHO_MOVE_FORCEINLINE friend reverse_iterator operator-(reverse_iterator l, difference_type off)
   {  return (l-= off);  }

   BHO_MOVE_FORCEINLINE friend difference_type operator-(const reverse_iterator& l, const reverse_iterator& r)
   {  return r.m_current - l.m_current;  }

   private:
   It m_current;   // the wrapped iterator
};

template< class Iterator >
BHO_MOVE_FORCEINLINE reverse_iterator<Iterator> make_reverse_iterator( Iterator i )
{  return reverse_iterator<Iterator>(i);  }

} //namespace movelib {
} //namespace bho {

#include <asio2/bho/move/detail/config_end.hpp>

#endif //BHO_MOVE_DETAIL_REVERSE_ITERATOR_HPP
