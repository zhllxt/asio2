/*
 * COPYRIGHT (C) 2017-2021, zhllxt
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

#if __has_include(<cereal/cereal.hpp>)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/config.hpp>

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
		, public rpc_invoker_t<derived_t, typename executor_t::args_type>
		, public rpc_call_cp  <derived_t, typename executor_t::args_type>
		, public rpc_recv_op  <derived_t, typename executor_t::args_type>
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
		using super::async_send;

	public:
		/**
		 * @constructor
		 */
		template<class ...Args>
		explicit rpc_client_impl_t(
			Args&&... args
		)
			: super(std::forward<Args>(args)...)
			, rpc_invoker_t<derived_t, typename executor_t::args_type>()
			, rpc_call_cp  <derived_t, typename executor_t::args_type>(this->io_, this->serializer_, this->deserializer_)
			, rpc_recv_op  <derived_t, typename executor_t::args_type>()
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
		template<typename String, typename StrOrInt, typename... Args>
		bool start(String&& host, StrOrInt&& port, Args&&... args)
		{
			if constexpr (is_websocket_client<executor_t>::value)
			{
				return executor_t::template start(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					std::forward<Args>(args)...);
			}
			else
			{
				return executor_t::template start(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					asio2::use_dgram, std::forward<Args>(args)...);
			}
		}

		/**
		 * @function : start the client, asynchronous connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt, typename... Args>
		bool async_start(String&& host, StrOrInt&& port, Args&&... args)
		{
			if constexpr (is_websocket_client<executor_t>::value)
			{
				return executor_t::template async_start(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					std::forward<Args>(args)...);
			}
			else
			{
				return executor_t::template async_start(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					asio2::use_dgram, std::forward<Args>(args)...);
			}
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

		template<typename DeferEvent>
		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			while (!this->reqs_.empty())
			{
				auto& fn = this->reqs_.begin()->second;
				fn(rpc::make_error_code(rpc::error::operation_aborted), std::string_view{});
			}

			super::_handle_disconnect(ec, std::move(this_ptr), std::move(chain));
		}

		template<typename MatchCondition>
		inline void _fire_recv(std::shared_ptr<derived_t>& this_ptr, std::string_view data,
			condition_wrap<MatchCondition>& condition)
		{
			this->listener_.notify(event_type::recv, data);

			this->derived()._rpc_handle_recv(this_ptr, data, condition);
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
		template<asio2::net_protocol np> struct template_args_rpc_client;

		template<>
		struct template_args_rpc_client<asio2::net_protocol::tcp> : public template_args_tcp_client
		{
			static constexpr bool rdc_call_cp_enabled = false;
		};

		template<>
		struct template_args_rpc_client<asio2::net_protocol::ws> : public template_args_ws_client
		{
			static constexpr bool rdc_call_cp_enabled = false;
		};
	}

	template<class derived_t, asio2::net_protocol np> class rpc_client_t;

	template<class derived_t>
	class rpc_client_t<derived_t, asio2::net_protocol::tcp> : public detail::rpc_client_impl_t<derived_t,
		detail::tcp_client_impl_t<derived_t, detail::template_args_rpc_client<asio2::net_protocol::tcp>>>
	{
	public:
		using detail::rpc_client_impl_t<derived_t, detail::tcp_client_impl_t
			<derived_t, detail::template_args_rpc_client<asio2::net_protocol::tcp>>>::rpc_client_impl_t;
	};

	template<class derived_t>
	class rpc_client_t<derived_t, asio2::net_protocol::ws> : public detail::rpc_client_impl_t<derived_t,
		detail::ws_client_impl_t<derived_t, detail::template_args_rpc_client<asio2::net_protocol::ws>>>
	{
	public:
		using detail::rpc_client_impl_t<derived_t, detail::ws_client_impl_t<derived_t,
			detail::template_args_rpc_client<asio2::net_protocol::ws>>>::rpc_client_impl_t;
	};

	template<asio2::net_protocol np> class rpc_client_use;

	template<>
	class rpc_client_use<asio2::net_protocol::tcp>
		: public rpc_client_t<rpc_client_use<asio2::net_protocol::tcp>, asio2::net_protocol::tcp>
	{
	public:
		using rpc_client_t<rpc_client_use<asio2::net_protocol::tcp>, asio2::net_protocol::tcp>::rpc_client_t;
	};

	template<>
	class rpc_client_use<asio2::net_protocol::ws>
		: public rpc_client_t<rpc_client_use<asio2::net_protocol::ws>, asio2::net_protocol::ws>
	{
	public:
		using rpc_client_t<rpc_client_use<asio2::net_protocol::ws>, asio2::net_protocol::ws>::rpc_client_t;
	};

#if defined(ASIO2_USE_SSL)
	namespace detail
	{
		template<asio2::net_protocol np> struct template_args_rpcs_client;

		template<>
		struct template_args_rpcs_client<asio2::net_protocol::tcps> : public template_args_tcp_client
		{
			static constexpr bool rdc_call_cp_enabled = false;
		};

		template<>
		struct template_args_rpcs_client<asio2::net_protocol::wss> : public template_args_wss_client
		{
			static constexpr bool rdc_call_cp_enabled = false;
		};
	}

	template<class derived_t, asio2::net_protocol np> class rpcs_client_t;

	template<class derived_t>
	class rpcs_client_t<derived_t, asio2::net_protocol::tcps> : public detail::rpc_client_impl_t<derived_t,
		detail::tcps_client_impl_t<derived_t, detail::template_args_rpcs_client<asio2::net_protocol::tcps>>>
	{
	public:
		using detail::rpc_client_impl_t<derived_t, detail::tcps_client_impl_t<derived_t,
			detail::template_args_rpcs_client<asio2::net_protocol::tcps>>>::rpc_client_impl_t;
	};

	template<class derived_t>
	class rpcs_client_t<derived_t, asio2::net_protocol::wss> : public detail::rpc_client_impl_t<derived_t,
		detail::wss_client_impl_t<derived_t, detail::template_args_rpcs_client<asio2::net_protocol::wss>>>
	{
	public:
		using detail::rpc_client_impl_t<derived_t, detail::wss_client_impl_t<
			derived_t, detail::template_args_rpcs_client<asio2::net_protocol::wss>>>::rpc_client_impl_t;
	};

	template<asio2::net_protocol np> class rpcs_client_use;

	template<>
	class rpcs_client_use<asio2::net_protocol::tcps>
		: public rpcs_client_t<rpcs_client_use<asio2::net_protocol::tcps>, asio2::net_protocol::tcps>
	{
	public:
		using rpcs_client_t<rpcs_client_use<asio2::net_protocol::tcps>, asio2::net_protocol::tcps>::rpcs_client_t;
	};

	template<>
	class rpcs_client_use<asio2::net_protocol::wss>
		: public rpcs_client_t<rpcs_client_use<asio2::net_protocol::wss>, asio2::net_protocol::wss>
	{
	public:
		using rpcs_client_t<rpcs_client_use<asio2::net_protocol::wss>, asio2::net_protocol::wss>::rpcs_client_t;
	};
#endif

#if !defined(ASIO2_USE_WEBSOCKET_RPC)
	/// Using tcp dgram mode as the underlying communication support
	using rpc_client = rpc_client_use<asio2::net_protocol::tcp>;
#else
	/// Using websocket as the underlying communication support
	using rpc_client = rpc_client_use<asio2::net_protocol::ws>;
#endif

#if defined(ASIO2_USE_SSL)
#if !defined(ASIO2_USE_WEBSOCKET_RPC)
	/// Using tcp dgram mode as the underlying communication support
	using rpcs_client = rpcs_client_use<asio2::net_protocol::tcps>;
#else
	/// Using websocket as the underlying communication support
	using rpcs_client = rpcs_client_use<asio2::net_protocol::wss>;
#endif
#endif
}

#include <asio2/base/detail/pop_options.hpp>

#endif

#endif // !__ASIO2_RPC_CLIENT_HPP__
