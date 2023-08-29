/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#if defined(ASIO2_ENABLE_SSL) || defined(ASIO2_USE_SSL)

#ifndef __ASIO2_WSS_SERVER_HPP__
#define __ASIO2_WSS_SERVER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/tcp/tcps_server.hpp>
#include <asio2/http/wss_session.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;

	template<class derived_t, class session_t>
	class wss_server_impl_t : public tcps_server_impl_t<derived_t, session_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;

	public:
		using super = tcps_server_impl_t<derived_t, session_t>;
		using self  = wss_server_impl_t <derived_t, session_t>;

		using session_type = session_t;

	public:
		/**
		 * @brief constructor
		 */
		template<class... Args>
		explicit wss_server_impl_t(
			asio::ssl::context::method method,
			Args&&... args
		)
			: super(method, std::forward<Args>(args)...)
		{
		}

		/**
		 * @brief constructor
		 */
		template<class... Args>
		explicit wss_server_impl_t(Args&&... args)
			: super(ASIO2_DEFAULT_SSL_METHOD, std::forward<Args>(args)...)
		{
		}

		/**
		 * @brief constructor
		 */
		explicit wss_server_impl_t()
			: super(ASIO2_DEFAULT_SSL_METHOD)
		{
		}

		/**
		 * @brief destructor
		 */
		~wss_server_impl_t()
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
		 * Function signature : void(std::shared_ptr<asio2::wss_session>& session_ptr)
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
	using wss_server_impl_t = detail::wss_server_impl_t<derived_t, session_t>;

	template<class session_t>
	class wss_server_t : public detail::wss_server_impl_t<wss_server_t<session_t>, session_t>
	{
	public:
		using detail::wss_server_impl_t<wss_server_t<session_t>, session_t>::wss_server_impl_t;
	};

	using wss_server = wss_server_t<wss_session>;
}

#if defined(ASIO2_INCLUDE_RATE_LIMIT)
#include <asio2/tcp/tcp_stream.hpp>
namespace asio2
{
	template<class session_t>
	class wss_rate_server_t : public asio2::wss_server_impl_t<wss_rate_server_t<session_t>, session_t>
	{
	public:
		using asio2::wss_server_impl_t<wss_rate_server_t<session_t>, session_t>::wss_server_impl_t;
	};

	using wss_rate_server = wss_rate_server_t<wss_rate_session>;
}
#endif

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_WSS_SERVER_HPP__

#endif
