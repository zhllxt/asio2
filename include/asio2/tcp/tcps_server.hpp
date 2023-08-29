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

#ifndef __ASIO2_TCPS_SERVER_HPP__
#define __ASIO2_TCPS_SERVER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/tcp/tcp_server.hpp>
#include <asio2/tcp/tcps_session.hpp>

namespace asio2::detail
{
	struct template_args_tcps_server
	{
		static constexpr bool is_session = false;
		static constexpr bool is_client  = false;
		static constexpr bool is_server  = true;
	};

	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;

	template<class derived_t, class session_t>
	class tcps_server_impl_t
		: public ssl_context_cp   <derived_t, template_args_tcps_server>
		, public tcp_server_impl_t<derived_t, session_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;

	public:
		using super = tcp_server_impl_t <derived_t, session_t>;
		using self  = tcps_server_impl_t<derived_t, session_t>;

		using session_type = session_t;

	public:
		/**
		 * @brief constructor
		 */
		template<class... Args>
		explicit tcps_server_impl_t(
			asio::ssl::context::method method,
			Args&&... args
		)
			: ssl_context_cp<derived_t, template_args_tcps_server>(method)
			, super(std::forward<Args>(args)...)
		{
		}

		/**
		 * @brief constructor
		 */
		template<class... Args>
		explicit tcps_server_impl_t(Args&&... args)
			: ssl_context_cp<derived_t, template_args_tcps_server>(ASIO2_DEFAULT_SSL_METHOD)
			, super(std::forward<Args>(args)...)
		{
		}

		/**
		 * @brief constructor
		 */
		explicit tcps_server_impl_t()
			: ssl_context_cp<derived_t, template_args_tcps_server>(ASIO2_DEFAULT_SSL_METHOD)
			, super()
		{
		}

		/**
		 * @brief destructor
		 */
		~tcps_server_impl_t()
		{
			this->stop();
		}

	public:
		/**
		 * @brief bind ssl handshake listener
		 * @param fun - a user defined callback function.
		 * @param obj - a pointer or reference to a class object, this parameter can be none.
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or reference.
		 * Function signature : void(std::shared_ptr<asio2::tcps_session>& session_ptr)
		 */
		template<class F, class ...C>
		inline derived_t & bind_handshake(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::handshake,
				observer_t<std::shared_ptr<session_t>&>(
					std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	protected:
		template<typename... Args>
		inline std::shared_ptr<session_t> _make_session(Args&&... args)
		{
			return super::_make_session(std::forward<Args>(args)..., *this);
		}
	};
}

namespace asio2
{
	template<class derived_t, class session_t>
	using tcps_server_impl_t = detail::tcps_server_impl_t<derived_t, session_t>;

	/**
	 * @brief ssl tcp server
	 * @throws constructor maybe throw exception "Too many open files" (exception code : 24)
	 * asio::error::no_descriptors - Too many open files
	 */
	template<class session_t>
	class tcps_server_t : public detail::tcps_server_impl_t<tcps_server_t<session_t>, session_t>
	{
	public:
		using detail::tcps_server_impl_t<tcps_server_t<session_t>, session_t>::tcps_server_impl_t;
	};

	/**
	 * @brief ssl tcp server
	 * @throws constructor maybe throw exception "Too many open files" (exception code : 24)
	 * asio::error::no_descriptors - Too many open files
	 */
	using tcps_server = tcps_server_t<tcps_session>;
}

#if defined(ASIO2_INCLUDE_RATE_LIMIT)
#include <asio2/tcp/tcp_stream.hpp>
namespace asio2
{
	template<class session_t>
	class tcps_rate_server_t : public asio2::tcps_server_impl_t<tcps_rate_server_t<session_t>, session_t>
	{
	public:
		using asio2::tcps_server_impl_t<tcps_rate_server_t<session_t>, session_t>::tcps_server_impl_t;
	};

	using tcps_rate_server = tcps_rate_server_t<tcps_rate_session>;
}
#endif

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_TCPS_SERVER_HPP__

#endif
