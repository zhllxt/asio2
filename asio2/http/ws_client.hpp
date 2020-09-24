/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_WS_CLIENT_HPP__
#define __ASIO2_WS_CLIENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/tcp/tcp_client.hpp>
#include <asio2/http/detail/http_util.hpp>
#include <asio2/http/component/ws_stream_cp.hpp>
#include <asio2/http/impl/ws_send_op.hpp>

namespace asio2::detail
{
	template<class derived_t, class socket_t, class stream_t, class body_t, class buffer_t>
	class ws_client_impl_t
		: public tcp_client_impl_t<derived_t, socket_t, buffer_t>
		, public ws_stream_cp<derived_t, stream_t, false>
		, public ws_send_op<derived_t, false>
	{
		template <class, bool>                       friend class user_timer_cp;
		template <class>                             friend class post_cp;
		template <class, bool>                       friend class reconnect_timer_cp;
		template <class, bool>                       friend class connect_timeout_cp;
		template <class, class, bool>                friend class connect_cp;
		template <class, class, bool>                friend class disconnect_cp;
		template <class>                             friend class data_persistence_cp;
		template <class>                             friend class event_queue_cp;
		template <class, bool>                       friend class send_cp;
		template <class, bool>                       friend class tcp_send_op;
		template <class, bool>                       friend class tcp_recv_op;
		template <class, class, bool>                friend class ws_stream_cp;
		template <class, bool>                       friend class ws_send_op;
		template <class, class, class>               friend class client_impl_t;
		template <class, class, class>               friend class tcp_client_impl_t;

	public:
		using self = ws_client_impl_t<derived_t, socket_t, stream_t, body_t, buffer_t>;
		using super = tcp_client_impl_t<derived_t, socket_t, buffer_t>;
		using body_type = body_t;
		using buffer_type = buffer_t;
		using ws_stream_comp = ws_stream_cp<derived_t, stream_t, false>;
		using super::send;

	public:
		/**
		 * @constructor
		 */
		explicit ws_client_impl_t(
			std::size_t init_buffer_size = tcp_frame_size,
			std::size_t max_buffer_size = (std::numeric_limits<std::size_t>::max)()
		)
			: super(init_buffer_size, max_buffer_size)
			, ws_stream_comp()
			, ws_send_op<derived_t, false>()
		{
		}

		/**
		 * @destructor
		 */
		~ws_client_impl_t()
		{
			this->stop();
		}

		/**
		 * @function : start the client, blocking connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt>
		bool start(String&& host, StrOrInt&& port)
		{
			return this->derived().template _do_connect<false>(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				condition_wrap<void>{});
		}

		/**
		 * @function : start the client, asynchronous connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt>
		bool async_start(String&& host, StrOrInt&& port)
		{
			return this->derived().template _do_connect<true>(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				condition_wrap<void>{});
		}

		/**
		 * @function : get the websocket upgraged response object
		 */
		inline const http::response_t<body_t>& upgrade_response() { return this->upgrade_rep_; }

		/**
		 * @function : get the websocket upgraged target
		 */
		inline const std::string& upgrade_target() { return this->upgrade_target_; }

		/**
		 * @function : set the websocket upgraged target
		 */
		inline derived_t & upgrade_target(std::string target)
		{
			this->upgrade_target_ = std::move(target);
			return (this->derived());
		}

	public:
		/**
		 * @function : bind websocket upgrade listener
		 * @param    : fun - a user defined callback function
		 * Function signature : void(asio::error_code ec)
		 */
		template<class F, class ...C>
		inline derived_t & bind_upgrade(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::upgrade,
				observer_t<error_code>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	protected:
		template<typename MatchCondition>
		inline void _do_init(condition_wrap<MatchCondition> condition)
		{
			super::_do_init(condition);

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
		inline void _handle_connect(const error_code & ec, std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition)
		{
			set_last_error(ec);

			if (ec)
				return this->derived()._done_connect(ec, std::move(this_ptr), std::move(condition));

			this->derived()._ws_start(this_ptr, condition, this->socket_);

			this->derived()._post_control_callback(this_ptr, condition);
			this->derived()._post_upgrade(std::move(this_ptr), std::move(condition), this->upgrade_rep_);
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

		inline void _fire_upgrade(detail::ignore, error_code ec)
		{
			this->listener_.notify(event::upgrade, ec);
		}

	protected:
		http::response_t<body_t> upgrade_rep_;

		std::string              upgrade_target_ = "/";
	};
}

namespace asio2
{
	class ws_client : public detail::ws_client_impl_t<ws_client, asio::ip::tcp::socket,
		websocket::stream<asio::ip::tcp::socket&>, http::string_body, beast::flat_buffer>
	{
	public:
		using ws_client_impl_t<ws_client, asio::ip::tcp::socket,
			websocket::stream<asio::ip::tcp::socket&>,
			http::string_body, beast::flat_buffer>::ws_client_impl_t;
	};
}

#endif // !__ASIO2_WS_CLIENT_HPP__
