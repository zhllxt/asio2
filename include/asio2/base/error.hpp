/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_ERROR_HPP__
#define __ASIO2_ERROR_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

//#if !defined(NDEBUG) && !defined(_DEBUG) && !defined(DEBUG)
//#define NDEBUG
//#endif

#include <asio2/base/detail/push_options.hpp>

#include <cerrno>
#include <cassert>
#include <string>
#include <system_error>
#include <ios>
#include <future>

#include <asio2/external/asio.hpp>
#include <asio2/external/assert.hpp>

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
	// global variable : int x;
	// singleton class member function : class st{ static st& get(){static st& s; return s;} }

	namespace detail
	{
		class [[maybe_unused]] external_linkaged_last_error
		{
		public:
			[[maybe_unused]] static error_code & get() noexcept
			{
				// thread local variable of error_code
				thread_local static error_code ec_last{};

				return ec_last;
			}
		};

		namespace internal_linkaged_last_error
		{
			[[maybe_unused]] static error_code & get() noexcept
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
		 * @brief get last error_code
		 */
		inline error_code & get_last_error() noexcept
		{
			return detail::external_linkaged_last_error::get();
		}

		/**
		 * @brief set last error_code
		 */
		template<class ErrorCodeEnum>
		inline void set_last_error(ErrorCodeEnum e) noexcept
		{
			using type = std::remove_cv_t<std::remove_reference_t<ErrorCodeEnum>>;
			if /**/ constexpr (
				std::is_same_v<type, asio::error::basic_errors   > ||
				std::is_same_v<type, asio::error::netdb_errors   > ||
				std::is_same_v<type, asio::error::addrinfo_errors> ||
				std::is_same_v<type, asio::error::misc_errors    > )
			{
				get_last_error() = asio::error::make_error_code(e);
			}
			else if constexpr (
				std::is_same_v<type, std::errc       > ||
				std::is_same_v<type, std::io_errc    > ||
				std::is_same_v<type, std::future_errc> )
			{
			#ifdef ASIO_STANDALONE
				get_last_error() = std::make_error_code(e);
			#else
				get_last_error().assign(static_cast<int>(e), asio::error::get_system_category());
			#endif
			}
			else if constexpr (std::is_integral_v<type>)
			{
				get_last_error().assign(static_cast<int>(e), asio::error::get_system_category());
			}
			else if constexpr (std::is_enum_v<type>)
			{
				get_last_error() = e;
			}
			else
			{
				ASIO2_ASSERT(false);
				get_last_error().assign(static_cast<int>(e), asio::error::get_system_category());
			}
		}

		/**
		 * @brief set last error_code
		 */
		template<typename T>
		inline void set_last_error(int ec, const T& ecat) noexcept
		{
			get_last_error().assign(ec, ecat);
		}

		/**
		 * @brief set last error_code
		 */
		inline void set_last_error(const error_code & ec) noexcept
		{
			get_last_error() = ec;
		}

		/**
		 * @brief set last error_code
		 */
		inline void set_last_error(const system_error & e) noexcept
		{
			get_last_error() = e.code();
		}

		/**
		 * @brief Replaces the error code and error category with default values.
		 */
		inline void clear_last_error() noexcept
		{
			get_last_error().clear();
		}

		/**
		 * @brief get last error value, same as get_last_error_val
		 */
		inline auto last_error_val() noexcept
		{
			return get_last_error().value();
		}

		/**
		 * @brief get last error value
		 */
		inline auto get_last_error_val() noexcept
		{
			return get_last_error().value();
		}

		/**
		 * @brief get last error message, same as get_last_error_msg
		 */
		inline auto last_error_msg()
		{
			return get_last_error().message();
		}

		/**
		 * @brief get last error message
		 */
		inline auto get_last_error_msg()
		{
			return get_last_error().message();
		}
	}
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_ERROR_HPP__
