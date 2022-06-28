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

#ifndef __ASIO2_HTTPS_SESSION_HPP__
#define __ASIO2_HTTPS_SESSION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/http/http_session.hpp>
#include <asio2/tcp/component/ssl_stream_cp.hpp>

namespace asio2::detail
{
	struct template_args_https_session : public template_args_http_session
	{
		using stream_t    = websocket::stream<asio::ssl::stream<asio::ip::tcp::socket&>&>;
	};

	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;

	template<class derived_t, class args_t = template_args_https_session>
	class https_session_impl_t
		: public http_session_impl_t<derived_t, args_t>
		, public ssl_stream_cp      <derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;

	public:
		using super = http_session_impl_t <derived_t, args_t>;
		using self  = https_session_impl_t<derived_t, args_t>;

		using args_type   = args_t;
		using key_type    = std::size_t;
		using body_type   = typename args_t::body_t;
		using buffer_type = typename args_t::buffer_t;

		using ws_stream_comp  = ws_stream_cp <derived_t, args_t>;
		using ssl_stream_comp = ssl_stream_cp<derived_t, args_t>;

		using super::send;
		using super::async_send;

	public:
		/**
		 * @constructor
		 */
		explicit https_session_impl_t(
			http_router_t<derived_t, args_t> & router,
			std::filesystem::path            & root_directory,
			bool                               is_arg0_session,
			bool                               support_websocket,
			asio::ssl::context               & ctx,
			session_mgr_t<derived_t>         & sessions,
			listener_t                       & listener,
			io_t                             & rwio,
			std::size_t                        init_buf_size,
			std::size_t                        max_buf_size
		)
			: super(router, root_directory, is_arg0_session, support_websocket,
				sessions, listener, rwio, init_buf_size, max_buf_size)
			, ssl_stream_comp(this->io_, ctx, asio::ssl::stream_base::server)
			, ctx_(ctx)
		{
		}

		/**
		 * @destructor
		 */
		~https_session_impl_t()
		{
		}

		/**
		 * @function : get the stream object refrence
		 */
		inline typename ssl_stream_comp::stream_type & stream() noexcept
		{
			ASIO2_ASSERT(bool(this->ssl_stream_));
			return (*(this->ssl_stream_));
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
			this->derived()._ssl_init(condition, this->socket_, this->ctx_);

			super::_do_init(std::move(this_ptr), condition);
		}

		template<typename MatchCondition, typename DeferEvent>
		inline void _handle_connect(const error_code& ec, std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition, DeferEvent chain)
		{
			detail::ignore_unused(ec);

			ASIO2_ASSERT(!ec);
			ASIO2_ASSERT(this->derived().sessions().io().running_in_this_thread());

			asio::dispatch(this->derived().io().context(), make_allocator(this->derived().wallocator(),
			[this, self_ptr = std::move(this_ptr), condition = std::move(condition), chain = std::move(chain)]
			() mutable
			{
				this->derived()._ssl_start(self_ptr, condition, this->socket_, this->ctx_);

				this->derived()._post_handshake(std::move(self_ptr), std::move(condition), std::move(chain));
			}));
		}

		template<typename DeferEvent>
		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			this->derived()._rdc_stop();

			if (this->is_http())
			{
				this->derived()._ssl_stop(this_ptr,
					defer_event
					{
						[this, ec, this_ptr, e = chain.move_event()](event_queue_guard<derived_t> g) mutable
						{
							super::_handle_disconnect(ec, std::move(this_ptr), defer_event(std::move(e), std::move(g)));
						}, chain.move_guard()
					}
				);
			}
			else
			{
				this->derived()._ws_stop(this_ptr,
					defer_event
					{
						[this, ec, this_ptr, e = chain.move_event()](event_queue_guard<derived_t> g) mutable
						{
							this->derived()._ssl_stop(this_ptr,
								defer_event
								{
									[this, ec, this_ptr, e = std::move(e)](event_queue_guard<derived_t> g) mutable
									{
										super::super::_handle_disconnect(ec,
											std::move(this_ptr), defer_event(std::move(e), std::move(g)));
									}, std::move(g)
								}
							);
						}, chain.move_guard()
					}
				);
			}
		}

		inline void _fire_handshake(std::shared_ptr<derived_t>& this_ptr)
		{
			// the _fire_handshake must be executed in the thread 0.
			ASIO2_ASSERT(this->sessions().io().running_in_this_thread());

			this->listener_.notify(event_type::handshake, this_ptr);
		}

	protected:
		asio::ssl::context & ctx_;
	};
}

namespace asio2
{
	template<class derived_t>
	class https_session_t : public detail::https_session_impl_t<derived_t, detail::template_args_https_session>
	{
	public:
		using detail::https_session_impl_t<derived_t, detail::template_args_https_session>::https_session_impl_t;
	};

	class https_session : public https_session_t<https_session>
	{
	public:
		using https_session_t<https_session>::https_session_t;
	};
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_HTTPS_SESSION_HPP__

#endif
