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

#ifndef __ASIO2_WSS_SESSION_HPP__
#define __ASIO2_WSS_SESSION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/tcp/tcps_session.hpp>

#include <asio2/http/ws_session.hpp>

namespace asio2::detail
{
	struct template_args_wss_session : public template_args_ws_session
	{
		using stream_t = websocket::stream<asio::ssl::stream<typename template_args_ws_session::socket_t&>&>;
	};

	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;

	template<class derived_t, class args_t = template_args_wss_session>
	class wss_session_impl_t
		: public tcps_session_impl_t<derived_t, args_t>
		, public ws_stream_cp       <derived_t, args_t>
		, public ws_send_op         <derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;

	public:
		using super = tcps_session_impl_t<derived_t, args_t>;
		using self  = wss_session_impl_t <derived_t, args_t>;

		using args_type   = args_t;
		using key_type    = std::size_t;
		using body_type   = typename args_t::body_t;
		using buffer_type = typename args_t::buffer_t;

		using ws_stream_comp = ws_stream_cp<derived_t, args_t>;

		using super::send;
		using super::async_send;

	public:
		/**
		 * @brief constructor
		 */
		explicit wss_session_impl_t(
			asio::ssl::context       & ctx,
			session_mgr_t<derived_t> & sessions,
			listener_t               & listener,
			std::shared_ptr<io_t>      rwio,
			std::size_t                init_buf_size,
			std::size_t                max_buf_size
		)
			: super(ctx, sessions, listener, std::move(rwio), init_buf_size, max_buf_size)
			, ws_stream_cp<derived_t, args_t>()
			, ws_send_op  <derived_t, args_t>()
		{
		}

		/**
		 * @brief destructor
		 */
		~wss_session_impl_t()
		{
		}

		/**
		 * @brief destroy the content of all member variables, this is used for solve the memory leaks.
		 * After this function is called, this class object cannot be used again.
		 */
		inline void destroy()
		{
			derived_t& derive = this->derived();

			derive.ws_stream_.reset();

			super::destroy();
		}

		/**
		 * @brief return the websocket stream object reference
		 */
		inline typename args_t::stream_t& stream() noexcept
		{
			return this->derived().ws_stream();
		}

		/**
		 * @brief return the websocket stream object reference
		 */
		inline typename args_t::stream_t const& stream() const noexcept
		{
			return this->derived().ws_stream();
		}

	public:
		/**
		 * @brief get this object hash key,used for session map
		 */
		inline key_type hash_key() const noexcept
		{
			return reinterpret_cast<key_type>(this);
		}

		/**
		 * @brief get the websocket upgraged request object
		 */
		inline       websocket::request_type& get_upgrade_request()      noexcept { return this->upgrade_req_; }

		/**
		 * @brief get the websocket upgraged request object
		 */
		inline const websocket::request_type& get_upgrade_request() const noexcept { return this->upgrade_req_; }

	protected:
		inline typename super::ssl_stream_type& upgrade_stream() noexcept
		{
			return this->ssl_stream();
		}
		inline typename super::ssl_stream_type const& upgrade_stream() const noexcept
		{
			return this->ssl_stream();
		}

		template<typename C>
		inline void _do_init(std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs)
		{
			super::_do_init(this_ptr, ecs);

			this->derived()._ws_init(ecs, this->derived().ssl_stream());
		}

		template<typename DeferEvent>
		inline void _post_shutdown(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			ASIO2_LOG_DEBUG("wss_session::_post_shutdown: {} {}", ec.value(), ec.message());

			this->derived()._ws_stop(this_ptr, defer_event
			{
				[this, ec, this_ptr, e = chain.move_event()](event_queue_guard<derived_t> g) mutable
				{
					super::_post_shutdown(ec, std::move(this_ptr), defer_event(std::move(e), std::move(g)));
				}, chain.move_guard()
			});
		}

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
			[&derive, this_ptr = std::move(this_ptr), ecs = std::move(ecs), chain = std::move(chain)]
			() mutable
			{
				derive._ssl_start(this_ptr, ecs, derive.socket(), derive.ctx_);

				derive._ws_start(this_ptr, ecs, derive.ssl_stream());

				derive._post_handshake(std::move(this_ptr), std::move(ecs), std::move(chain));
			}));
		}

		template<typename C, typename DeferEvent>
		inline void _handle_handshake(
			const error_code& ec,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			ASIO2_ASSERT(this->derived().io_->running_in_this_thread());

			// Use "sessions_.dispatch" to ensure that the _fire_accept function and the _fire_handshake
			// function are fired in the same thread
			this->sessions_.dispatch(
			[this, ec, this_ptr = std::move(this_ptr), ecs = std::move(ecs), chain = std::move(chain)]
			() mutable
			{
				ASIO2_ASSERT(this->derived().sessions_.io_->running_in_this_thread());

				set_last_error(ec);

				this->derived()._fire_handshake(this_ptr);

				if (ec)
				{
					this->derived()._do_disconnect(ec, std::move(this_ptr), std::move(chain));

					return;
				}

				asio::dispatch(this->io_->context(), make_allocator(this->wallocator_,
				[this, this_ptr = std::move(this_ptr), ecs = std::move(ecs), chain = std::move(chain)]
				() mutable
				{
					ASIO2_ASSERT(this->derived().io_->running_in_this_thread());

					this->derived()._post_read_upgrade_request(
						std::move(this_ptr), std::move(ecs), std::move(chain));
				}));
			});
		}

		template<class Data, class Callback>
		inline bool _do_send(Data& data, Callback&& callback)
		{
			return this->derived()._ws_send(data, std::forward<Callback>(callback));
		}

	protected:
		template<typename C>
		inline void _post_recv(std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			this->derived()._ws_post_recv(std::move(this_ptr), std::move(ecs));
		}

		template<typename C>
		inline void _handle_recv(
			const error_code& ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			this->derived()._ws_handle_recv(ec, bytes_recvd, std::move(this_ptr), std::move(ecs));
		}

		inline void _fire_upgrade(std::shared_ptr<derived_t>& this_ptr)
		{
			// the _fire_upgrade must be executed in the thread 0.
			ASIO2_ASSERT(this->sessions_.io_->running_in_this_thread());

			this->listener_.notify(event_type::upgrade, this_ptr);
		}

	protected:
		websocket::request_type                   upgrade_req_;
	};
}

namespace asio2
{
	using wss_session_args = detail::template_args_wss_session;

	template<class derived_t, class args_t>
	using wss_session_impl_t = detail::wss_session_impl_t<derived_t, args_t>;

	template<class derived_t>
	class wss_session_t : public detail::wss_session_impl_t<derived_t, detail::template_args_wss_session>
	{
	public:
		using detail::wss_session_impl_t<derived_t, detail::template_args_wss_session>::wss_session_impl_t;
	};

	class wss_session : public wss_session_t<wss_session>
	{
	public:
		using wss_session_t<wss_session>::wss_session_t;
	};
}

#include <asio2/base/detail/pop_options.hpp>

#if defined(ASIO2_INCLUDE_RATE_LIMIT)
#include <asio2/tcp/tcp_stream.hpp>
namespace asio2
{
	struct wss_rate_session_args : public wss_session_args
	{
		using socket_t = asio2::tcp_stream<asio2::simple_rate_policy>;
		using stream_t = websocket::stream<asio::ssl::stream<socket_t&>&>;
	};

	template<class derived_t>
	class wss_rate_session_t : public asio2::wss_session_impl_t<derived_t, wss_rate_session_args>
	{
	public:
		using asio2::wss_session_impl_t<derived_t, wss_rate_session_args>::wss_session_impl_t;
	};

	class wss_rate_session : public asio2::wss_rate_session_t<wss_rate_session>
	{
	public:
		using asio2::wss_rate_session_t<wss_rate_session>::wss_rate_session_t;
	};
}
#endif

#endif // !__ASIO2_WSS_SESSION_HPP__

#endif
