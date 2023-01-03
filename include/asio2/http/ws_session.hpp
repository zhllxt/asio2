/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_WS_SESSION_HPP__
#define __ASIO2_WS_SESSION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/tcp/tcp_session.hpp>
#include <asio2/http/impl/ws_stream_cp.hpp>
#include <asio2/http/impl/ws_send_op.hpp>
#include <asio2/http/request.hpp>
#include <asio2/http/response.hpp>

namespace asio2::detail
{
	struct template_args_ws_session : public template_args_tcp_session
	{
		using stream_t    = websocket::stream<typename template_args_tcp_session::socket_t&>;
		using body_t      = http::string_body;
		using buffer_t    = beast::flat_buffer;
	};

	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;

	template<class derived_t, class args_t = template_args_ws_session>
	class ws_session_impl_t
		: public tcp_session_impl_t<derived_t, args_t>
		, public ws_stream_cp      <derived_t, args_t>
		, public ws_send_op        <derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;

	public:
		using super = tcp_session_impl_t<derived_t, args_t>;
		using self  = ws_session_impl_t <derived_t, args_t>;

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
		explicit ws_session_impl_t(
			session_mgr_t<derived_t> & sessions,
			listener_t               & listener,
			io_t                     & rwio,
			std::size_t                init_buf_size,
			std::size_t                max_buf_size
		)
			: super(sessions, listener, rwio, init_buf_size, max_buf_size)
			, ws_stream_cp<derived_t, args_t>()
			, ws_send_op  <derived_t, args_t>()
		{
		}

		/**
		 * @brief destructor
		 */
		~ws_session_impl_t()
		{
		}

		/**
		 * @brief return the websocket stream object refrence
		 */
		inline typename args_t::stream_t& stream() noexcept
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

	protected:
		template<typename C>
		inline void _do_init(std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs)
		{
			super::_do_init(this_ptr, ecs);

			this->derived()._ws_init(ecs, this->socket_);
		}

		template<typename DeferEvent>
		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			this->derived()._ws_stop(this_ptr,
				defer_event
				{
					[this, ec, this_ptr, e = chain.move_event()] (event_queue_guard<derived_t> g) mutable
					{
						super::_handle_disconnect(ec, std::move(this_ptr), defer_event(std::move(e), std::move(g)));
					}, chain.move_guard()
				}
			);
		}

		template<typename C, typename DeferEvent>
		inline void _handle_connect(
			const error_code& ec,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			detail::ignore_unused(ec);

			ASIO2_ASSERT(!ec);
			ASIO2_ASSERT(this->derived().sessions().io().running_in_this_thread());

			asio::dispatch(this->io().context(), make_allocator(this->wallocator_,
			[this, this_ptr = std::move(this_ptr), ecs = std::move(ecs), chain = std::move(chain)]
			() mutable
			{
				this->derived()._ws_start(this_ptr, ecs, this->socket_);

				this->derived()._post_control_callback(this_ptr, ecs);
				this->derived()._post_upgrade(std::move(this_ptr), std::move(ecs), std::move(chain));
			}));
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
			ASIO2_ASSERT(this->sessions().io().running_in_this_thread());

			this->listener_.notify(event_type::upgrade, this_ptr);
		}

	protected:
	};
}

namespace asio2
{
	using ws_session_args = detail::template_args_ws_session;

	template<class derived_t, class args_t>
	using ws_session_impl_t = detail::ws_session_impl_t<derived_t, args_t>;

	template<class derived_t>
	class ws_session_t : public detail::ws_session_impl_t<derived_t, detail::template_args_ws_session>
	{
	public:
		using detail::ws_session_impl_t<derived_t, detail::template_args_ws_session>::ws_session_impl_t;
	};

	class ws_session : public ws_session_t<ws_session>
	{
	public:
		using ws_session_t<ws_session>::ws_session_t;
	};
}

#if defined(ASIO2_INCLUDE_RATE_LIMIT)
#include <asio2/tcp/tcp_stream.hpp>
namespace asio2
{
	struct ws_rate_session_args : public ws_session_args
	{
		using socket_t = asio2::tcp_stream<asio2::simple_rate_policy>;
		using stream_t = websocket::stream<socket_t&>;
	};

	template<class derived_t>
	class ws_rate_session_t : public asio2::ws_session_impl_t<derived_t, ws_rate_session_args>
	{
	public:
		using asio2::ws_session_impl_t<derived_t, ws_rate_session_args>::ws_session_impl_t;
	};

	class ws_rate_session : public asio2::ws_rate_session_t<ws_rate_session>
	{
	public:
		using asio2::ws_rate_session_t<ws_rate_session>::ws_rate_session_t;
	};
}
#endif

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_WS_SESSION_HPP__
