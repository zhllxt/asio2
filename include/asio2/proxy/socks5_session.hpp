/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_SOCKS5_SESSION_HPP__
#define __ASIO2_SOCKS5_SESSION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/tcp/tcp_session.hpp>
#include <asio2/component/socks/socks5_server_cp.hpp>
#include <asio2/proxy/socks5_client.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;

	template<class derived_t, class args_t = template_args_tcp_session>
	class socks5_session_impl_t
		: public tcp_session_impl_t<derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;

	public:
		using super = tcp_session_impl_t   <derived_t, args_t>;
		using self  = socks5_session_impl_t<derived_t, args_t>;

		using args_type   = args_t;
		using key_type    = std::size_t;
		using buffer_type = typename args_t::buffer_t;

	protected:
		using super::send;
		using super::async_send;

	public:
		/**
		 * @brief constructor
		 */
		template<class... Args>
		explicit socks5_session_impl_t(Args&&... args)
			: super(std::forward<Args>(args)...)
		{
			this->end_client_ = std::make_shared<asio2::socks5_client>(this->io_);
		}

		/**
		 * @brief destructor
		 */
		~socks5_session_impl_t()
		{
		}

	public:
		/**
		 * @brief destroy the content of all member variables, this is used for solve the memory leaks.
		 * After this function is called, this class object cannot be used again.
		 */
		inline void destroy()
		{
			derived_t& derive = this->derived();

			derive.end_client_.reset();
			derive.socks5_options_ = {};

			super::destroy();
		}

		/**
		 * @brief get this object hash key,used for session map
		 */
		inline key_type hash_key() const noexcept
		{
			return reinterpret_cast<key_type>(this);
		}

		/**
		 * @brief Set the socks5 options.
		 */
		inline derived_t& set_socks5_options(socks5::options socks5_options)
		{
			this->socks5_options_ = std::move(socks5_options);
			return this->derived();
		}

		/**
		 * @brief Set the socks5 options.
		 */
		inline socks5::options& get_socks5_options() noexcept
		{
			return this->socks5_options_;
		}

		/**
		 * @brief Get the socks5 options.
		 */
		inline const socks5::options& get_socks5_options() const noexcept
		{
			return this->socks5_options_;
		}

	protected:
		template<typename C, typename DeferEvent>
		inline void _handle_connect(
			const error_code& ec,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			detail::ignore_unused(ec);

			derived_t& derive = this->derived();

			ASIO2_ASSERT(!ec);
			ASIO2_ASSERT(derive.sessions_.io_->running_in_this_thread());

			asio::dispatch(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, &derive, this_ptr = std::move(this_ptr), ecs = std::move(ecs), chain = std::move(chain)]
			() mutable
			{
				socks5_async_handshake(derive.socket(), derive.socks5_options_,
				[&derive, this_ptr](auto& s5_server_handshake_op, auto& compose_op) mutable
				{
					derive.end_client_->bind_connect([]() mutable
					{
						set_last_error(get_last_error());
					}).bind_disconnect([this_ptr]() mutable
					{
						this_ptr->stop();
					}).bind_recv([this_ptr](std::string_view data) mutable
					{
						this_ptr->async_send(data);
					});

					auto host = s5_server_handshake_op.host;
					auto port = s5_server_handshake_op.port;

					return derive.end_client_->async_start(std::move(host), std::move(port), std::move(compose_op));
				},
				[this, &derive, this_ptr, ecs = std::move(ecs), chain = std::move(chain)]
				(const error_code& ec) mutable
				{
					derive.sessions_.dispatch(
					[this, &derive, ec, this_ptr = std::move(this_ptr), ecs = std::move(ecs), chain = std::move(chain)]
					() mutable
					{
						set_last_error(ec);

						derive._fire_socks5_handshake(this_ptr);

						super::_handle_connect(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
					});
				});
			}));
		}

		template<typename DeferEvent>
		inline void _handle_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			if (this->end_client_)
			{
				this->end_client_->stop();
				this->end_client_.reset();
			}

			super::_handle_stop(ec, std::move(this_ptr), std::move(chain));
		}

		template<typename C>
		inline void _fire_recv(
			std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>&, std::string_view data)
		{
			this->listener_.notify(event_type::recv, this_ptr, data);

			if (this->end_client_)
				this->end_client_->async_send(data);
		}

		inline void _fire_socks5_handshake(std::shared_ptr<derived_t>& this_ptr)
		{
			// the _fire_socks5_handshake must be executed in the thread 0.
			ASIO2_ASSERT(this->sessions_.io_->running_in_this_thread());

			this->listener_.notify(event_type::upgrade, this_ptr);
		}

	protected:
		socks5::options socks5_options_{};

		// create a new client, and connect to the target server.
		std::shared_ptr<asio2::socks5_client> end_client_;
	};
}

namespace asio2
{
	using socks5_session_args = detail::template_args_tcp_session;

	template<class derived_t, class args_t>
	using socks5_session_impl_t = detail::socks5_session_impl_t<derived_t, args_t>;

	/**
	 * @brief socks5 tcp session
	 */
	template<class derived_t>
	class socks5_session_t : public detail::socks5_session_impl_t<derived_t, detail::template_args_tcp_session>
	{
	public:
		using detail::socks5_session_impl_t<derived_t, detail::template_args_tcp_session>::socks5_session_impl_t;
	};

	/**
	 * @brief socks5 tcp session
	 */
	class socks5_session : public socks5_session_t<socks5_session>
	{
	public:
		using socks5_session_t<socks5_session>::socks5_session_t;
	};
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_SOCKS5_SESSION_HPP__
