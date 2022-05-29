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

#include <asio2/base/detail/push_options.hpp>

#include <cerrno>
#include <cassert>
#include <string>
#include <system_error>
#include <ios>
#include <future>

#include <asio2/external/asio.hpp>

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
		 * @function : get last error_code
		 */
		inline error_code & get_last_error() noexcept
		{
			return detail::external_linkaged_last_error::get();
		}

		/**
		 * @function : set last error_code
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
			else
			{
				asio::error_code ec;
				switch (static_cast<int>(e))
				{
				case static_cast<int>(asio::error::access_denied                   ) : ec = asio::error::access_denied                   ; break;
				case static_cast<int>(asio::error::address_family_not_supported    ) : ec = asio::error::address_family_not_supported    ; break;
				case static_cast<int>(asio::error::address_in_use                  ) : ec = asio::error::address_in_use                  ; break;
				case static_cast<int>(asio::error::already_connected               ) : ec = asio::error::already_connected               ; break;
				case static_cast<int>(asio::error::already_started                 ) : ec = asio::error::already_started                 ; break;
				case static_cast<int>(asio::error::broken_pipe                     ) : ec = asio::error::broken_pipe                     ; break;
				case static_cast<int>(asio::error::connection_aborted              ) : ec = asio::error::connection_aborted              ; break;
				case static_cast<int>(asio::error::connection_refused              ) : ec = asio::error::connection_refused              ; break;
				case static_cast<int>(asio::error::connection_reset                ) : ec = asio::error::connection_reset                ; break;
				case static_cast<int>(asio::error::bad_descriptor                  ) : ec = asio::error::bad_descriptor                  ; break;
				case static_cast<int>(asio::error::fault                           ) : ec = asio::error::fault                           ; break;
				case static_cast<int>(asio::error::host_unreachable                ) : ec = asio::error::host_unreachable                ; break;
				case static_cast<int>(asio::error::in_progress                     ) : ec = asio::error::in_progress                     ; break;
				case static_cast<int>(asio::error::interrupted                     ) : ec = asio::error::interrupted                     ; break;
				case static_cast<int>(asio::error::invalid_argument                ) : ec = asio::error::invalid_argument                ; break;
				case static_cast<int>(asio::error::message_size                    ) : ec = asio::error::message_size                    ; break;
				case static_cast<int>(asio::error::name_too_long                   ) : ec = asio::error::name_too_long                   ; break;
				case static_cast<int>(asio::error::network_down                    ) : ec = asio::error::network_down                    ; break;
				case static_cast<int>(asio::error::network_reset                   ) : ec = asio::error::network_reset                   ; break;
				case static_cast<int>(asio::error::network_unreachable             ) : ec = asio::error::network_unreachable             ; break;
				case static_cast<int>(asio::error::no_descriptors                  ) : ec = asio::error::no_descriptors                  ; break;
				case static_cast<int>(asio::error::no_buffer_space                 ) : ec = asio::error::no_buffer_space                 ; break;
				case static_cast<int>(asio::error::no_memory                       ) : ec = asio::error::no_memory                       ; break;
				case static_cast<int>(asio::error::no_permission                   ) : ec = asio::error::no_permission                   ; break;
				case static_cast<int>(asio::error::no_protocol_option              ) : ec = asio::error::no_protocol_option              ; break;
				case static_cast<int>(asio::error::no_such_device                  ) : ec = asio::error::no_such_device                  ; break;
				case static_cast<int>(asio::error::not_connected                   ) : ec = asio::error::not_connected                   ; break;
				case static_cast<int>(asio::error::not_socket                      ) : ec = asio::error::not_socket                      ; break;
				case static_cast<int>(asio::error::operation_aborted               ) : ec = asio::error::operation_aborted               ; break;
				case static_cast<int>(asio::error::operation_not_supported         ) : ec = asio::error::operation_not_supported         ; break;
				case static_cast<int>(asio::error::shut_down                       ) : ec = asio::error::shut_down                       ; break;
				case static_cast<int>(asio::error::timed_out                       ) : ec = asio::error::timed_out                       ; break;
				case static_cast<int>(asio::error::try_again                       ) : ec = asio::error::try_again                       ; break;
				case static_cast<int>(asio::error::would_block                     ) : ec = asio::error::would_block                     ; break;
				case static_cast<int>(asio::error::host_not_found                  ) : ec = asio::error::host_not_found                  ; break;
				case static_cast<int>(asio::error::host_not_found_try_again        ) : ec = asio::error::host_not_found_try_again        ; break;
				case static_cast<int>(asio::error::no_data                         ) : ec = asio::error::no_data                         ; break;
				case static_cast<int>(asio::error::no_recovery                     ) : ec = asio::error::no_recovery                     ; break;
				case static_cast<int>(asio::error::service_not_found               ) : ec = asio::error::service_not_found               ; break;
				case static_cast<int>(asio::error::socket_type_not_supported       ) : ec = asio::error::socket_type_not_supported       ; break;
				case static_cast<int>(asio::error::already_open                    ) : ec = asio::error::already_open                    ; break;
				case static_cast<int>(asio::error::eof                             ) : ec = asio::error::eof                             ; break;
				case static_cast<int>(asio::error::not_found                       ) : ec = asio::error::not_found                       ; break;
				case static_cast<int>(asio::error::fd_set_failure                  ) : ec = asio::error::fd_set_failure                  ; break;
				default: ec.assign(static_cast<int>(e), asio::error::get_system_category()); break;
				}
				get_last_error() = ec;
			}
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
		 * @function : get last error value, same as get_last_error_val
		 */
		inline auto last_error_val() noexcept
		{
			return get_last_error().value();
		}

		/**
		 * @function : get last error value
		 */
		inline auto get_last_error_val() noexcept
		{
			return get_last_error().value();
		}

		/**
		 * @function : get last error message, same as get_last_error_msg
		 */
		inline auto last_error_msg()
		{
			return get_last_error().message();
		}

		/**
		 * @function : get last error message
		 */
		inline auto get_last_error_msg()
		{
			return get_last_error().message();
		}
	}
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_ERROR_HPP__
