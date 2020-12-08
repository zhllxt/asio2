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

#include <asio2/rpc/detail/rpc_protocol.hpp>
#include <asio2/rpc/detail/rpc_invoker.hpp>
#include <asio2/rpc/impl/rpc_recv_op.hpp>
#include <asio2/rpc/component/rpc_call_cp.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;

	template<class derived_t, class executor_t>
	class rpc_client_impl_t
		: public executor_t
		, public rpc_invoker_t<derived_t>
		, public rpc_call_cp  <derived_t>
		, public rpc_recv_op  <derived_t>
		, protected id_maker  <typename rpc_header::id_type>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;

	public:
		using super = executor_t;
		using self  = rpc_client_impl_t<derived_t, executor_t>;

		using executor_type = executor_t;

	protected:
		using super::send;
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
			, rpc_invoker_t<derived_t>()
			, rpc_call_cp  <derived_t>(this->io_, this->serializer_, this->deserializer_)
			, rpc_recv_op  <derived_t>()
			, id_maker     <typename rpc_header::id_type>()
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

		template<typename MatchCondition>
		inline void _fire_recv(std::shared_ptr<derived_t>& this_ptr, std::string_view s,
			condition_wrap<MatchCondition>& condition)
		{
			this->listener_.notify(event_type::recv, s);

			this->derived()._rpc_handle_recv(this_ptr, s, condition);
		}

	protected:
		detail::rpc_serializer                  serializer_;
		detail::rpc_deserializer                deserializer_;
		detail::rpc_header                      header_;
	};
}

namespace asio2
{
	namespace detail
	{
		struct template_args_rpc_tcp_client : public template_args_tcp_client
		{
			static constexpr bool rdc_call_cp_enabled = false;
		};

		struct template_args_rpc_ws_client : public template_args_ws_client
		{
			static constexpr bool rdc_call_cp_enabled = false;
		};
	}

	template<class>
	class rpc_client_t;

	/// Using tcp dgram mode as the underlying communication support
	template<>
	class rpc_client_t<detail::use_tcp> : public detail::rpc_client_impl_t<rpc_client_t<detail::use_tcp>,
		detail::tcp_client_impl_t<rpc_client_t<detail::use_tcp>, detail::template_args_rpc_tcp_client>>
	{
	public:
		using detail::rpc_client_impl_t<rpc_client_t<detail::use_tcp>, detail::tcp_client_impl_t<
			rpc_client_t<detail::use_tcp>, detail::template_args_rpc_tcp_client>>::rpc_client_impl_t;
	};

	/// Using websocket as the underlying communication support
	template<>
	class rpc_client_t<detail::use_websocket> : public detail::rpc_client_impl_t<
		rpc_client_t<detail::use_websocket>, detail::ws_client_impl_t<
		rpc_client_t<detail::use_websocket>, detail::template_args_rpc_ws_client>>
	{
	public:
		using detail::rpc_client_impl_t<rpc_client_t<detail::use_websocket>,
			detail::ws_client_impl_t<rpc_client_t<detail::use_websocket>,
			detail::template_args_rpc_ws_client>>::rpc_client_impl_t;
	};

#if !defined(ASIO2_USE_WEBSOCKET_RPC)
	using rpc_client = rpc_client_t<detail::use_tcp>;
#else
	using rpc_client = rpc_client_t<detail::use_websocket>;
#endif

#if defined(ASIO2_USE_SSL)
	namespace detail
	{
		struct template_args_rpc_wss_client : public template_args_wss_client
		{
			static constexpr bool rdc_call_cp_enabled = false;
		};
	}

	template<class>
	class rpcs_client_t;

	template<>
	class rpcs_client_t<detail::use_tcp> : public detail::rpc_client_impl_t<
		rpcs_client_t<detail::use_tcp>,
		detail::tcps_client_impl_t<rpcs_client_t<detail::use_tcp>,
		detail::template_args_rpc_tcp_client>>
	{
	public:
		using detail::rpc_client_impl_t<rpcs_client_t<detail::use_tcp>,
			detail::tcps_client_impl_t<rpcs_client_t<detail::use_tcp>,
			detail::template_args_rpc_tcp_client>>::rpc_client_impl_t;
	};

	template<>
	class rpcs_client_t<detail::use_websocket> : public detail::rpc_client_impl_t<
		rpcs_client_t<detail::use_websocket>,
		detail::wss_client_impl_t<rpcs_client_t<detail::use_websocket>,
		detail::template_args_rpc_wss_client>>
	{
	public:
		using detail::rpc_client_impl_t<rpcs_client_t<detail::use_websocket>,
			detail::wss_client_impl_t<rpcs_client_t<
			detail::use_websocket>, detail::template_args_rpc_wss_client>>::rpc_client_impl_t;
	};

#if !defined(ASIO2_USE_WEBSOCKET_RPC)
	using rpcs_client = rpcs_client_t<detail::use_tcp>;
#else
	using rpcs_client = rpcs_client_t<detail::use_websocket>;
#endif
#endif
}

#endif // !__ASIO2_RPC_CLIENT_HPP__
