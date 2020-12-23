/*
 * COPYRIGHT (C) 2017-2019, zhllxt
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

	template<class derived_t, class args_t>
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

		using key_type    = std::size_t;
		using body_type   = typename args_t::body_t;
		using buffer_type = typename args_t::buffer_t;

		using ws_stream_comp  = ws_stream_cp <derived_t, args_t>;
		using ssl_stream_comp = ssl_stream_cp<derived_t, args_t>;

		using super::send;
		using super::_data_persistence;

		/**
		 * @constructor
		 */
		explicit https_session_impl_t(
			http_router_t<derived_t> & router,
			std::filesystem::path    & root_directory,
			bool                       is_arg0_session,
			bool                       support_websocket,
			asio::ssl::context       & ctx,
			session_mgr_t<derived_t> & sessions,
			listener_t               & listener,
			io_t                     & rwio,
			std::size_t                init_buffer_size,
			std::size_t                max_buffer_size
		)
			: super(router, root_directory, is_arg0_session, support_websocket,
				sessions, listener, rwio, init_buffer_size, max_buffer_size)
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
		inline typename ssl_stream_comp::stream_type & stream()
		{
			ASIO2_ASSERT(bool(this->ssl_stream_));
			return (*(this->ssl_stream_));
		}

	public:
		/**
		 * @function : get this object hash key,used for session map
		 */
		inline const key_type hash_key() const
		{
			return reinterpret_cast<key_type>(this);
		}

	protected:
		template<typename MatchCondition>
		inline void _do_init(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			super::_do_init(std::move(this_ptr), condition);

			this->derived()._ssl_init(condition, this->socket_, this->ctx_);
		}

		template<typename MatchCondition>
		inline void _handle_connect(const error_code& ec, std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition)
		{
			this->derived().post([this, self_ptr = std::move(this_ptr), condition = std::move(condition)]() mutable
			{
				this->derived()._ssl_start(self_ptr, condition, this->socket_, this->ctx_);

				this->derived()._post_handshake(std::move(self_ptr), std::move(condition));
			});
		}

		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			this->derived()._rdc_stop();

			if (this->is_http())
			{
				this->derived()._ssl_stop(this_ptr, [this, ec, this_ptr]() mutable
				{
					super::_handle_disconnect(ec, std::move(this_ptr));
				});
			}
			else
			{
				this->derived()._ws_stop(this_ptr, [this, ec, this_ptr]() mutable
				{
					this->derived()._ssl_stop(this_ptr, [this, ec, this_ptr]()
					{
						super::super::_handle_disconnect(ec, std::move(this_ptr));
					});
				});
			}
		}

		inline void _fire_handshake(std::shared_ptr<derived_t>& this_ptr, error_code ec)
		{
			this->listener_.notify(event_type::handshake, this_ptr, ec);
		}

	protected:
		asio::ssl::context & ctx_;
	};
}

namespace asio2
{
	class https_session : public detail::https_session_impl_t<https_session, detail::template_args_https_session>
	{
	public:
		using https_session_impl_t<https_session, detail::template_args_https_session>::https_session_impl_t;
	};
}

#endif // !__ASIO2_HTTPS_SESSION_HPP__

#endif
