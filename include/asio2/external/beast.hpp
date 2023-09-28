/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_BEAST_HPP__
#define __ASIO2_BEAST_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/config.hpp>

#include <asio2/base/detail/push_options.hpp>

#ifdef ASIO2_HEADER_ONLY
#ifndef BEAST_HEADER_ONLY
#define BEAST_HEADER_ONLY 1
#endif
#endif

#ifdef ASIO2_HEADER_ONLY
	#include <asio2/bho/beast.hpp>
	#if defined(ASIO2_ENABLE_SSL) || defined(ASIO2_USE_SSL)
		// boost 1.72(107200) BOOST_BEAST_VERSION 277
		#if defined(BEAST_VERSION) && (BEAST_VERSION >= 277)
			#include <asio2/bho/beast/ssl.hpp>
		#endif
		#include <asio2/bho/beast/websocket/ssl.hpp>
	#endif
	#ifndef BEAST_VERSION
	#define BEAST_VERSION BHO_BEAST_VERSION
	#endif
	#ifndef BEAST_VERSION_STRING
	#define BEAST_VERSION_STRING BHO_BEAST_VERSION_STRING
	#endif
	#ifndef BHO_BEAST_VERSION
	#define BHO_BEAST_VERSION BHO_BEAST_VERSION
	#endif
	#ifndef BHO_BEAST_VERSION_STRING
	#define BHO_BEAST_VERSION_STRING BHO_BEAST_VERSION_STRING
	#endif
#else
	#ifndef BOOST_BEAST_USE_STD_STRING_VIEW
	#define BOOST_BEAST_USE_STD_STRING_VIEW
	#endif
	#include <boost/beast.hpp>
	#if defined(ASIO2_ENABLE_SSL) || defined(ASIO2_USE_SSL)
		// boost 1.72(107200) BOOST_BEAST_VERSION 277
		#if defined(BOOST_BEAST_VERSION) && (BOOST_BEAST_VERSION >= 277)
			#include <boost/beast/ssl.hpp>
		#endif
		#include <boost/beast/websocket/ssl.hpp>
	#endif
	#ifndef BEAST_VERSION
	#define BEAST_VERSION BOOST_BEAST_VERSION
	#endif
	#ifndef BEAST_VERSION_STRING
	#define BEAST_VERSION_STRING BOOST_BEAST_VERSION_STRING
	#endif
	#ifndef BHO_BEAST_VERSION
	#define BHO_BEAST_VERSION BOOST_BEAST_VERSION
	#endif
	#ifndef BHO_BEAST_VERSION_STRING
	#define BHO_BEAST_VERSION_STRING BOOST_BEAST_VERSION_STRING
	#endif
#endif

#ifdef ASIO2_HEADER_ONLY
	namespace bho::beast::http
	{
		template<class Body, class Fields = fields>
		using request_t = request<Body, Fields>;

		template<class Body, class Fields = fields>
		using response_t = response<Body, Fields>;
	}
	namespace beast = ::bho::beast;
#else
	namespace boost::beast::http
	{
		template<class Body, class Fields = fields>
		using request_t = request<Body, Fields>;

		template<class Body, class Fields = fields>
		using response_t = response<Body, Fields>;
	}
	namespace beast = ::boost::beast;
	namespace asio  = ::boost::asio;
	namespace bho   = ::boost; // bho means boost header only
#endif

namespace http      = ::beast::http;
namespace websocket = ::beast::websocket;

namespace asio2
{
	namespace http      = ::beast::http;
	namespace websocket = ::beast::websocket;
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_BEAST_HPP__
