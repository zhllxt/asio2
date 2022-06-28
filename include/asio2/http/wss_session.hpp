/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#if defined(ASIO2_USE_SSL)

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
		using stream_t    = websocket::stream<asio::ssl::stream<asio::ip::tcp::socket&>&>;
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
		 * @constructor
		 */
		explicit wss_session_impl_t(
			asio::ssl::context       & ctx,
			session_mgr_t<derived_t> & sessions,
			listener_t               & listener,
			io_t                     & rwio,
			std::size_t                init_buf_size,
			std::size_t                max_buf_size
		)
			: super(ctx, sessions, listener, rwio, init_buf_size, max_buf_size)
			, ws_stream_cp<derived_t, args_t>()
			, ws_send_op  <derived_t, args_t>()
		{
		}

		/**
		 * @destructor
		 */
		~wss_session_impl_t()
		{
		}

	public:
		/**
		 * @function : get this object hash key,used for session map
		 */
		inline key_type hash_key() const noexcept
		{
			return reinterpret_cast<key_type>(this);
		}

	protected:
		template<typename MatchCondition>
		inline void _do_init(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			super::_do_init(std::move(this_ptr), condition);

			this->derived()._ws_init(condition, this->ssl_stream());
		}

		template<typename DeferEvent>
		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			this->derived()._rdc_stop();

			this->derived()._ws_stop(this_ptr,
				defer_event
				{
					[this, ec, this_ptr, e = chain.move_event()](event_queue_guard<derived_t> g) mutable
					{
						super::_handle_disconnect(ec, std::move(this_ptr), defer_event(std::move(e), std::move(g)));
					}, chain.move_guard()
				}
			);
		}

		template<typename MatchCondition, typename DeferEvent>
		inline void _handle_connect(const error_code& ec, std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition, DeferEvent chain)
		{
			detail::ignore_unused(ec);

			ASIO2_ASSERT(!ec);
			ASIO2_ASSERT(this->derived().sessions().io().running_in_this_thread());

			asio::dispatch(this->derived().io().context(), make_allocator(this->derived().wallocator(),
			[this, this_ptr = std::move(this_ptr), condition = std::move(condition), chain = std::move(chain)]
			() mutable
			{
				this->derived()._ssl_start(this_ptr, condition, this->socket_, this->ctx_);

				this->derived()._ws_start(this_ptr, condition, this->ssl_stream());

				this->derived()._post_handshake(std::move(this_ptr), std::move(condition), std::move(chain));
			}));
		}

		template<typename MatchCondition, typename DeferEvent>
		inline void _handle_handshake(const error_code & ec, std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition, DeferEvent chain)
		{
			ASIO2_ASSERT(this->derived().io().running_in_this_thread());

			// Use "sessions().dispatch" to ensure that the _fire_accept function and the _fire_handshake
			// function are fired in the same thread
			this->sessions().dispatch(
			[this, ec, this_ptr = std::move(this_ptr), condition = std::move(condition), chain = std::move(chain)]
			() mutable
			{
				ASIO2_ASSERT(this->derived().sessions().io().running_in_this_thread());

				try
				{
					set_last_error(ec);

					this->derived()._fire_handshake(this_ptr);

					asio::detail::throw_error(ec);

					asio::dispatch(this->io().context(), make_allocator(this->wallocator_,
					[this, this_ptr = std::move(this_ptr), condition = std::move(condition), chain = std::move(chain)]
					() mutable
					{
						ASIO2_ASSERT(this->derived().io().running_in_this_thread());

						this->derived()._post_control_callback(this_ptr, condition);
						this->derived()._post_upgrade(std::move(this_ptr), std::move(condition), std::move(chain));
					}));
				}
				catch (system_error & e)
				{
					set_last_error(e);

					this->derived()._do_disconnect(e.code(), std::move(this_ptr), std::move(chain));
				}
			});
		}

		template<class Data, class Callback>
		inline bool _do_send(Data& data, Callback&& callback)
		{
			return this->derived()._ws_send(data, std::forward<Callback>(callback));
		}

	protected:
		template<typename MatchCondition>
		inline void _post_recv(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived()._ws_post_recv(std::move(this_ptr), std::move(condition));
		}

		template<typename MatchCondition>
		inline void _handle_recv(const error_code& ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived()._ws_handle_recv(ec, bytes_recvd, std::move(this_ptr), std::move(condition));
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

#endif // !__ASIO2_WSS_SESSION_HPP__

#endif
