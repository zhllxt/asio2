//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2014-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/move for documentation.
//
//////////////////////////////////////////////////////////////////////////////

//! \file

#ifndef BHO_MOVE_DETAIL_ITERATOR_TRAITS_HPP
#define BHO_MOVE_DETAIL_ITERATOR_TRAITS_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif
#
#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <cstddef>
#include <asio2/bho/move/detail/type_traits.hpp>

#include <asio2/bho/move/detail/std_ns_begin.hpp>
BHO_MOVE_STD_NS_BEG

struct input_iterator_tag;
struct forward_iterator_tag;
struct bidirectional_iterator_tag;
struct random_access_iterator_tag;
struct output_iterator_tag;
struct contiguous_iterator_tag;

BHO_MOVE_STD_NS_END
#include <asio2/bho/move/detail/std_ns_end.hpp>

namespace bho{  namespace movelib{

template<class Iterator>
struct iterator_traits
{
   typedef typename Iterator::difference_type   difference_type;
   typedef typename Iterator::value_type        value_type;
   typedef typename Iterator::pointer           pointer;
   typedef typename Iterator::reference         reference;
   typedef typename Iterator::iterator_category iterator_category;
   typedef typename bho::move_detail::make_unsigned<difference_type>::type size_type;
};

template<class T>
struct iterator_traits<T*>
{
   typedef std::ptrdiff_t                    difference_type;
   typedef T                                 value_type;
   typedef T*                                pointer;
   typedef T&                                reference;
   typedef std::random_access_iterator_tag   iterator_category;
   typedef typename bho::move_detail::make_unsigned<difference_type>::type size_type;
};

template<class T>
struct iterator_traits<const T*>
{
   typedef std::ptrdiff_t                    difference_type;
   typedef T                                 value_type;
   typedef const T*                          pointer;
   typedef const T&                          reference;
   typedef std::random_access_iterator_tag   iterator_category;
   typedef typename bho::move_detail::make_unsigned<difference_type>::type size_type;
};

}} //namespace bho {  namespace movelib{

#endif //#ifndef BHO_MOVE_DETAIL_ITERATOR_TRAITS_HPP
