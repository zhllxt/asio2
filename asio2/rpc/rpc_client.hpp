/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_RPC_CLIENT_HPP__
#define __ASIO2_RPC_CLIENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/tcp/tcp_client.hpp>
#include <asio2/tcp/tcps_client.hpp>
#include <asio2/http/ws_client.hpp>
#include <asio2/http/wss_client.hpp>

#include <asio2/rpc/detail/protocol.hpp>
#include <asio2/rpc/detail/invoker.hpp>
#include <asio2/rpc/impl/rpc_recv_op.hpp>
#include <asio2/rpc/component/rpc_call_cp.hpp>

namespace asio2::detail
{
	template<class derived_t, class executor_t>
	class rpc_client_impl_t
		: public executor_t
		, public invoker_t<derived_t>
		, public rpc_call_cp<derived_t, false>
		, public rpc_recv_op<derived_t, false>
		, protected id_maker<typename header::id_type>
	{
		template <class>                             friend class invoker_t;
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
		template <class, class, bool>                friend class ssl_stream_cp;
		template <class, class, bool>                friend class ws_stream_cp;
		template <class, bool>                       friend class ws_send_op;
		template <class, bool>                       friend class rpc_call_cp;
		template <class, bool>                       friend class rpc_recv_op;
		template <class, class, class>               friend class client_impl_t;
		template <class, class, class>               friend class tcp_client_impl_t;
		template <class, class, class>               friend class tcps_client_impl_t;
		template <class, class, class, class, class> friend class ws_client_impl_t;
		template <class, class, class, class, class> friend class wss_client_impl_t;

	public:
		using self = rpc_client_impl_t<derived_t, executor_t>;
		using super = executor_t;
		using executor_type = executor_t;

	protected:
		//using super::send;
		using super::_handle_recv;

	public:
		/**
		 * @constructor
		 */
		template<class ...Args>
		explicit rpc_client_impl_t(
			Args&&... args
		)
			: super(std::forward<Args>(args)...)
			, invoker_t<derived_t>()
			, rpc_call_cp<derived_t, false>(this->io_, this->serializer_, this->deserializer_)
			, rpc_recv_op<derived_t, false>()
			, id_maker<typename header::id_type>()
		{
		}

		/**
		 * @destructor
		 */
		~rpc_client_impl_t()
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
			if constexpr (is_websocket_client<executor_t>::value)
				return executor_t::template start(
					std::forward<String>(host), std::forward<StrOrInt>(port));
			else
				return executor_t::template start(
					std::forward<String>(host), std::forward<StrOrInt>(port), asio2::use_dgram);
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
			if constexpr (is_websocket_client<executor_t>::value)
				return executor_t::template async_start(
					std::forward<String>(host), std::forward<StrOrInt>(port));
			else
				return executor_t::template async_start(
					std::forward<String>(host), std::forward<StrOrInt>(port), asio2::use_dgram);
		}

		/**
		 * @function : set the default timeout for calling rpc functions
		 */
		template<class Rep, class Period>
		inline derived_t & default_timeout(std::chrono::duration<Rep, Period> duration)
		{
			this->timeout_ = duration;
			return (this->derived());
		}

		/**
		 * @function : get the default timeout for calling rpc functions
		 */
		inline std::chrono::steady_clock::duration default_timeout()
		{
			return this->timeout_;
		}

	protected:
		template<typename MatchCondition, typename Socket>
		inline void _ws_start(
			const std::shared_ptr<derived_t>& this_ptr,
			const condition_wrap<MatchCondition>& condition, Socket& socket)
		{
			super::_ws_start(this_ptr, condition, socket);

			this->derived().ws_stream().binary(true);
		}

		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			while (!this->reqs_.empty())
			{
				auto& fn = this->reqs_.begin()->second;
				fn(asio::error::operation_aborted, std::string_view{});
			}

			super::_handle_disconnect(ec, std::move(this_ptr));
		}

		inline void _fire_recv(std::shared_ptr<derived_t> this_ptr, std::string_view s)
		{
			this->listener_.notify(event::recv, s);

			this->derived()._rpc_handle_recv(this_ptr, s);
		}

	protected:
		detail::serializer                  serializer_;
		detail::deserializer                deserializer_;
		detail::header                      header_;
		std::chrono::steady_clock::duration timeout_ = std::chrono::milliseconds(http_execute_timeout);
	};
}

namespace asio2
{
	template<class>
	class rpc_client_t;

	/// Using tcp dgram mode as the underlying communication support
	template<>
	class rpc_client_t<detail::use_tcp> : public detail::rpc_client_impl_t<rpc_client_t<detail::use_tcp>,
		detail::tcp_client_impl_t<rpc_client_t<detail::use_tcp>, asio::ip::tcp::socket, asio::streambuf>>
	{
	public:
		using detail::rpc_client_impl_t<rpc_client_t<detail::use_tcp>, detail::tcp_client_impl_t<
			rpc_client_t<detail::use_tcp>, asio::ip::tcp::socket, asio::streambuf>>::rpc_client_impl_t;
	};

	/// Using websocket as the underlying communication support
	template<>
	class rpc_client_t<detail::use_websocket> : public detail::rpc_client_impl_t<
		rpc_client_t<detail::use_websocket>,
		detail::ws_client_impl_t<rpc_client_t<detail::use_websocket>, asio::ip::tcp::socket,
		websocket::stream<asio::ip::tcp::socket&>, http::string_body, beast::flat_buffer>>
	{
	public:
		using detail::rpc_client_impl_t<rpc_client_t<detail::use_websocket>,
			detail::ws_client_impl_t<rpc_client_t<detail::use_websocket>,
			asio::ip::tcp::socket, websocket::stream<asio::ip::tcp::socket&>,
			http::string_body, beast::flat_buffer>>::rpc_client_impl_t;
	};

#if !defined(ASIO2_USE_WEBSOCKET_RPC)
	using rpc_client = rpc_client_t<detail::use_tcp>;
#else
	using rpc_client = rpc_client_t<detail::use_websocket>;
#endif

#if defined(ASIO2_USE_SSL)
	template<class>
	class rpcs_client_t;

	template<>
	class rpcs_client_t<detail::use_tcp> : public detail::rpc_client_impl_t<
		rpcs_client_t<detail::use_tcp>,
		detail::tcps_client_impl_t<rpcs_client_t<detail::use_tcp>,
		asio::ip::tcp::socket, asio::streambuf>>
	{
	public:
		using detail::rpc_client_impl_t<rpcs_client_t<detail::use_tcp>,
			detail::tcps_client_impl_t<rpcs_client_t<detail::use_tcp>,
			asio::ip::tcp::socket, asio::streambuf>>::rpc_client_impl_t;
	};

	template<>
	class rpcs_client_t<detail::use_websocket> : public detail::rpc_client_impl_t<
		rpcs_client_t<detail::use_websocket>,
		detail::wss_client_impl_t<rpcs_client_t<detail::use_websocket>,
		asio::ip::tcp::socket,
		websocket::stream<asio::ssl::stream<asio::ip::tcp::socket&>&>,
		http::string_body, beast::flat_buffer>>
	{
	public:
		using detail::rpc_client_impl_t<rpcs_client_t<detail::use_websocket>,
			detail::wss_client_impl_t<rpcs_client_t<
			detail::use_websocket>, asio::ip::tcp::socket,
			websocket::stream<asio::ssl::stream<asio::ip::tcp::socket&>&>,
			http::string_body, beast::flat_buffer>>::rpc_client_impl_t;
	};

#if !defined(ASIO2_USE_WEBSOCKET_RPC)
	using rpcs_client = rpcs_client_t<detail::use_tcp>;
#else
	using rpcs_client = rpcs_client_t<detail::use_websocket>;
#endif
#endif
}

#endif // !__ASIO2_RPC_CLIENT_HPP__
