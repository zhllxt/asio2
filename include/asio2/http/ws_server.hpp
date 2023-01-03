/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_WS_SERVER_HPP__
#define __ASIO2_WS_SERVER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/tcp/tcp_server.hpp>
#include <asio2/http/ws_session.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;

	template<class derived_t, class session_t>
	class ws_server_impl_t : public tcp_server_impl_t<derived_t, session_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;

	public:
		using super = tcp_server_impl_t<derived_t, session_t>;
		using self  = ws_server_impl_t <derived_t, session_t>;

		using session_type = session_t;

	public:
		/**
		 * @brief constructor
		 */
		template<class ...Args>
		explicit ws_server_impl_t(Args&&... args)
			: super(std::forward<Args>(args)...)
		{
		}

		/**
		 * @brief destructor
		 */
		~ws_server_impl_t()
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
		bool start(String&& host, StrOrInt&& service, Args&&... args)
		{
			return this->derived()._do_start(
				std::forward<String>(host), std::forward<StrOrInt>(service),
				ecs_helper::make_ecs('0', std::forward<Args>(args)...));
		}

	public:
		/**
		 * @brief bind websocket upgrade listener
		 * @param fun - a user defined callback function.
		 * Function signature : void(std::shared_ptr<asio2::ws_session>& session_ptr)
		 */
		template<class F, class ...C>
		inline derived_t & bind_upgrade(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::upgrade, observer_t<std::shared_ptr<session_t>&>
				(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	protected:

	};
}

namespace asio2
{
	template<class derived_t, class session_t>
	using ws_server_impl_t = detail::ws_server_impl_t<derived_t, session_t>;

	template<class session_t>
	class ws_server_t : public detail::ws_server_impl_t<ws_server_t<session_t>, session_t>
	{
	public:
		using detail::ws_server_impl_t<ws_server_t<session_t>, session_t>::ws_server_impl_t;
	};

	using ws_server = ws_server_t<ws_session>;
}

#if defined(ASIO2_INCLUDE_RATE_LIMIT)
#include <asio2/tcp/tcp_stream.hpp>
namespace asio2
{
	template<class session_t>
	class ws_rate_server_t : public asio2::ws_server_impl_t<ws_rate_server_t<session_t>, session_t>
	{
	public:
		using asio2::ws_server_impl_t<ws_rate_server_t<session_t>, session_t>::ws_server_impl_t;
	};

	using ws_rate_server = ws_rate_server_t<ws_rate_session>;
}
#endif

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_WS_SERVER_HPP__
