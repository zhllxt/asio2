/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_SOCKS5_CLIENT_HPP__
#define __ASIO2_SOCKS5_CLIENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/tcp/tcp_client.hpp>
#include <asio2/udp/udp_cast.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_UDP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;
	ASIO2_CLASS_FORWARD_DECLARE_UDP_CLIENT;

	template<class, class> class socks5_session_impl_t;

	template<class derived_t, class executor_t>
	class socks5_client_impl_t
		: public executor_t
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_UDP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;
		ASIO2_CLASS_FRIEND_DECLARE_UDP_CLIENT;

		template<class, class> friend class socks5_session_impl_t;

	public:
		using super = executor_t;
		using self  = socks5_client_impl_t<derived_t, executor_t>;

		using executor_type = executor_t;

		using args_type = typename executor_t::args_type;

	public:
		/**
		 * @brief constructor
		 */
		template<class... Args>
		explicit socks5_client_impl_t(Args&&... args)
			: super(std::forward<Args>(args)...)
		{
			this->connect_finish_timer_ = std::make_shared<asio::steady_timer>(this->io_->context());
		}

		/**
		 * @brief destructor
		 */
		~socks5_client_impl_t()
		{
			this->stop();
		}

		/**
		 * @brief destroy the content of all member variables, this is used for solve the memory leaks.
		 * After this function is called, this class object cannot be used again.
		 */
		inline void destroy()
		{
			derived_t& derive = this->derived();

			derive.connect_finish_timer_.reset();

			super::destroy();
		}

	protected:
		template<typename C, typename DeferEvent>
		inline void _handle_connect(
			const error_code& ec,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			detail::cancel_timer(*(this->derived().connect_finish_timer_));

			super::_handle_connect(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
		}

	protected:
		std::shared_ptr<asio::steady_timer> connect_finish_timer_;
	};
}

namespace asio2
{
	template<class derived_t, class executor_t>
	using socks5_client_impl_t = detail::socks5_client_impl_t<derived_t, executor_t>;

	/**
	 * @brief socks5 tcp client
	 */
	template<class derived_t>
	class socks5_tcp_client_t : public detail::socks5_client_impl_t<derived_t, tcp_client_t<derived_t>>
	{
	public:
		using detail::socks5_client_impl_t<derived_t, tcp_client_t<derived_t>>::socks5_client_impl_t;
	};

	/**
	 * @brief socks5 tcp client
	 */
	class socks5_tcp_client : public socks5_tcp_client_t<socks5_tcp_client>
	{
	public:
		using socks5_tcp_client_t<socks5_tcp_client>::socks5_tcp_client_t;
	};

	/**
	 * @brief socks5 udp client
	 */
	template<class derived_t>
	class socks5_udp_client_t : public detail::socks5_client_impl_t<derived_t, udp_cast_t<derived_t>>
	{
	public:
		using detail::socks5_client_impl_t<derived_t, udp_cast_t<derived_t>>::socks5_client_impl_t;
	};

	/**
	 * @brief socks5 udp client
	 */
	class socks5_udp_client : public socks5_udp_client_t<socks5_udp_client>
	{
	public:
		using socks5_udp_client_t<socks5_udp_client>::socks5_udp_client_t;
	};
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_SOCKS5_CLIENT_HPP__
