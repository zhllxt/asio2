/*
 * COPYRIGHT (C) 2017, zhllxt
 * 
 * date     : 2018-01-24
 * version  : 1.3
 * 
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 * current asio version is 1.11.0 
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


// ASIO2_VERSION / 100 is the major version
// ASIO2_VERSION % 100 is the minor version
#define ASIO2_VERSION 104 // 1.4


// if ASIO2_DUMP_EXCEPTION_LOG is defined,when important exception occured,it will print and dump the exception msg.
#define ASIO2_DUMP_EXCEPTION_LOG


#if defined(__GNUC__) || defined(__GNUG__)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wconversion"
#endif


#include <asio2/util/buffer.hpp>
#include <asio2/util/def.hpp>
#include <asio2/util/helper.hpp>
#include <asio2/util/ini.hpp>
#include <asio2/util/logger.hpp>
#include <asio2/util/pool.hpp>
#include <asio2/util/rwlock.hpp>
#include <asio2/util/spin_lock.hpp>
#include <asio2/util/thread_pool.hpp>

#include <asio2/server.hpp>
#include <asio2/client.hpp>
#include <asio2/sender.hpp>


#if defined(__GNUC__) || defined(__GNUG__)
#	pragma GCC diagnostic pop
#endif



#endif // !__ASIO2_HPP__
