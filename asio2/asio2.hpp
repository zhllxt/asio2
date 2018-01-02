/*
 * COPYRIGHT (C) 2017, zhllxt
 * 
 * version	: 1.1
 * 
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
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


// if ASIO2_WRITE_LOG is defined,when important exception occured,it will print the exception msg.
#define ASIO2_WRITE_LOG


#if defined(__GNUC__) || defined(__GNUG__)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wconversion"
#endif


#include <asio2/server.hpp>
#include <asio2/client.hpp>
#include <asio2/sender.hpp>


#if defined(__GNUC__) || defined(__GNUG__)
#	pragma GCC diagnostic pop
#endif



#endif // !__ASIO2_HPP__
