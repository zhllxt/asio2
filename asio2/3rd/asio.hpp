/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_ASIO_HPP__
#define __ASIO2_ASIO_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/config.hpp>

#ifdef ASIO_STANDALONE
#  ifndef ASIO_HEADER_ONLY
#    define ASIO_HEADER_ONLY
#  endif
#endif

#include <asio2/base/detail/push_options.hpp>

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

namespace asio2
{
	using error_code = ::asio::error_code;
	using system_error = ::asio::system_error;
}

namespace asio
{
	/*
	 * used for rdc mode, call("abc") or async_call("abc")
	 * Without the following overload, the compilation will fail.
	 */

	template<class CharT, class Traits = std::char_traits<CharT>>
	inline typename std::enable_if_t<
		std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char    > ||
		std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, wchar_t > ||
		std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char16_t> ||
		std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char32_t>,
		ASIO_CONST_BUFFER> buffer(CharT* & data) ASIO_NOEXCEPT
	{
		return asio::buffer(std::basic_string_view<CharT>(data));
	}

	template<class CharT, class Traits = std::char_traits<CharT>>
	inline typename std::enable_if_t<
		std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char    > ||
		std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, wchar_t > ||
		std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char16_t> ||
		std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char32_t>,
		ASIO_CONST_BUFFER> buffer(const CharT* & data) ASIO_NOEXCEPT
	{
		return asio::buffer(std::basic_string_view<CharT>(data));
	}

	template<class CharT, class Traits = std::char_traits<CharT>>
	inline typename std::enable_if_t<
		std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char    > ||
		std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, wchar_t > ||
		std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char16_t> ||
		std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char32_t>,
		ASIO_CONST_BUFFER> buffer(CharT* const& data) ASIO_NOEXCEPT
	{
		return asio::buffer(std::basic_string_view<CharT>(data));
	}

	template<class CharT, class Traits = std::char_traits<CharT>>
	inline typename std::enable_if_t<
		std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char    > ||
		std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, wchar_t > ||
		std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char16_t> ||
		std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char32_t>,
		ASIO_CONST_BUFFER> buffer(const CharT* const& data) ASIO_NOEXCEPT
	{
		return asio::buffer(std::basic_string_view<CharT>(data));
	}
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_ASIO_HPP__
