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
#include <vector>

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

		inline std::string const&     username() const noexcept { return  username_; }
		inline std::string const&     password() const noexcept { return  password_; }

		inline std::string& get_username() noexcept { return  username_; }
		inline std::string& get_password() noexcept { return  password_; }

		inline std::string const& get_username() const noexcept { return  username_; }
		inline std::string const& get_password() const noexcept { return  password_; }

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
	class options;

	struct option_base {};

	template<method... ms>
	class option : public socks5::option_base, public socks5::detail::method_field<option<ms...>, ms>...
	{
		friend class options;

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
		option(option&&) noexcept = default;
		option(option const&) = default;
		option& operator=(option&&) noexcept = default;
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
			String1&& proxy_host, String2&& proxy_port)
			: host_(asio2::detail::to_string(std::forward<String1>(proxy_host)))
			, port_(asio2::detail::to_string(std::forward<String2>(proxy_port)))
		{
		}

		template<class String1, class String2, class String3, class String4, std::enable_if_t<
			!std::is_base_of_v<socks5::option_base, asio2::detail::remove_cvref_t<String1>> &&
			!std::is_base_of_v<socks5::option_base, asio2::detail::remove_cvref_t<String2>> &&
			!std::is_base_of_v<socks5::option_base, asio2::detail::remove_cvref_t<String3>> &&
			!std::is_base_of_v<socks5::option_base, asio2::detail::remove_cvref_t<String4>>, int> = 0>
		explicit option(
			std::integral_constant<int, asio2::detail::to_underlying(method::password)>,
			String1&& proxy_host, String2&& proxy_port, String3&& username, String4&& password)
			: host_(asio2::detail::to_string(std::forward<String1>(proxy_host)))
			, port_(asio2::detail::to_string(std::forward<String2>(proxy_port)))
		{
			this->username(asio2::detail::to_string(std::forward<String3>(username)));
			this->password(asio2::detail::to_string(std::forward<String4>(password)));
		}

		template<class String3, class String4, std::enable_if_t<
			!std::is_base_of_v<socks5::option_base, asio2::detail::remove_cvref_t<String3>> &&
			!std::is_base_of_v<socks5::option_base, asio2::detail::remove_cvref_t<String4>>, int> = 0>
		explicit option(
			std::integral_constant<int, asio2::detail::to_underlying(method::password)>,
			String3&& username, String4&& password)
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

		inline std::string const&     host() const noexcept { return host_; }
		inline std::string const&     port() const noexcept { return port_; }

		inline std::string& get_host() noexcept { return host_; }
		inline std::string& get_port() noexcept { return port_; }

		inline std::string const& get_host() const noexcept { return host_; }
		inline std::string const& get_port() const noexcept { return port_; }

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

		inline socks5::command     command() const              noexcept { return cmd_;                }
		inline socks5::command get_command() const              noexcept { return cmd_;                }

		inline option&             command(socks5::command cmd) noexcept { cmd_ = cmd; return (*this); }
		inline option&         set_command(socks5::command cmd) noexcept { cmd_ = cmd; return (*this); }

	protected:
		std::array<method, sizeof...(ms)> methods_{ ms... };

		std::string host_{};
		std::string port_{};

		socks5::command cmd_{};
	};

	class options : public socks5::option_base
	{
	protected:
		template<class S5Opt>
		inline std::string socks5_opt_username(S5Opt&& s5opt) noexcept
		{
			if constexpr (socks5::detail::has_member_username<S5Opt>::value)
				return s5opt.username();
			else
				return "";
		}

		template<class S5Opt>
		inline std::string socks5_opt_password(S5Opt&& s5opt) noexcept
		{
			if constexpr (socks5::detail::has_member_password<S5Opt>::value)
				return s5opt.password();
			else
				return "";
		}

	public:
		options() = default;
		~options() = default;

		options(options&&) noexcept = default;
		options(options const&) = default;
		options& operator=(options&&) noexcept = default;
		options& operator=(options const&) = default;

		template<class String1, class String2>
		options(String1&& host, String2&& port)
		{
			methods_.emplace_back(socks5::method::anonymous);
			host_ = asio2::detail::to_string(std::forward<String1>(host));
			port_ = asio2::detail::to_string(std::forward<String2>(port));
		}

		template<class String1, class String2, class String3, class String4>
		options(String1&& host, String2&& port, String3&& username, String4&& password)
		{
			methods_.emplace_back(socks5::method::anonymous);
			methods_.emplace_back(socks5::method::password);
			host_ = asio2::detail::to_string(std::forward<String1>(host));
			port_ = asio2::detail::to_string(std::forward<String2>(port));
			username_ = asio2::detail::to_string(std::forward<String3>(username));
			password_ = asio2::detail::to_string(std::forward<String4>(password));
		}

		template<method... ms>
		options& operator=(const socks5::option<ms...>& opt)
		{
			methods_.clear();
			for (socks5::method m : opt.methods_)
			{
				methods_.emplace_back(m);
			}
			cmd_ = opt.cmd_;
			host_ = opt.host_;
			port_ = opt.port_;
			username_ = socks5_opt_username(opt);
			password_ = socks5_opt_password(opt);

			return *this;
		}

		template<method... ms>
		options(const socks5::option<ms...>& opt)
		{
			*this = opt;
		}

		template<class... Ms>
		inline options& set_methods(Ms... ms)
		{
			methods_.clear();
			(methods_.emplace_back(ms), ...);
			return *this;
		}

		inline std::vector<socks5::method>& get_methods() noexcept { return methods_; }
		inline std::vector<socks5::method>& methods()     noexcept { return methods_; }

		inline const std::vector<socks5::method>& get_methods() const noexcept { return methods_; }
		inline const std::vector<socks5::method>& methods()     const noexcept { return methods_; }

		inline std::size_t get_methods_count() const noexcept { return methods_.size(); }
		inline std::size_t methods_count()     const noexcept { return methods_.size(); }

		inline bool has_password_method() const noexcept
		{
			for (socks5::method m : methods_)
			{
				if (m == socks5::method::password)
					return true;
			}
			return false;
		}

		inline options& set_command(socks5::command cmd)
		{
			cmd_ = cmd;
			return *this;
		}

		inline options& command(socks5::command cmd)
		{
			cmd_ = cmd;
			return *this;
		}

		inline socks5::command get_command() { return cmd_; }
		inline socks5::command command()     { return cmd_; }

		template<class String1>
		inline options& set_host(String1&& host)
		{
			host_ = asio2::detail::to_string(std::forward<String1>(host));
			return (*this);
		}

		template<class String1>
		inline options& host(String1&& host)
		{
			return set_host(std::forward<String1>(host));
		}

		template<class StrOrInt>
		inline options& set_port(StrOrInt&& port)
		{
			port_ = asio2::detail::to_string(std::forward<StrOrInt>(port));
			return (*this);
		}

		template<class StrOrInt>
		inline options& port(StrOrInt&& port)
		{
			return set_port(std::forward<StrOrInt>(port));
		}

		template<class String1>
		inline options& set_username(String1&& username)
		{
			username_ = asio2::detail::to_string(std::forward<String1>(username));
			return (*this);
		}

		template<class String1>
		inline options& username(String1&& username)
		{
			return set_username(std::forward<String1>(username));
		}

		template<class String1>
		inline options& set_password(String1&& password)
		{
			password_ = asio2::detail::to_string(std::forward<String1>(password));
			return (*this);
		}

		template<class String1>
		inline options& password(String1&& password)
		{
			return set_password(std::forward<String1>(password));
		}

		inline std::string& get_host()     noexcept { return host_;}
		inline std::string& get_port()     noexcept { return port_;}
		inline std::string& get_username() noexcept { return username_; }
		inline std::string& get_password() noexcept { return password_; }

		inline std::string& host()         noexcept { return host_; }
		inline std::string& port()         noexcept { return port_; }
		inline std::string& username()     noexcept { return username_; }
		inline std::string& password()     noexcept { return password_; }

		inline const std::string& get_host()     const noexcept { return host_;}
		inline const std::string& get_port()     const noexcept { return port_;}
		inline const std::string& get_username() const noexcept { return username_; }
		inline const std::string& get_password() const noexcept { return password_; }

		inline const std::string& host()         const noexcept { return host_; }
		inline const std::string& port()         const noexcept { return port_; }
		inline const std::string& username()     const noexcept { return username_; }
		inline const std::string& password()     const noexcept { return password_; }

		inline options& set_auth_callback(std::function<bool(const std::string&, const std::string&)> auth_cb)
		{
			auth_cb_ = std::move(auth_cb);
			return *this;
		}
		inline std::function<bool(const std::string&, const std::string&)>& get_auth_callback() noexcept
		{
			return auth_cb_;
		}
		inline const std::function<bool(const std::string&, const std::string&)>& get_auth_callback() const noexcept
		{
			return auth_cb_;
		}

	protected:
		std::vector<socks5::method> methods_;

		std::string host_{};
		std::string port_{};

		std::string username_{};
		std::string password_{};

		socks5::command cmd_{};

		std::function<bool(const std::string&, const std::string&)> auth_cb_;
	};
}

#endif // !__ASIO2_SOCKS5_OPTION_HPP__
