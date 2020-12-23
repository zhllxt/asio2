/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_WS_SESSION_HPP__
#define __ASIO2_WS_SESSION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/tcp/tcp_session.hpp>
#include <asio2/http/component/ws_stream_cp.hpp>
#include <asio2/http/impl/ws_send_op.hpp>
#include <asio2/http/request.hpp>
#include <asio2/http/response.hpp>

namespace asio2::detail
{
	struct template_args_ws_session : public template_args_tcp_session
	{
		using stream_t    = websocket::stream<asio::ip::tcp::socket&>;
		using body_t      = http::string_body;
		using buffer_t    = beast::flat_buffer;
	};

	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;

	template<class derived_t, class args_t>
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

		using key_type    = std::size_t;
		using body_type   = typename args_t::body_t;
		using buffer_type = typename args_t::buffer_t;

		using ws_stream_comp = ws_stream_cp<derived_t, args_t>;

		using super::send;

		/**
		 * @constructor
		 */
		explicit ws_session_impl_t(
			session_mgr_t<derived_t> & sessions,
			listener_t               & listener,
			io_t                     & rwio,
			std::size_t                init_buffer_size,
			std::size_t                max_buffer_size
		)
			: super(sessions, listener, rwio, init_buffer_size, max_buffer_size)
			, ws_stream_cp<derived_t, args_t>()
			, ws_send_op  <derived_t, args_t>()
		{
		}

		/**
		 * @destructor
		 */
		~ws_session_impl_t()
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

	protected:
		template<typename MatchCondition>
		inline void _do_init(std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition)
		{
			super::_do_init(std::move(this_ptr), condition);

			this->derived()._ws_init(condition, this->socket_);
		}

		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			this->derived()._rdc_stop();

			this->derived()._ws_stop(this_ptr, [this, ec, this_ptr]()
			{
				super::_handle_disconnect(ec, std::move(this_ptr));
			});
		}

		template<typename MatchCondition>
		inline void _handle_connect(const error_code& ec, std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition)
		{
			asio::post(this->io_.strand(), make_allocator(this->rallocator_,
				[this, self_ptr = std::move(this_ptr), condition = std::move(condition)]() mutable
			{
				this->derived()._ws_start(self_ptr, condition, this->socket_);

				this->derived()._post_control_callback(self_ptr, condition);
				this->derived()._post_upgrade(std::move(self_ptr), std::move(condition));
			}));
		}

		template<class Data, class Callback>
		inline bool _do_send(Data& data, Callback&& callback)
		{
			return this->derived()._ws_send(data, std::forward<Callback>(callback));
		}

	protected:
		template<typename MatchCondition>
		inline void _post_recv(std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition)
		{
			this->derived()._ws_post_recv(std::move(this_ptr), std::move(condition));
		}

		template<typename MatchCondition>
		inline void _handle_recv(const error_code& ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived()._ws_handle_recv(ec, bytes_recvd, std::move(this_ptr), std::move(condition));
		}

		inline void _fire_upgrade(std::shared_ptr<derived_t>& this_ptr, error_code ec)
		{
			this->listener_.notify(event_type::upgrade, this_ptr, ec);
		}

	protected:
	};
}

namespace asio2
{
	class ws_session : public detail::ws_session_impl_t<ws_session, detail::template_args_ws_session>
	{
	public:
		using ws_session_impl_t<ws_session, detail::template_args_ws_session>::ws_session_impl_t;
	};
}

#endif // !__ASIO2_WS_SESSION_HPP__
