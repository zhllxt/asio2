/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_SELECTOR_HPP__
#define __ASIO2_SELECTOR_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/config.hpp>

#ifdef ASIO_STANDALONE
#  ifndef ASIO_HEADER_ONLY
#    define ASIO_HEADER_ONLY
#  endif
#endif

#ifdef _MSC_VER
#  pragma warning(push) 
#  pragma warning(disable:4311)
#  pragma warning(disable:4312)
#  pragma warning(disable:4996)
#endif

#if defined(__GNUC__) || defined(__GNUG__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wunused-variable"
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunused-variable"
#  pragma clang diagnostic ignored "-Wexceptions"
#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
#  pragma clang diagnostic ignored "-Wunused-private-field"
#  pragma clang diagnostic ignored "-Wunused-local-typedef"
#  pragma clang diagnostic ignored "-Wunknown-warning-option"
#endif

#ifdef ASIO_STANDALONE
	#include <asio/asio.hpp>
	#if defined(ASIO2_USE_SSL)
		#include <asio/ssl.hpp>
	#endif
	#ifndef BOOST_ASIO_VERSION
		#define BOOST_ASIO_VERSION ASIO_VERSION
	#endif
#else
	#include <boost/asio.hpp>
	#if defined(ASIO2_USE_SSL)
		#include <boost/asio/ssl.hpp>
	#endif
	#ifndef ASIO_VERSION
		#define ASIO_VERSION BOOST_ASIO_VERSION
	#endif
#endif // ASIO_STANDALONE

#ifdef BEAST_HEADER_ONLY
	#include <beast/beast.hpp>
	#if defined(ASIO2_USE_SSL)
		// boost 1.72(107200) BOOST_BEAST_VERSION 277
		#if defined(BEAST_VERSION) && (BEAST_VERSION >= 277)
			#include <beast/ssl.hpp>
		#endif
		#include <beast/websocket/ssl.hpp>
	#endif
	#ifndef BOOST_BEAST_VERSION
		#define BOOST_BEAST_VERSION BEAST_VERSION
	#endif
	#ifndef BOOST_BEAST_VERSION_STRING
		#define BOOST_BEAST_VERSION_STRING BEAST_VERSION_STRING
	#endif
#else
	#include <boost/beast.hpp>
	#if defined(ASIO2_USE_SSL)
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
#endif // BEAST_HEADER_ONLY


#ifdef ASIO_STANDALONE
	//namespace asio = ::asio;
#else
	namespace boost::asio
	{
		using error_code = ::boost::system::error_code;
		using system_error = ::boost::system::system_error;
	}
	namespace asio = ::boost::asio;

	// [ adding definitions to namespace alias ]
	// This is currently not allowed and probably won't be in C++1Z either,
	// but note that a recent proposal is allowing
	// https://stackoverflow.com/questions/31629101/adding-definitions-to-namespace-alias?r=SearchResults
	//namespace asio
	//{
	//	using error_code = ::boost::system::error_code;
	//	using system_error = ::boost::system::system_error;
	//}
#endif // ASIO_STANDALONE

#ifdef BEAST_HEADER_ONLY
#else
	namespace beast = ::boost::beast;
#endif // BEAST_HEADER_ONLY

namespace http = ::beast::http;
namespace websocket = ::beast::websocket;

namespace asio2
{
	using error_code = ::asio::error_code;
	using system_error = ::asio::system_error;

	namespace http = ::beast::http;
	namespace websocket = ::beast::websocket;
}

#if defined(__clang__)
#  pragma clang diagnostic pop
#endif

#if defined(__GNUC__) || defined(__GNUG__)
#  pragma GCC diagnostic pop
#endif

#if defined(_MSC_VER)
#  pragma warning(pop) 
#endif

#endif // !__ASIO2_SELECTOR_HPP__
