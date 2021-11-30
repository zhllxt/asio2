/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_ERROR_HPP__
#define __ASIO2_ERROR_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if !defined(NDEBUG) && !defined(_DEBUG) && !defined(DEBUG)
#define NDEBUG
#endif

#include <cerrno>
#include <cassert>
#include <string>
#include <system_error>

#include <asio2/3rd/asio.hpp>

#ifdef ASIO2_ASSERT
	static_assert(false, "Unknown ASIO2_ASSERT definition will affect the relevant functions of this program.");
#else
	#if defined(_DEBUG) || defined(DEBUG)
		#define ASIO2_ASSERT(x) assert(x)
	#else
		#define ASIO2_ASSERT(x) ((void)0)
	#endif
#endif

namespace asio2
{
	// Very important features:
	// One Definition Rule : https://en.wikipedia.org/wiki/One_Definition_Rule

	// use anonymous namespace to resolve global function redefinition problem
	namespace
	{
		/**
		 * thread local variable of error_code
		 * In vs2017, sometimes the "namespace's thread_local error_code" will cause crash at
		 * system_category() -> (_Immortalize<_System_error_category>())-> _Execute_once(...),
		 * and the crash happens before the "main" function.
		 */
		//thread_local static error_code ec_last;

		/**
		 * @function : get last error_code
		 */
		inline error_code & get_last_error()
		{
			// thread local variable of error_code
			thread_local static error_code ec_last{};

			return ec_last;
		}

		/**
		 * @function : set last error_code
		 */
		inline void set_last_error(int ec)
		{
			get_last_error().assign(ec, asio::error::get_system_category());
		}

		/**
		 * @function : set last error_code
		 */
		template<typename T>
		inline void set_last_error(int ec, const T& ecat)
		{
			get_last_error().assign(ec, ecat);
		}

		/**
		 * @function : set last error_code
		 */
		inline void set_last_error(const error_code & ec)
		{
			get_last_error() = ec;
		}

		/**
		 * @function : set last error_code
		 */
		inline void set_last_error(const system_error & e)
		{
			get_last_error() = e.code();
		}

		/**
		 * @function : Replaces the error code and error category with default values.
		 */
		inline void clear_last_error()
		{
			get_last_error().clear();
		}

		/**
		 * @function : get last error value
		 */
		inline auto last_error_val()
		{
			return get_last_error().value();
		}

		/**
		 * @function : get last error message
		 */
		inline auto last_error_msg()
		{
			return get_last_error().message();
		}
	}
}

#endif // !__ASIO2_ERROR_HPP__
