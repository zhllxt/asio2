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

#ifndef __ASIO2_TCPS_SESSION_HPP__
#define __ASIO2_TCPS_SESSION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/tcp/tcp_session.hpp>
#include <asio2/tcp/component/ssl_stream_cp.hpp>
#include <asio2/tcp/component/ssl_context_cp.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;

	template<class derived_t, class args_t>
	class tcps_session_impl_t
		: public tcp_session_impl_t<derived_t, args_t>
		, public ssl_stream_cp     <derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;

	public:
		using super = tcp_session_impl_t <derived_t, args_t>;
		using self  = tcps_session_impl_t<derived_t, args_t>;

		using key_type    = std::size_t;
		using buffer_type = typename args_t::buffer_t;

		using ssl_stream_comp = ssl_stream_cp<derived_t, args_t>;

		using super::send;

		/**
		 * @constructor
		 */
		explicit tcps_session_impl_t(
			asio::ssl::context       & ctx,
			session_mgr_t<derived_t> & sessions,
			listener_t               & listener,
			io_t                     & rwio,
			std::size_t                init_buffer_size,
			std::size_t                max_buffer_size
		)
			: super(sessions, listener, rwio, init_buffer_size, max_buffer_size)
			, ssl_stream_comp(this->io_, ctx, asio::ssl::stream_base::server)
			, ctx_(ctx)
		{
		}

		/**
		 * @destructor
		 */
		~tcps_session_impl_t()
		{
		}

	public:
		/**
		 * @function : get this object hash key,used for session map
		 */
		inline const key_type hash_key() const
		{
			return reinterpret_cast<key_type>(this);
		}

		/**
		 * @function : get the stream object refrence
		 */
		inline typename ssl_stream_comp::stream_type & stream()
		{
			ASIO2_ASSERT(bool(this->ssl_stream_));
			return (*(this->ssl_stream_));
		}

	protected:
		template<typename MatchCondition>
		inline void _do_init(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			super::_do_init(std::move(this_ptr), condition);

			this->derived()._ssl_init(condition, this->socket_, this->ctx_);
		}

		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			this->derived()._rdc_stop();

			this->derived()._ssl_stop(this_ptr, [this, ec, this_ptr]() mutable
			{
				super::_handle_disconnect(ec, std::move(this_ptr));
			});
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
	class tcps_session : public detail::tcps_session_impl_t<tcps_session, detail::template_args_tcp_session>
	{
	public:
		using tcps_session_impl_t<tcps_session, detail::template_args_tcp_session>::tcps_session_impl_t;
	};
}

#endif // !__ASIO2_TCPS_SESSION_HPP__

#endif
