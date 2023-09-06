/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_SOCKS5_TRANSFER_HPP__
#define __ASIO2_SOCKS5_TRANSFER_HPP__

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
	class socks5_transfer_impl_t
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
		using self  = socks5_transfer_impl_t<derived_t, executor_t>;

		using executor_type = executor_t;

		using args_type = typename executor_t::args_type;

		using super::async_start;

	public:
		/**
		 * @brief constructor
		 */
		template<class... Args>
		explicit socks5_transfer_impl_t(Args&&... args)
			: super(std::forward<Args>(args)...)
		{
			this->connect_finish_timer_ = std::make_shared<asio::steady_timer>(this->io_->context());
		}

		/**
		 * @brief destructor
		 */
		~socks5_transfer_impl_t()
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

		/**
		 * @brief async start the client, asynchronous connect to server.
		 * @param host - A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port - A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt, typename CompletionToken, typename... Args>
		auto async_start(String&& host, StrOrInt&& port, CompletionToken&& token, Args&&... args)
		{
			derived_t& derive = this->derived();

			bool f = executor_t::template async_start(
				std::forward<String>(host), std::forward<StrOrInt>(port), std::forward<Args>(args)...);

			derive.connect_finish_timer_->expires_after(f ?
				std::chrono::steady_clock::duration::max() :
				std::chrono::steady_clock::duration::zero());

			return asio::async_compose<CompletionToken, void(asio::error_code)>(
				detail::wait_timer_op{*(derive.connect_finish_timer_)},
				token, derive.socket());
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
	using socks5_transfer_impl_t = detail::socks5_transfer_impl_t<derived_t, executor_t>;

	/**
	 * @brief socks5 tcp transfer
	 */
	template<class derived_t>
	class socks5_tcp_transfer_t : public detail::socks5_transfer_impl_t<derived_t, tcp_client_t<derived_t>>
	{
	public:
		using detail::socks5_transfer_impl_t<derived_t, tcp_client_t<derived_t>>::socks5_transfer_impl_t;
	};

	/**
	 * @brief socks5 tcp transfer
	 */
	class socks5_tcp_transfer : public socks5_tcp_transfer_t<socks5_tcp_transfer>
	{
	public:
		using socks5_tcp_transfer_t<socks5_tcp_transfer>::socks5_tcp_transfer_t;
	};

	/**
	 * @brief socks5 udp transfer
	 */
	template<class derived_t>
	class socks5_udp_transfer_t : public detail::socks5_transfer_impl_t<derived_t, udp_cast_t<derived_t>>
	{
	public:
		using detail::socks5_transfer_impl_t<derived_t, udp_cast_t<derived_t>>::socks5_transfer_impl_t;
	};

	/**
	 * @brief socks5 udp transfer
	 */
	class socks5_udp_transfer : public socks5_udp_transfer_t<socks5_udp_transfer>
	{
	public:
		using socks5_udp_transfer_t<socks5_udp_transfer>::socks5_udp_transfer_t;
	};
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_SOCKS5_TRANSFER_HPP__
