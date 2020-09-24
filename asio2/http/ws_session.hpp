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
	template <class>                             class session_mgr_t;
	template <class, class>                      class tcp_server_impl_t;
	template <class, class>                      class ws_server_impl_t;

	template<class derived_t, class socket_t, class stream_t, class body_t, class buffer_t>
	class ws_session_impl_t
		: public tcp_session_impl_t<derived_t, socket_t, buffer_t>
		, public ws_stream_cp<derived_t, stream_t, true>
		, public ws_send_op<derived_t, true>
	{
		template <class, bool>                       friend class user_timer_cp;
		template <class>                             friend class post_cp;
		template <class, class, bool>                friend class connect_cp;
		template <class, class, bool>                friend class disconnect_cp;
		template <class>                             friend class data_persistence_cp;
		template <class>                             friend class event_queue_cp;
		template <class, bool>                       friend class send_cp;
		template <class, bool>                       friend class silence_timer_cp;
		template <class, bool>                       friend class connect_timeout_cp;
		template <class, bool>                       friend class tcp_send_op;
		template <class, bool>                       friend class tcp_recv_op;
		template <class, class, bool>                friend class ws_stream_cp;
		template <class, bool>                       friend class ws_send_op;
		template <class>                             friend class session_mgr_t;
		template <class, class, class>               friend class session_impl_t;
		template <class, class, class>               friend class tcp_session_impl_t;
		template <class, class>                      friend class tcp_server_impl_t;
		template <class, class>                      friend class ws_server_impl_t;

	public:
		using self = ws_session_impl_t<derived_t, socket_t, stream_t, body_t, buffer_t>;
		using super = tcp_session_impl_t<derived_t, socket_t, buffer_t>;
		using body_type = body_t;
		using key_type = std::size_t;
		using buffer_type = buffer_t;
		using ws_stream_comp = ws_stream_cp<derived_t, stream_t, true>;
		using super::send;

		/**
		 * @constructor
		 */
		explicit ws_session_impl_t(
			session_mgr_t<derived_t> & sessions,
			listener_t & listener,
			io_t & rwio,
			std::size_t init_buffer_size,
			std::size_t max_buffer_size
		)
			: super(sessions, listener, rwio, init_buffer_size, max_buffer_size)
			, ws_stream_comp()
			, ws_send_op<derived_t, true>()
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
				[this, self_ptr = std::move(this_ptr), condition]() mutable
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

		inline void _fire_upgrade(std::shared_ptr<derived_t>& this_ptr, error_code ec)
		{
			this->listener_.notify(event::upgrade, this_ptr, ec);
		}

	protected:
	};
}

namespace asio2
{
	class ws_session : public detail::ws_session_impl_t<ws_session, asio::ip::tcp::socket,
		websocket::stream<asio::ip::tcp::socket&>, http::string_body, beast::flat_buffer>
	{
	public:
		using ws_session_impl_t<ws_session, asio::ip::tcp::socket,
			websocket::stream<asio::ip::tcp::socket&>,
			http::string_body, beast::flat_buffer>::ws_session_impl_t;
	};
}

#endif // !__ASIO2_WS_SESSION_HPP__
