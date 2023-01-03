/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_SOCKS5_OPTION_HPP__
#define __ASIO2_SOCKS5_OPTION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <array>

#include <asio2/component/socks/socks5_core.hpp>

namespace asio2::socks5::detail
{
	template<class derived_t, method m>
	class method_field
	{
	};

	template<class derived_t>
	class method_field<derived_t, method::password>
	{
	public:
		method_field() = default;
		method_field(method_field&&) noexcept = default;
		method_field(method_field const&) = default;
		method_field& operator=(method_field&&) noexcept = default;
		method_field& operator=(method_field const&) = default;

		method_field(std::string username, std::string password)
			: username_(std::move(username))
			, password_(std::move(password))
		{
			ASIO2_ASSERT(username_.size() <= std::size_t(0xff) && password_.size() <= std::size_t(0xff));
		}

		inline derived_t& set_username(std::string v)
		{
			username_ = std::move(v);
			ASIO2_ASSERT(username_.size() <= std::size_t(0xff));
			return static_cast<derived_t&>(*this);
		}
		inline derived_t& set_password(std::string v)
		{
			password_ = std::move(v);
			ASIO2_ASSERT(password_.size() <= std::size_t(0xff));
			return static_cast<derived_t&>(*this);
		}

		inline derived_t& username(std::string v)
		{
			return this->set_username(std::move(v));
		}
		inline derived_t& password(std::string v)
		{
			return this->set_password(std::move(v));
		}

		inline std::string&     username() noexcept { return  username_; }
		inline std::string&     password() noexcept { return  password_; }

		inline std::string& get_username() noexcept { return  username_; }
		inline std::string& get_password() noexcept { return  password_; }

	protected:
		std::string username_{};
		std::string password_{};
	};

	template<class T, class = void>
	struct has_member_username : std::false_type {};

	template<class T>
	struct has_member_username<T, std::void_t<decltype(std::declval<std::decay_t<T>>().username())>> : std::true_type {};

	template<class T, class = void>
	struct has_member_password : std::false_type {};

	template<class T>
	struct has_member_password<T, std::void_t<decltype(std::declval<std::decay_t<T>>().password())>> : std::true_type {};
}

namespace asio2::socks5
{
	struct option_base {};

	template<method... ms>
	class option : public socks5::option_base, public socks5::detail::method_field<option<ms...>, ms>...
	{
	protected:
		// vs2017 15.9.31 not supported
		//std::enable_if_t<((!std::is_base_of_v<socks5::option_base, asio2::detail::remove_cvref_t<Args>>) && ...)
		template<class... Args>
		constexpr static bool not_options() noexcept
		{
			return ((!std::is_base_of_v<socks5::option_base, asio2::detail::remove_cvref_t<Args>>) && ...);
		}

	public:
		option() = default;
		option(option&&) = default;
		option(option const&) = default;
		option& operator=(option&&) = default;
		option& operator=(option const&) = default;

		// constructor sfinae
		template<class... Args, std::enable_if_t<not_options<Args...>(), int> = 0>
		explicit option(Args&&... args) : option(
			std::conditional_t<option<ms...>::has_password_method(),
			std::integral_constant<int, asio2::detail::to_underlying(method::password)>,
			std::integral_constant<int, asio2::detail::to_underlying(method::anonymous)>>{}
		, std::forward<Args>(args)...)
		{
		}

		template<class String1, class String2, std::enable_if_t<
			!std::is_base_of_v<socks5::option_base, asio2::detail::remove_cvref_t<String1>> &&
			!std::is_base_of_v<socks5::option_base, asio2::detail::remove_cvref_t<String2>>, int> = 0>
		explicit option(
			std::integral_constant<int, asio2::detail::to_underlying(method::anonymous)>,
			String1&& proxy_host, String2&& proxy_port,
			socks5::command cmd = socks5::command::connect)
			: host_(asio2::detail::to_string(std::forward<String1>(proxy_host)))
			, port_(asio2::detail::to_string(std::forward<String2>(proxy_port)))
			, cmd_ (cmd)
		{
		}

		template<class String1, class String2, class String3, class String4, std::enable_if_t<
			!std::is_base_of_v<socks5::option_base, asio2::detail::remove_cvref_t<String1>> &&
			!std::is_base_of_v<socks5::option_base, asio2::detail::remove_cvref_t<String2>> &&
			!std::is_base_of_v<socks5::option_base, asio2::detail::remove_cvref_t<String3>> &&
			!std::is_base_of_v<socks5::option_base, asio2::detail::remove_cvref_t<String4>>, int> = 0>
		explicit option(
			std::integral_constant<int, asio2::detail::to_underlying(method::password)>,
			String1&& proxy_host, String2&& proxy_port, String3&& username, String4&& password,
			socks5::command cmd = socks5::command::connect)
			: host_(asio2::detail::to_string(std::forward<String1>(proxy_host)))
			, port_(asio2::detail::to_string(std::forward<String2>(proxy_port)))
			, cmd_ (cmd)
		{
			this->username(asio2::detail::to_string(std::forward<String3>(username)));
			this->password(asio2::detail::to_string(std::forward<String4>(password)));
		}

		inline option& set_host(std::string proxy_host)
		{
			host_ = std::move(proxy_host);
			return (*this);
		}
		template<class StrOrInt>
		inline option& set_port(StrOrInt&& proxy_port)
		{
			port_ = asio2::detail::to_string(std::forward<StrOrInt>(proxy_port));
			return (*this);
		}

		inline option& host(std::string proxy_host)
		{
			return this->set_host(std::move(proxy_host));
		}
		template<class StrOrInt>
		inline option& port(StrOrInt&& proxy_port)
		{
			return this->set_port(std::forward<StrOrInt>(proxy_port));
		}

		inline std::string&     host() noexcept { return host_; }
		inline std::string&     port() noexcept { return port_; }

		inline std::string& get_host() noexcept { return host_; }
		inline std::string& get_port() noexcept { return port_; }

		// vs2017 15.9.31 not supported
		//constexpr static bool has_password_method = ((ms == method::password) || ...);
		template<typename = void>
		constexpr static bool has_password_method() noexcept
		{
			return ((ms == method::password) || ...);
		}

		inline std::array<method, sizeof...(ms)> get_methods() noexcept
		{
			return methods_;
		}

		inline std::array<method, sizeof...(ms)> methods() noexcept
		{
			return methods_;
		}

		constexpr auto get_methods_count() const noexcept
		{
			return methods_.size();
		}

		constexpr auto methods_count() const noexcept
		{
			return methods_.size();
		}

		inline socks5::command     command()                    noexcept { return cmd_;                }
		inline socks5::command get_command()                    noexcept { return cmd_;                }

		inline option&             command(socks5::command cmd) noexcept { cmd_ = cmd; return (*this); }
		inline option&         set_command(socks5::command cmd) noexcept { cmd_ = cmd; return (*this); }

	protected:
		std::array<method, sizeof...(ms)> methods_{ ms... };

		std::string host_{};
		std::string port_{};

		socks5::command cmd_{ socks5::command::connect };
	};
}

#endif // !__ASIO2_SOCKS5_OPTION_HPP__
