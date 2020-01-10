/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef ASIO_STANDALONE

#ifndef __ASIO2_HTTP_SESSION_HPP__
#define __ASIO2_HTTP_SESSION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/tcp/tcp_session.hpp>
#include <asio2/http/detail/http_util.hpp>
#include <asio2/http/component/http_send_cp.hpp>
#include <asio2/http/impl/http_send_op.hpp>
#include <asio2/http/impl/http_recv_op.hpp>

namespace asio2::detail
{
	template <class>                      class session_mgr_t;
	template <class, class>               class tcp_server_impl_t;
	template <class, class>               class http_server_impl_t;

	template<class derived_t, class socket_t, class body_t, class buffer_t>
	class http_session_impl_t
		: public tcp_session_impl_t<derived_t, socket_t, buffer_t>
		, public http_send_cp<derived_t, body_t, buffer_t, true>
		, public http_send_op<derived_t, body_t, buffer_t, true>
		, public http_recv_op<derived_t, body_t, buffer_t, true>
	{
		template <class, bool>                friend class user_timer_cp;
		template <class>                      friend class data_persistence_cp;
		template <class>                      friend class event_queue_cp;
		template <class, bool>                friend class send_cp;
		template <class, bool>                friend class silence_timer_cp;
		template <class, bool>                friend class connect_timeout_cp;
		template <class, bool>                friend class tcp_send_op;
		template <class, bool>                friend class tcp_recv_op;
		template <class, class, class, bool>  friend class http_send_cp;
		template <class, class, class, bool>  friend class http_send_op;
		template <class, class, class, bool>  friend class http_recv_op;
		template <class>                      friend class session_mgr_t;
		template <class, class, class>        friend class session_impl_t;
		template <class, class, class>        friend class tcp_session_impl_t;
		template <class, class>               friend class tcp_server_impl_t;
		template <class, class>               friend class http_server_impl_t;

	public:
		using self = http_session_impl_t<derived_t, socket_t, body_t, buffer_t>;
		using super = tcp_session_impl_t<derived_t, socket_t, buffer_t>;
		using key_type = std::size_t;
		using body_type = body_t;
		using buffer_type = buffer_t;
		using super::send;
		using http_send_cp<derived_t, body_t, buffer_t, true>::send;
		using data_persistence_cp<derived_t>::_data_persistence;

		/**
		 * @constructor
		 */
		explicit http_session_impl_t(
			session_mgr_t<derived_t> & sessions,
			listener_t & listener,
			io_t & rwio,
			std::size_t init_buffer_size,
			std::size_t max_buffer_size
		)
			: super(sessions, listener, rwio, init_buffer_size, max_buffer_size)
			, http_send_cp<derived_t, body_t, buffer_t, true>(rwio)
			, http_send_op<derived_t, body_t, buffer_t, true>()
		{
			this->silence_timeout_ = std::chrono::milliseconds(http_silence_timeout);
		}

		/**
		 * @destructor
		 */
		~http_session_impl_t()
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
		template<class T>
		inline auto _data_persistence(T&& data)
		{
			return this->derived()._data_persistence(asio::buffer(data));
		}

		template<class CharT, class SizeT>
		inline auto _data_persistence(CharT * s, SizeT count)
		{
			return this->derived()._data_persistence(asio::buffer((const void*)s, count * sizeof(CharT)));
		}

		template<typename = void>
		inline auto _data_persistence(asio::const_buffer&& data)
		{
			return copyable_wrapper(http::make_response<body_type>(std::string_view(
				reinterpret_cast<std::string_view::const_pointer>(data.data()), data.size())));
		}

		template<bool isRequest, class Body, class Fields = http::fields>
		inline auto _data_persistence(http::message<isRequest, Body, Fields>& msg)
		{
			return this->derived()._data_persistence(const_cast<const http::message<isRequest, Body, Fields>&>(msg));
		}

		template<bool isRequest, class Body, class Fields = http::fields>
		inline auto _data_persistence(const http::message<isRequest, Body, Fields>& msg)
		{
			return copyable_wrapper(std::move(msg));
		}

		template<bool isRequest, class Body, class Fields = http::fields>
		inline auto _data_persistence(http::message<isRequest, Body, Fields>&& msg)
		{
			return copyable_wrapper(std::move(msg));
		}

		template<class Data, class Callback>
		inline bool _do_send(Data& data, Callback&& callback)
		{
			return this->derived().template _http_send<false>(this->derived().stream(),
				data, std::forward<Callback>(callback));
		}

	protected:
		template<typename MatchCondition>
		inline void _post_recv(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived()._http_post_recv(std::move(this_ptr), condition);
		}

		template<typename MatchCondition>
		inline void _handle_recv(const error_code& ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived()._http_handle_recv(ec, bytes_recvd, std::move(this_ptr), condition);
		}

		inline void _fire_recv(std::shared_ptr<derived_t>& this_ptr, http::request<body_t> & req)
		{
			this->listener_.notify(event::recv, this_ptr, req);
		}

	protected:
		http::request<body_t> req_;
	};
}

namespace asio2
{
	class http_session : public detail::http_session_impl_t<http_session, asio::ip::tcp::socket, http::string_body, beast::flat_buffer>
	{
	public:
		using http_session_impl_t<http_session, asio::ip::tcp::socket, http::string_body, beast::flat_buffer>::http_session_impl_t;
	};
}

#endif // !__ASIO2_HTTP_SESSION_HPP__

#endif
