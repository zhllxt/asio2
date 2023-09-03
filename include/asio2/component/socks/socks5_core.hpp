/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_SOCKS5_CORE_HPP__
#define __ASIO2_SOCKS5_CORE_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>

#include <memory>
#include <chrono>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>

#include <asio2/base/error.hpp>
#include <asio2/base/define.hpp>

#include <asio2/base/detail/util.hpp>

namespace asio2::socks5
{
	///-----------------------------------------------------------------------------------------------
	/// SOCKS Protocol Version 5: 
	///		https://www.ietf.org/rfc/rfc1928.txt
	/// Username/Password Authentication for SOCKS V5 : 
	///		https://www.ietf.org/rfc/rfc1929.txt
	/// GSSAPI : Generic Security Services Application Program Interface
	///		https://en.wikipedia.org/wiki/Generic_Security_Services_Application_Program_Interface
	///-----------------------------------------------------------------------------------------------

	/// The type of error category used by the library
	using error_category = asio::error_category;

	/// The type of error condition used by the library
	using error_condition = asio::error_condition;

	enum class error
	{
		success = 0,

		/// unsupported version.
		unsupported_version,

		/// unsupported method.
		unsupported_method,

		/// no acceptable methods.
		no_acceptable_methods,

		/// username required.
		username_required,

		/// unsupported authentication version.
		unsupported_authentication_version,

		/// authentication failed.
		authentication_failed,

		/// general failure.
		general_failure,

		/// no identd running.
		no_identd,

		/// no identd running.
		identd_error,

		/// general socks server failure
		general_socks_server_failure,

		/// connection not allowed by ruleset
		connection_not_allowed_by_ruleset,

		/// network unreachable
		network_unreachable,

		/// host unreachable
		host_unreachable,

		/// connection refused
		connection_refused,

		/// ttl expired
		ttl_expired,

		/// command not supported
		command_not_supported,

		/// address type not supported
		address_type_not_supported,

		/// to x'ff' unassigned
		unassigned,
	};

	enum class condition
	{
		success = 0,

		/// unsupported version.
		unsupported_version,

		/// unsupported method.
		unsupported_method,

		/// no acceptable methods.
		no_acceptable_methods,

		/// username required.
		username_required,

		/// unsupported authentication version.
		unsupported_authentication_version,

		/// authentication failed.
		authentication_failed,

		/// general failure.
		general_failure,

		/// no identd running.
		no_identd,

		/// no identd running.
		identd_error,

		/// general socks server failure
		general_socks_server_failure,

		/// connection not allowed by ruleset
		connection_not_allowed_by_ruleset,

		/// network unreachable
		network_unreachable,

		/// host unreachable
		host_unreachable,

		/// connection refused
		connection_refused,

		/// ttl expired
		ttl_expired,

		/// command not supported
		command_not_supported,

		/// address type not supported
		address_type_not_supported,

		/// to x'ff' unassigned
		unassigned,
	};

	class socks5_error_category : public error_category
	{
	public:
		const char* name() const noexcept override
		{
			return "asio2.socks5";
		}

		inline std::string message(int ev) const override
		{
			switch (static_cast<error>(ev))
			{
			case error::success:
				return "Success";
			case error::unsupported_version:
				return "unsupported version";
			case error::unsupported_method:
				return "unsupported method";
			case error::no_acceptable_methods:
				return "no acceptable methods";
			case error::username_required:
				return "username required";
			case error::unsupported_authentication_version:
				return "unsupported authentication version";
			case error::authentication_failed:
				return "authentication failed";
			case error::general_failure:
				return "general failure";
			case error::no_identd:
				return "no identd running";
			case error::identd_error:
				return "no identd running";
			case error::general_socks_server_failure:
				return "general socks server failure";
			case error::connection_not_allowed_by_ruleset:
				return "connection not allowed by ruleset";
			case error::network_unreachable:
				return "network unreachable";
			case error::host_unreachable:
				return "host unreachable";
			case error::connection_refused:
				return "connection refused";
			case error::ttl_expired:
				return "ttl expired";
			case error::command_not_supported:
				return "command not supported";
			case error::address_type_not_supported:
				return "address type not supported";
			case error::unassigned:
				return "to x'ff' unassigned";
			default:
				return "Unknown PROXY error";
			}
		}

		inline error_condition default_error_condition(int ev) const noexcept override
		{
			return error_condition{ ev, *this };
		}
	};

	inline const socks5_error_category& socks5_category() noexcept
	{
		static socks5_error_category const cat{};
		return cat;
	}

	inline error_code make_error_code(error e) noexcept
	{
		return error_code{ static_cast<std::underlying_type<error>::type>(e), socks5_category() };
	}

	inline error_condition make_error_condition(condition c) noexcept
	{
		return error_condition{ static_cast<std::underlying_type<condition>::type>(c), socks5_category() };
	}
}

namespace std
{
	template<>
	struct is_error_code_enum<::asio2::socks5::error>
	{
		static bool const value = true;
	};
	template<>
	struct is_error_condition_enum<::asio2::socks5::condition>
	{
		static bool const value = true;
	};
}

namespace asio2::socks5
{
	enum class method : std::uint8_t
	{
		anonymous         = 0x00, // X'00' NO AUTHENTICATION REQUIRED
		gssapi            = 0x01, // X'01' GSSAPI
		password          = 0x02, // X'02' USERNAME/PASSWORD
		//iana              = 0x03, // X'03' to X'7F' IANA ASSIGNED
		//reserved          = 0x80, // X'80' to X'FE' RESERVED FOR PRIVATE METHODS
		noacceptable      = 0xFF, // X'FF' NO ACCEPTABLE METHODS
	};

	enum class command : std::uint8_t
	{
		connect       = 0x01, // CONNECT X'01'
		bind          = 0x02, // BIND X'02'
		udp_associate = 0x03, // UDP ASSOCIATE X'03'
	};
}

namespace socks5 = ::asio2::socks5;

#endif // !__ASIO2_SOCKS5_CORE_HPP__
