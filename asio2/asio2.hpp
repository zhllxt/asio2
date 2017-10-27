/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 *
 */

 /*
  * 
  * if you find some bugs,or have any troubles or questions on using this library,please contact me.
  * 
  */

#ifndef __ASIO2_HPP__
#define __ASIO2_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if !defined(NDEBUG) && !defined(DEBUG) && !defined(_DEBUG)
#	define NDEBUG
#endif


// if PRINT_FATAL_EXCEPTION is defined,when important exception occured,it will print the exception msg.if you don't 
// want print the exception msg,you can use #undef PRINT_FATAL_EXCEPTION after include <asio2/asio2.hpp> to avoid it.
#define PRINT_FATAL_EXCEPTION



#if defined(__GNUC__) || defined(__GNUG__)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wconversion"
#endif



#include <asio2/util/pool.hpp>
#include <asio2/util/object_pool.hpp>
#include <asio2/util/rwlock.hpp>
#include <asio2/util/spin_lock.hpp>
#include <asio2/util/helper.hpp>
#include <asio2/util/thread_pool.hpp>

#include <asio2/server.hpp>
#include <asio2/client.hpp>
#include <asio2/sender.hpp>



#if defined(__GNUC__) || defined(__GNUG__)
#	pragma GCC diagnostic pop
#endif



#endif // !__ASIO2_HPP__
