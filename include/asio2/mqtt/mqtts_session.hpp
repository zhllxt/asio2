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

#ifndef __ASIO2_MQTTS_SESSION_HPP__
#define __ASIO2_MQTTS_SESSION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/mqtt/mqtt_session.hpp>
#include <asio2/tcp/impl/ssl_stream_cp.hpp>

namespace asio2::detail
{
	struct template_args_mqtts_session : public template_args_mqtt_session
	{
		using stream_t = asio::ssl::stream<typename template_args_mqtt_session::socket_t&>;
	};

	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;

	template<class derived_t, class args_t = template_args_mqtts_session>
	class mqtts_session_impl_t
		: public mqtt_session_impl_t<derived_t, args_t>
		, public ssl_stream_cp      <derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;

	public:
		using super = mqtt_session_impl_t <derived_t, args_t>;
		using self  = mqtts_session_impl_t<derived_t, args_t>;

		using args_type   = args_t;
		using key_type    = std::size_t;

		using ssl_stream_comp = ssl_stream_cp<derived_t, args_t>;

		using super::send;
		using super::async_send;

	public:
		/**
		 * @brief constructor
		 */
		explicit mqtts_session_impl_t(
			asio::ssl::context       & ctx,
			mqtt::broker_state<derived_t, args_t>& broker_state,
			session_mgr_t <derived_t>& sessions,
			listener_t               & listener,
			std::shared_ptr<io_t>      rwio,
			std::size_t                init_buf_size,
			std::size_t                max_buf_size
		)
			: super(broker_state, sessions, listener, std::move(rwio), init_buf_size, max_buf_size)
			, ssl_stream_comp(ctx, asio::ssl::stream_base::server)
			, ctx_(ctx)
		{
		}

		/**
		 * @brief destructor
		 */
		~mqtts_session_impl_t()
		{
		}

		/**
		 * @brief destroy the content of all member variables, this is used for solve the memory leaks.
		 * After this function is called, this class object cannot be used again.
		 */
		inline void destroy()
		{
			derived_t& derive = this->derived();

			derive.ssl_stream_.reset();

			super::destroy();
		}

		/**
		 * @brief get the stream object reference
		 */
		inline typename ssl_stream_comp::ssl_stream_type & stream() noexcept
		{
			return this->derived().ssl_stream();
		}

		/**
		 * @brief get the stream object reference
		 */
		inline typename ssl_stream_comp::ssl_stream_type const& stream() const noexcept
		{
			return this->derived().ssl_stream();
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

			this->derived()._ssl_init(ecs, this->derived().socket(), this->ctx_);
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

				super::_handle_connect(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
			});
		}

		template<typename DeferEvent>
		inline void _post_shutdown(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			this->derived()._ssl_stop(this_ptr, defer_event
			{
				[this, ec, this_ptr, e = chain.move_event()](event_queue_guard<derived_t> g) mutable
				{
					super::_post_shutdown(ec, std::move(this_ptr), defer_event(std::move(e), std::move(g)));
				}, chain.move_guard()
			});
		}

		inline void _fire_handshake(std::shared_ptr<derived_t>& this_ptr)
		{
			// the _fire_handshake must be executed in the thread 0.
			ASIO2_ASSERT(this->sessions_.io_->running_in_this_thread());

			this->listener_.notify(event_type::handshake, this_ptr);
		}

	protected:
		asio::ssl::context & ctx_;
	};
}

namespace asio2
{
	using mqtts_session_args = detail::template_args_mqtts_session;

	template<class derived_t, class args_t>
	using mqtts_session_impl_t = detail::mqtts_session_impl_t<derived_t, args_t>;

	template<class derived_t>
	class mqtts_session_t : public detail::mqtts_session_impl_t<derived_t, detail::template_args_mqtts_session>
	{
	public:
		using detail::mqtts_session_impl_t<derived_t, detail::template_args_mqtts_session>::mqtts_session_impl_t;
	};

	class mqtts_session : public mqtts_session_t<mqtts_session>
	{
	public:
		using mqtts_session_t<mqtts_session>::mqtts_session_t;
	};
}

#if defined(ASIO2_INCLUDE_RATE_LIMIT)
#include <asio2/tcp/tcp_stream.hpp>
namespace asio2
{
	struct mqtts_rate_session_args : public mqtts_session_args
	{
		using socket_t = asio2::tcp_stream<asio2::simple_rate_policy>;
		using stream_t = asio::ssl::stream<socket_t&>;
	};

	template<class derived_t>
	class mqtts_rate_session_t : public asio2::mqtts_session_impl_t<derived_t, mqtts_rate_session_args>
	{
	public:
		using asio2::mqtts_session_impl_t<derived_t, mqtts_rate_session_args>::mqtts_session_impl_t;
	};

	class mqtts_rate_session : public asio2::mqtts_rate_session_t<mqtts_rate_session>
	{
	public:
		using asio2::mqtts_rate_session_t<mqtts_rate_session>::mqtts_rate_session_t;
	};
}
#endif

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_MQTTS_SESSION_HPP__

#endif
