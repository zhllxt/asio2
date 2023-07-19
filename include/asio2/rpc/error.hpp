/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_RPC_ERROR_HPP__
#define __ASIO2_RPC_ERROR_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/error.hpp>

namespace asio2::rpc
{
	/**
	 */
	enum class error
	{
		// The operation completed successfully. 
		success = 0,

		// Already open.
		already_open = 1,

		// End of file or stream.
		eof,

		// Element not found.
		not_found,

		// Operation aborted.
		operation_aborted,

		// Invalid argument.
		invalid_argument,

		// Illegal data.
		illegal_data,

		// The operation timed out.
		timed_out,

		// Does not have associated data.
		no_data,

		// Operation now in progress.
		in_progress,

		// Transport endpoint is not connected.
		not_connected,

		// Unspecified error.
		unspecified_error,
	};

	enum class condition
	{
		// The operation completed successfully. 
		success = 0,

		// Already open.
		already_open = 1,

		// End of file or stream.
		eof,

		// Element not found.
		not_found,

		// Operation aborted.
		operation_aborted,

		// Invalid argument.
		invalid_argument,

		// Illegal data.
		illegal_data,

		// The operation timed out.
		timed_out,

		// Does not have associated data.
		no_data,

		// Operation now in progress.
		in_progress,

		// Transport endpoint is not connected.
		not_connected,

		// Unspecified error.
		unspecified_error,
	};

	/// The type of error category used by the library
	using error_category  = asio::error_category;

	/// The type of error condition used by the library
	using error_condition = asio::error_condition;

	class rpc_error_category : public error_category
	{
	public:
		const char* name() const noexcept override
		{
			return "asio2.rpc";
		}

		inline std::string message(int ev) const override
		{
			switch (static_cast<error>(ev))
			{
			case error::success               : return "The operation completed successfully.";
			case error::already_open          : return "Already open.";
			case error::eof                   : return "End of file or stream.";
			case error::not_found             : return "Element not found.";
			case error::operation_aborted     : return "Operation aborted.";
			case error::invalid_argument      : return "Invalid argument.";
			case error::illegal_data          : return "Illegal data.";
			case error::timed_out             : return "The operation timed out.";
			case error::no_data               : return "Does not have associated data.";
			case error::in_progress           : return "Operation now in progress.";
			case error::not_connected         : return "Transport endpoint is not connected.";
			case error::unspecified_error     : return "Unspecified error.";
			default                           : return "Unknown error";
			}
		}

		inline error_condition default_error_condition(int ev) const noexcept override
		{
			return error_condition{ ev, *this };
		}
	};

	inline const rpc_error_category& rpc_category() noexcept
	{
		static rpc_error_category const cat{};
		return cat;
	}

	inline asio::error_code make_error_code(error e)
	{
		return asio::error_code{ static_cast<std::underlying_type<error>::type>(e), rpc_category() };
	}

	inline error_condition make_error_condition(condition c)
	{
		return error_condition{ static_cast<std::underlying_type<condition>::type>(c), rpc_category() };
	}

	template<typename = void>
	inline constexpr std::string_view to_string(error e)
	{
		using namespace std::string_view_literals;
		switch (e)
		{
		case error::success               : return "The operation completed successfully.";
		case error::operation_aborted     : return "Operation aborted.";
		case error::invalid_argument      : return "Invalid argument.";
		case error::illegal_data          : return "Illegal data.";
		case error::timed_out             : return "The operation timed out.";
		case error::no_data               : return "Does not have associated data.";
		case error::in_progress           : return "Operation now in progress.";
		case error::not_connected         : return "Transport endpoint is not connected.";
		case error::not_found             : return "Element not found.";
		case error::unspecified_error     : return "Unspecified error.";
		default                           : return "Unknown error";
		}
		return "Unknown error";
	};
}

namespace rpc = ::asio2::rpc;

namespace std
{
	template<>
	struct is_error_code_enum<::asio2::rpc::error>
	{
		static bool const value = true;
	};
	template<>
	struct is_error_condition_enum<::asio2::rpc::condition>
	{
		static bool const value = true;
	};
}

#endif // !__ASIO2_RPC_ERROR_HPP__
