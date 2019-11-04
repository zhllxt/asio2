/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_SELECTOR_HPP__
#define __ASIO2_SELECTOR_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/config.hpp>

#ifdef ASIO_STANDALONE
	#include <asio/asio.hpp>
	#if defined(ASIO2_USE_SSL)
		#include <asio/ssl.hpp>
	#endif
#else
	#include <boost/asio.hpp>
	#include <boost/beast.hpp>
	#if defined(ASIO2_USE_SSL)
		#include <boost/asio/ssl.hpp>
		#include <boost/beast/websocket/ssl.hpp>
	#endif
	#ifndef ASIO_VERSION
		#define ASIO_VERSION BOOST_ASIO_VERSION
	#endif
#endif // ASIO_STANDALONE


#ifdef ASIO_STANDALONE
	//namespace asio = ::asio;
#else
	namespace boost::asio
	{
		using error_code = ::boost::system::error_code;
	}
	namespace asio = ::boost::asio;
	namespace beast = ::boost::beast;
	namespace http = ::boost::beast::http;
	namespace websocket = ::boost::beast::websocket;
#endif // ASIO_STANDALONE

namespace asio2
{
#ifdef ASIO_STANDALONE
	using error_code = ::asio::error_code;
	using system_error = ::asio::system_error;
#else
	using error_code = ::boost::system::error_code;
	using system_error = ::boost::system::system_error;

	namespace http = ::boost::beast::http;
	namespace websocket = ::boost::beast::websocket;
#endif // ASIO_STANDALONE
}

#define ASIO2_SEND_CORE_ASYNC

#endif // !__ASIO2_SELECTOR_HPP__
