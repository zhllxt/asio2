/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_SOCKS5_SERVER_HPP__
#define __ASIO2_SOCKS5_SERVER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/tcp/tcp_server.hpp>
#include <asio2/proxy/socks5_session.hpp>

namespace asio2::detail
{
struct socks5_server_match_role
{
	template <typename Iterator>
	std::pair<Iterator, bool> operator()(Iterator begin, Iterator end) const
	{
		// +----+------+------+----------+----------+----------+
		// |RSV | FRAG | ATYP | DST.ADDR | DST.PORT |   DATA   |
		// +----+------+------+----------+----------+----------+
		// | 2  |  1   |  1   | Variable |    2     | Variable |
		// +----+------+------+----------+----------+----------+

		for (Iterator p = begin; p < end;)
		{
			if (this->get_front_client_type() == asio2::net_protocol::tcp)
				return std::pair(end, true);

			if (end - p < static_cast<diff_type>(5))
				break;

			std::uint16_t data_size = *(reinterpret_cast<const std::uint16_t*>(p.operator->()));

			// use little endian
			if (!is_little_endian())
			{
				swap_bytes<sizeof(std::uint16_t)>(reinterpret_cast<std::uint8_t*>(std::addressof(data_size)));
			}

			std::uint8_t atyp = std::uint8_t(p[3]);

			diff_type need = 0;

			// ATYP
			if /**/ (atyp == std::uint8_t(0x01))
				need = static_cast<diff_type>(2 + 1 + 1 + 4 + 2 + data_size);
			else if (atyp == std::uint8_t(0x03))
				need = static_cast<diff_type>(2 + 1 + 1 + p[4] + 2 + data_size);
			else if (atyp == std::uint8_t(0x04))
				need = static_cast<diff_type>(2 + 1 + 1 + 16 + 2 + data_size);
			else
				return std::pair(begin, true);

			if (end - p < need)
				break;

			return std::pair(p + need, true);
		}

		return std::pair(begin, false);
	}

	template<class T>
	void init(std::shared_ptr<T>& session_ptr)
	{
		this->get_front_client_type = [p = session_ptr.get()]() mutable
		{
			return p->get_front_client_type();
		};
	}

	std::function<asio2::net_protocol()> get_front_client_type;
};
}

#ifdef ASIO_STANDALONE
namespace asio
#else
namespace boost::asio
#endif
{
	template <> struct is_match_condition<asio2::detail::socks5_server_match_role> : public std::true_type {};
}

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;

	template<class derived_t, class session_t>
	class socks5_server_impl_t
		: public tcp_server_impl_t<derived_t, session_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;

	public:
		using super = tcp_server_impl_t   <derived_t, session_t>;
		using self  = socks5_server_impl_t<derived_t, session_t>;

		using session_type = session_t;

	public:
		/**
		 * @brief constructor
		 */
		template<class... Args>
		explicit socks5_server_impl_t(Args&&... args)
			: super(std::forward<Args>(args)...)
		{
		}

		/**
		 * @brief destructor
		 */
		~socks5_server_impl_t()
		{
			this->stop();
		}

		/**
		 * @brief start the server
		 * @param host - A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param service - A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt, typename... Args>
		inline bool start(String&& host, StrOrInt&& service, Args&&... args)
		{
			return this->derived()._do_start(
				std::forward<String>(host), std::forward<StrOrInt>(service),
				ecs_helper::make_ecs(detail::socks5_server_match_role{}, std::forward<Args>(args)...));
		}

	public:
		/**
		 * @brief bind socks5 handshake listener
		 * @param fun - a user defined callback function.
		 * @param obj - a pointer or reference to a class object, this parameter can be none.
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or reference.
		 * Function signature : void(std::shared_ptr<asio2::socks5_session>& session_ptr)
		 */
		template<class F, class ...C>
		inline derived_t& bind_socks5_handshake(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::upgrade,
				observer_t<std::shared_ptr<session_t>&>(
					std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	};
}

namespace asio2
{
	template<class derived_t, class session_t>
	using socks5_server_impl_t = detail::socks5_server_impl_t<derived_t, session_t>;

	/**
	 * @brief socks5 tcp server
	 * @throws constructor maybe throw exception "Too many open files" (exception code : 24)
	 * asio::error::no_descriptors - Too many open files
	 */
	template<class session_t>
	class socks5_server_t : public detail::socks5_server_impl_t<socks5_server_t<session_t>, session_t>
	{
	public:
		using detail::socks5_server_impl_t<socks5_server_t<session_t>, session_t>::socks5_server_impl_t;
	};

	/**
	 * @brief socks5 tcp server
	 * @throws constructor maybe throw exception "Too many open files" (exception code : 24)
	 * asio::error::no_descriptors - Too many open files
	 */
	using socks5_server = socks5_server_t<socks5_session>;
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_SOCKS5_SERVER_HPP__
