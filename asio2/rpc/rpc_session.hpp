/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_RPC_SESSION_HPP__
#define __ASIO2_RPC_SESSION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/tcp/tcp_session.hpp>
#include <asio2/tcp/tcps_session.hpp>
#include <asio2/http/ws_session.hpp>
#include <asio2/http/wss_session.hpp>

#include <asio2/rpc/detail/serialization.hpp>
#include <asio2/rpc/detail/protocol.hpp>
#include <asio2/rpc/detail/invoker.hpp>
#include <asio2/rpc/component/rpc_call_cp.hpp>
#include <asio2/rpc/impl/rpc_recv_op.hpp>

namespace asio2::detail
{
	template<class derived_t, class executor_t>
	class rpc_session_impl_t
		: public executor_t
		, public rpc_call_cp<derived_t, true>
		, public rpc_recv_op<derived_t, true>
		, protected id_maker<typename header::id_type>
	{
		friend executor_t;

		template <class, bool>         friend class user_timer_cp;
		template <class, bool>         friend class send_queue_cp;
		template <class, bool>         friend class send_cp;
		template <class, bool>         friend class silence_timer_cp;
		template <class, bool>         friend class connect_timeout_cp;
		template <class, bool>         friend class tcp_send_op;
		template <class, bool>         friend class tcp_recv_op;
		template <class, class, bool>  friend class ws_stream_cp;
		template <class, bool>         friend class ws_send_op;
		template <class, bool>         friend class rpc_call_cp;
		template <class, bool>         friend class rpc_recv_op;
		template <class>               friend class session_mgr_t;

		template <class, class, class>               friend class session_impl_t;
		template <class, class, class>               friend class tcp_session_impl_t;
		template <class, class, class>               friend class tcps_session_impl_t;
		template <class, class, class, class, class> friend class ws_session_impl_t;
		template <class, class, class, class, class> friend class wss_session_impl_t;

		template <class, class> friend class server_impl_t;
		template <class, class> friend class tcp_server_impl_t;
		template <class, class> friend class tcps_server_impl_t;
		template <class, class> friend class ws_server_impl_t;
		template <class, class> friend class wss_server_impl_t;
		template <class, class> friend class rpc_server_impl_t;
		template <class, class> friend class rpcs_server_impl_t;

	public:
		using self = rpc_session_impl_t<derived_t, executor_t>;
		using super = executor_t;
		using executor_type = executor_t;

	protected:
		using super::send;
		using super::_handle_recv;

	public:
		/**
		 * @constructor
		 */
		template<class ...Args>
		explicit rpc_session_impl_t(
			Args&&... args
		)
			: super(std::forward<Args>(args)...)
			, rpc_call_cp<derived_t, true>(this->io_, this->serializer_, this->deserializer_)
			, rpc_recv_op<derived_t, true>()
			, id_maker<typename header::id_type>()
		{
		}

		/**
		 * @destructor
		 */
		~rpc_session_impl_t()
		{
		}

		/**
		 * @function : set call rpc function timeout duration value
		 */
		template<class Rep, class Period>
		inline derived_t & timeout(std::chrono::duration<Rep, Period> duration)
		{
			this->timeout_ = duration;
			return (this->derived());
		}

		/**
		 * @function : get call rpc function timeout duration value
		 */
		inline std::chrono::steady_clock::duration timeout()
		{
			return this->timeout_;
		}

	protected:
		inline derived_t& _invoker(invoker<derived_t>* invoke)
		{
			this->invoker_ = invoke;
			return (this->derived());
		}

		inline invoker<derived_t>& _invoker()
		{
			return (*(this->invoker_));
		}

		inline void _handle_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			while (!this->reqs_.empty())
			{
				auto& fn = this->reqs_.begin()->second;
				fn(asio::error::operation_aborted, std::string_view{});
			}

			super::_handle_stop(ec, std::move(this_ptr));
		}

		inline void _handle_recv(std::shared_ptr<derived_t>& this_ptr, std::string_view s)
		{
			this->derived()._rpc_handle_recv(this_ptr, s);
		}

		inline void _fire_send(std::shared_ptr<derived_t>& this_ptr, std::string_view s)
		{
			this->listener_.notify(event::send, this_ptr, s);
		}

	protected:
		serializer serializer_;
		deserializer deserializer_;
		header header_;
		std::chrono::steady_clock::duration timeout_ = std::chrono::milliseconds(http_execute_timeout);
		invoker<derived_t>* invoker_ = nullptr;
	};
}

namespace asio2
{
#if 1
	/// Using tcp dgram mode as the underlying communication support
	class rpc_session : public detail::rpc_session_impl_t<rpc_session,
		detail::tcp_session_impl_t<rpc_session, asio::ip::tcp::socket, asio::streambuf>>
	{
	public:
		using detail::rpc_session_impl_t<rpc_session,
			detail::tcp_session_impl_t<rpc_session, asio::ip::tcp::socket, asio::streambuf>>::rpc_session_impl_t;
	};

	#if defined(ASIO2_USE_SSL)
	class rpcs_session : public detail::rpc_session_impl_t<rpcs_session,
		detail::tcps_session_impl_t<rpcs_session, asio::ip::tcp::socket, asio::streambuf>>
	{
	public:
		using detail::rpc_session_impl_t<rpcs_session,
			detail::tcps_session_impl_t<rpcs_session, asio::ip::tcp::socket, asio::streambuf>>::rpc_session_impl_t;
	};
	#endif
#else
	/// Using websocket as the underlying communication support
	#ifndef ASIO_STANDALONE
	class rpc_session : public detail::rpc_session_impl_t<rpc_session, detail::ws_session_impl_t<rpc_session, asio::ip::tcp::socket,
		websocket::stream<asio::ip::tcp::socket&>, http::string_body, beast::flat_buffer>>
	{
	public:
		using detail::rpc_session_impl_t<rpc_session, detail::ws_session_impl_t<rpc_session, asio::ip::tcp::socket,
			websocket::stream<asio::ip::tcp::socket&>, http::string_body, beast::flat_buffer>>::rpc_session_impl_t;
	};

	#if defined(ASIO2_USE_SSL)
	class rpcs_session : public detail::rpc_session_impl_t<rpcs_session, detail::wss_session_impl_t<rpcs_session, asio::ip::tcp::socket,
		websocket::stream<asio::ssl::stream<asio::ip::tcp::socket&>&>, http::string_body, beast::flat_buffer>>
	{
	public:
		using detail::rpc_session_impl_t<rpcs_session, detail::wss_session_impl_t<rpcs_session, asio::ip::tcp::socket,
			websocket::stream<asio::ssl::stream<asio::ip::tcp::socket&>&>, http::string_body, beast::flat_buffer>>::rpc_session_impl_t;
	};
	#endif
	#endif
#endif
}

#endif // !__ASIO2_RPC_SESSION_HPP__
