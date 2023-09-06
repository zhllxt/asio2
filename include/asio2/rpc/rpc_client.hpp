/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_RPC_CLIENT_HPP__
#define __ASIO2_RPC_CLIENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if __has_include(<cereal/cereal.hpp>)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/config.hpp>

#include <asio2/udp/udp_client.hpp>
#include <asio2/tcp/tcp_client.hpp>
#include <asio2/tcp/tcps_client.hpp>
#include <asio2/http/ws_client.hpp>
#include <asio2/http/wss_client.hpp>

#include <asio2/rpc/detail/rpc_protocol.hpp>
#include <asio2/rpc/detail/rpc_invoker.hpp>
#include <asio2/rpc/impl/rpc_recv_op.hpp>
#include <asio2/rpc/impl/rpc_call_cp.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;

	ASIO2_CLASS_FORWARD_DECLARE_UDP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_UDP_CLIENT;

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

		ASIO2_CLASS_FRIEND_DECLARE_UDP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_UDP_CLIENT;

		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;

	public:
		using super = executor_t;
		using self  = rpc_client_impl_t<derived_t, executor_t>;

		using executor_type = executor_t;

		using args_type = typename executor_t::args_type;

		static constexpr asio2::net_protocol net_protocol = args_type::net_protocol;

	protected:
		using super::send;
		using super::async_send;

	public:
		/**
		 * @brief constructor
		 */
		template<class ...Args>
		explicit rpc_client_impl_t(
			Args&&... args
		)
			: super(std::forward<Args>(args)...)
			, rpc_invoker_t<derived_t, typename executor_t::args_type>()
			, rpc_call_cp  <derived_t, typename executor_t::args_type>(this->serializer_, this->deserializer_)
			, rpc_recv_op  <derived_t, typename executor_t::args_type>()
			, id_maker     <typename rpc_header::id_type>()
		{
		}

		/**
		 * @brief destructor
		 */
		~rpc_client_impl_t()
		{
			this->stop();
		}

		/**
		 * @brief start the client, blocking connect to server
		 * @param host - A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port - A string identifying the requested service. This may be a
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
			else if constexpr (net_protocol == asio2::net_protocol::udp)
			{
				return executor_t::template start(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					asio2::use_kcp, std::forward<Args>(args)...);
			}
			else
			{
				return executor_t::template start(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					asio2::use_dgram, std::forward<Args>(args)...);
			}
		}

		/**
		 * @brief start the client, asynchronous connect to server
		 * @param host - A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port - A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt, typename... Args>
		bool async_start(String&& host, StrOrInt&& port, Args&&... args)
		{
			//static_assert(net_protocol != asio2::net_protocol::udp, "net_protocol::udp not supported");
			if constexpr (is_websocket_client<executor_t>::value)
			{
				return executor_t::template async_start(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					std::forward<Args>(args)...);
			}
			else if constexpr (net_protocol == asio2::net_protocol::udp)
			{
				return executor_t::template start(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					asio2::use_kcp, std::forward<Args>(args)...);
			}
			else
			{
				return executor_t::template async_start(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					asio2::use_dgram, std::forward<Args>(args)...);
			}
		}

	protected:
		template<typename C, typename Socket>
		inline void _ws_start(std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs, Socket& socket)
		{
			super::_ws_start(this_ptr, ecs, socket);

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

		template<typename C>
		inline void _fire_recv(
			std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs, std::string_view data)
		{
			data = detail::call_data_filter_before_recv(this->derived(), data);

			this->listener_.notify(event_type::recv, data);

			this->derived()._rpc_handle_recv(this_ptr, ecs, data);
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
		struct template_args_rpc_client<asio2::net_protocol::udp> : public template_args_udp_client
		{
			static constexpr asio2::net_protocol net_protocol = asio2::net_protocol::udp;

			static constexpr bool rdc_call_cp_enabled = false;

			static constexpr std::size_t function_storage_size = 72;
		};

		template<>
		struct template_args_rpc_client<asio2::net_protocol::tcp> : public template_args_tcp_client
		{
			static constexpr asio2::net_protocol net_protocol = asio2::net_protocol::tcp;

			static constexpr bool rdc_call_cp_enabled = false;

			static constexpr std::size_t function_storage_size = 72;
		};

		template<>
		struct template_args_rpc_client<asio2::net_protocol::ws> : public template_args_ws_client
		{
			static constexpr asio2::net_protocol net_protocol = asio2::net_protocol::ws;

			static constexpr bool rdc_call_cp_enabled = false;

			static constexpr std::size_t function_storage_size = 72;
		};
	}

	using rpc_client_args_udp = detail::template_args_rpc_client<asio2::net_protocol::udp>;
	using rpc_client_args_tcp = detail::template_args_rpc_client<asio2::net_protocol::tcp>;
	using rpc_client_args_ws  = detail::template_args_rpc_client<asio2::net_protocol::ws >;

	template<class derived_t, class executor_t>
	using rpc_client_impl_t = detail::rpc_client_impl_t<derived_t, executor_t>;


	template<class derived_t, asio2::net_protocol np> class rpc_client_t;

	template<class derived_t>
	class rpc_client_t<derived_t, asio2::net_protocol::udp> : public detail::rpc_client_impl_t<derived_t,
		detail::udp_client_impl_t<derived_t, detail::template_args_rpc_client<asio2::net_protocol::udp>>>
	{
	public:
		using detail::rpc_client_impl_t<derived_t, detail::udp_client_impl_t
			<derived_t, detail::template_args_rpc_client<asio2::net_protocol::udp>>>::rpc_client_impl_t;
	};

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

	template<asio2::net_protocol np>
	class rpc_client_use : public rpc_client_t<rpc_client_use<np>, np>
	{
	public:
		using rpc_client_t<rpc_client_use<np>, np>::rpc_client_t;
	};
	
	using rpc_tcp_client = rpc_client_use<asio2::net_protocol::tcp>;
	using rpc_ws_client  = rpc_client_use<asio2::net_protocol::ws>;
	using rpc_kcp_client = rpc_client_use<asio2::net_protocol::udp>;

#if !defined(ASIO2_USE_WEBSOCKET_RPC)
	/// Using tcp dgram mode as the underlying communication support
	using rpc_client = rpc_client_use<asio2::net_protocol::tcp>;
#else
	/// Using websocket as the underlying communication support
	using rpc_client = rpc_client_use<asio2::net_protocol::ws>;
#endif
}

#if defined(ASIO2_INCLUDE_RATE_LIMIT)
#include <asio2/tcp/tcp_stream.hpp>
namespace asio2
{
	struct rpc_rate_client_args_tcp : public rpc_client_args_tcp
	{
		using socket_t = asio2::tcp_stream<asio2::simple_rate_policy>;
	};
	struct rpc_rate_client_args_ws : public rpc_client_args_ws
	{
		using socket_t = asio2::tcp_stream<asio2::simple_rate_policy>;
		using stream_t = websocket::stream<socket_t&>;
	};

	template<class derived_t, asio2::net_protocol np> class rpc_rate_client_t;

	template<class derived_t>
	class rpc_rate_client_t<derived_t, asio2::net_protocol::tcp> : public detail::rpc_client_impl_t<derived_t,
		detail::tcp_client_impl_t<derived_t, rpc_rate_client_args_tcp>>
	{
	public:
		using detail::rpc_client_impl_t<derived_t,
			detail::tcp_client_impl_t<derived_t, rpc_rate_client_args_tcp>>::rpc_client_impl_t;
	};

	template<class derived_t>
	class rpc_rate_client_t<derived_t, asio2::net_protocol::ws> : public detail::rpc_client_impl_t<derived_t,
		detail::ws_client_impl_t<derived_t, rpc_rate_client_args_ws>>
	{
	public:
		using detail::rpc_client_impl_t<derived_t,
			detail::ws_client_impl_t<derived_t, rpc_rate_client_args_ws>>::rpc_client_impl_t;
	};

	template<asio2::net_protocol np>
	class rpc_rate_client_use : public rpc_rate_client_t<rpc_rate_client_use<np>, np>
	{
	public:
		using rpc_rate_client_t<rpc_rate_client_use<np>, np>::rpc_rate_client_t;
	};

#if !defined(ASIO2_USE_WEBSOCKET_RPC)
	/// Using tcp dgram mode as the underlying communication support
	using rpc_rate_client = rpc_rate_client_use<asio2::net_protocol::tcp>;
#else
	/// Using websocket as the underlying communication support
	using rpc_rate_client = rpc_rate_client_use<asio2::net_protocol::ws>;
#endif
}
#endif

#include <asio2/base/detail/pop_options.hpp>

#endif

#endif // !__ASIO2_RPC_CLIENT_HPP__
