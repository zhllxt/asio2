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

	// ---- internal linkage ----
	// static global variable : static int x;
	// static global function, namespace scope or not : static bool test(){...}
	// enum definition : enum Boolean { NO,YES };
	// class definition : class Point { int d_x; int d_y; ... };
	// inline function definition : inline int operator==(const Point& left,const Point&right) { ... }
	// union definition
	// const variable definition
	//
	// ---- external linkage ----
	// not inlined class member function : Point& Point::operator+=(const Point& right) { ... }
	// not inlined not static function : Point operator+(const Point& left, const Point& right) { ... }
	// global variable : static int x;
	// singleton class member function : class st{ static st& get(){static st& s; return s;} }

	namespace detail
	{
		class external_linkaged_last_error
		{
		public:
			static error_code & get() noexcept
			{
				// thread local variable of error_code
				thread_local static error_code ec_last{};

				return ec_last;
			}
		};

		namespace internal_linkaged_last_error
		{
			static error_code & get() noexcept
			{
				// thread local variable of error_code
				thread_local static error_code ec_last{};

				return ec_last;
			}
		}
	}

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
		inline error_code & get_last_error() noexcept
		{
			return detail::external_linkaged_last_error::get();
		}

		/**
		 * @function : set last error_code
		 */
		inline void set_last_error(int ec) noexcept
		{
			get_last_error().assign(ec, asio::error::get_system_category());
		}

		/**
		 * @function : set last error_code
		 */
		template<typename T>
		inline void set_last_error(int ec, const T& ecat) noexcept
		{
			get_last_error().assign(ec, ecat);
		}

		/**
		 * @function : set last error_code
		 */
		inline void set_last_error(const error_code & ec) noexcept
		{
			get_last_error() = ec;
		}

		/**
		 * @function : set last error_code
		 */
		inline void set_last_error(const system_error & e) noexcept
		{
			get_last_error() = e.code();
		}

		/**
		 * @function : Replaces the error code and error category with default values.
		 */
		inline void clear_last_error() noexcept
		{
			get_last_error().clear();
		}

		/**
		 * @function : get last error value
		 */
		inline auto last_error_val() noexcept
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
