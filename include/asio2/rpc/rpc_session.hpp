/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_RPC_SESSION_HPP__
#define __ASIO2_RPC_SESSION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if __has_include(<cereal/cereal.hpp>)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/config.hpp>

#include <asio2/udp/udp_session.hpp>
#include <asio2/tcp/tcp_session.hpp>
#include <asio2/tcp/tcps_session.hpp>
#include <asio2/http/ws_session.hpp>
#include <asio2/http/wss_session.hpp>

#include <asio2/rpc/detail/rpc_serialization.hpp>
#include <asio2/rpc/detail/rpc_protocol.hpp>
#include <asio2/rpc/detail/rpc_invoker.hpp>
#include <asio2/rpc/impl/rpc_recv_op.hpp>
#include <asio2/rpc/impl/rpc_call_cp.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	
	ASIO2_CLASS_FORWARD_DECLARE_UDP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_UDP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_UDP_SESSION;
	
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;

	template<class derived_t, class executor_t>
	class rpc_session_impl_t
		: public executor_t
		, public rpc_call_cp<derived_t, typename executor_t::args_type>
		, public rpc_recv_op<derived_t, typename executor_t::args_type>
		, protected id_maker<typename rpc_header::id_type>
	{
		friend executor_t;

		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		
		ASIO2_CLASS_FRIEND_DECLARE_UDP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_UDP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_UDP_SESSION;

		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;

	public:
		using super = executor_t;
		using self  = rpc_session_impl_t<derived_t, executor_t>;

		using executor_type = executor_t;

		using args_type     = typename executor_t::args_type;

		static constexpr asio2::net_protocol net_protocol = args_type::net_protocol;

	protected:
		using super::send;
		using super::async_send;

	public:
		/**
		 * @brief constructor
		 */
		template<class ...Args>
		explicit rpc_session_impl_t(
			rpc_invoker_t<derived_t, typename executor_t::args_type>& invoker,
			Args&&... args
		)
			: super(std::forward<Args>(args)...)
			, rpc_call_cp<derived_t, typename executor_t::args_type>(this->serializer_, this->deserializer_)
			, rpc_recv_op<derived_t, typename executor_t::args_type>()
			, id_maker<typename rpc_header::id_type>()
			, invoker_(invoker)
		{
		}

		/**
		 * @brief destructor
		 */
		~rpc_session_impl_t()
		{
		}

	protected:
		inline rpc_invoker_t<derived_t, typename executor_t::args_type>& _invoker() noexcept
		{
			return (this->invoker_);
		}
		inline rpc_invoker_t<derived_t, typename executor_t::args_type> const& _invoker() const noexcept
		{
			return (this->invoker_);
		}

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

			this->listener_.notify(event_type::recv, this_ptr, data);

			this->derived()._rpc_handle_recv(this_ptr, ecs, data);
		}

	protected:
		rpc_serializer                                             serializer_;
		rpc_deserializer                                           deserializer_;
		rpc_header                                                 header_;
		rpc_invoker_t<derived_t, typename executor_t::args_type> & invoker_;
	};
}

namespace asio2
{
	namespace detail
	{
		template<asio2::net_protocol np> struct template_args_rpc_session;

		template<>
		struct template_args_rpc_session<asio2::net_protocol::udp> : public template_args_udp_session
		{
			static constexpr asio2::net_protocol net_protocol = asio2::net_protocol::udp;

			static constexpr bool rdc_call_cp_enabled = false;

			static constexpr std::size_t function_storage_size = 72;
		};

		template<>
		struct template_args_rpc_session<asio2::net_protocol::tcp> : public template_args_tcp_session
		{
			static constexpr asio2::net_protocol net_protocol = asio2::net_protocol::tcp;

			static constexpr bool rdc_call_cp_enabled = false;

			static constexpr std::size_t function_storage_size = 72;
		};

		template<>
		struct template_args_rpc_session<asio2::net_protocol::ws> : public template_args_ws_session
		{
			static constexpr asio2::net_protocol net_protocol = asio2::net_protocol::ws;

			static constexpr bool rdc_call_cp_enabled = false;

			static constexpr std::size_t function_storage_size = 72;
		};
	}

	using rpc_session_args_udp = detail::template_args_rpc_session<asio2::net_protocol::udp>;
	using rpc_session_args_tcp = detail::template_args_rpc_session<asio2::net_protocol::tcp>;
	using rpc_session_args_ws  = detail::template_args_rpc_session<asio2::net_protocol::ws >;

	template<class derived_t, class executor_t>
	using rpc_session_impl_t = detail::rpc_session_impl_t<derived_t, executor_t>;


	template<class derived_t, asio2::net_protocol np> class rpc_session_t;

	template<class derived_t>
	class rpc_session_t<derived_t, asio2::net_protocol::udp> : public detail::rpc_session_impl_t<derived_t,
		detail::udp_session_impl_t<derived_t, detail::template_args_rpc_session<asio2::net_protocol::udp>>>
	{
	public:
		using detail::rpc_session_impl_t<derived_t, detail::udp_session_impl_t<
			derived_t, detail::template_args_rpc_session<asio2::net_protocol::udp>>>::rpc_session_impl_t;
	};

	template<class derived_t>
	class rpc_session_t<derived_t, asio2::net_protocol::tcp> : public detail::rpc_session_impl_t<derived_t,
		detail::tcp_session_impl_t<derived_t, detail::template_args_rpc_session<asio2::net_protocol::tcp>>>
	{
	public:
		using detail::rpc_session_impl_t<derived_t, detail::tcp_session_impl_t<
			derived_t, detail::template_args_rpc_session<asio2::net_protocol::tcp>>>::rpc_session_impl_t;
	};

	template<class derived_t>
	class rpc_session_t<derived_t, asio2::net_protocol::ws> : public detail::rpc_session_impl_t<derived_t,
		detail::ws_session_impl_t<derived_t, detail::template_args_rpc_session<asio2::net_protocol::ws>>>
	{
	public:
		using detail::rpc_session_impl_t<derived_t, detail::ws_session_impl_t<
			derived_t, detail::template_args_rpc_session<asio2::net_protocol::ws>>>::rpc_session_impl_t;
	};

	template<asio2::net_protocol np>
	class rpc_session_use : public rpc_session_t<rpc_session_use<np>, np>
	{
	public:
		using rpc_session_t<rpc_session_use<np>, np>::rpc_session_t;
	};

	using rpc_tcp_session = rpc_session_use<asio2::net_protocol::tcp>;
	using rpc_ws_session  = rpc_session_use<asio2::net_protocol::ws>;
	using rpc_kcp_session = rpc_session_use<asio2::net_protocol::udp>;

#if !defined(ASIO2_USE_WEBSOCKET_RPC)
	/// Using tcp dgram mode as the underlying communication support
	using rpc_session = rpc_session_use<asio2::net_protocol::tcp>;
#else
	/// Using websocket as the underlying communication support
	using rpc_session = rpc_session_use<asio2::net_protocol::ws>;
#endif
}

#if defined(ASIO2_INCLUDE_RATE_LIMIT)
#include <asio2/tcp/tcp_stream.hpp>
namespace asio2
{
	struct rpc_rate_session_args_tcp : public rpc_session_args_tcp
	{
		using socket_t = asio2::tcp_stream<asio2::simple_rate_policy>;
	};
	struct rpc_rate_session_args_ws : public rpc_session_args_ws
	{
		using socket_t = asio2::tcp_stream<asio2::simple_rate_policy>;
		using stream_t = websocket::stream<socket_t&>;
	};

	template<class derived_t, asio2::net_protocol np> class rpc_rate_session_t;

	template<class derived_t>
	class rpc_rate_session_t<derived_t, asio2::net_protocol::tcp> : public detail::rpc_session_impl_t<derived_t,
		detail::tcp_session_impl_t<derived_t, rpc_rate_session_args_tcp>>
	{
	public:
		using detail::rpc_session_impl_t<derived_t,
			detail::tcp_session_impl_t<derived_t, rpc_rate_session_args_tcp>>::rpc_session_impl_t;
	};

	template<class derived_t>
	class rpc_rate_session_t<derived_t, asio2::net_protocol::ws> : public detail::rpc_session_impl_t<derived_t,
		detail::ws_session_impl_t<derived_t, rpc_rate_session_args_ws>>
	{
	public:
		using detail::rpc_session_impl_t<derived_t,
			detail::ws_session_impl_t<derived_t, rpc_rate_session_args_ws>>::rpc_session_impl_t;
	};

	template<asio2::net_protocol np>
	class rpc_rate_session_use : public rpc_rate_session_t<rpc_rate_session_use<np>, np>
	{
	public:
		using rpc_rate_session_t<rpc_rate_session_use<np>, np>::rpc_rate_session_t;
	};

#if !defined(ASIO2_USE_WEBSOCKET_RPC)
	/// Using tcp dgram mode as the underlying communication support
	using rpc_rate_session = rpc_rate_session_use<asio2::net_protocol::tcp>;
#else
	/// Using websocket as the underlying communication support
	using rpc_rate_session = rpc_rate_session_use<asio2::net_protocol::ws>;
#endif
}
#endif

#include <asio2/base/detail/pop_options.hpp>

#endif

#endif // !__ASIO2_RPC_SESSION_HPP__
