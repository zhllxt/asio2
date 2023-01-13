/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#if defined(ASIO2_ENABLE_SSL) || defined(ASIO2_USE_SSL)

#ifndef __ASIO2_RPCS_SESSION_HPP__
#define __ASIO2_RPCS_SESSION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if __has_include(<cereal/cereal.hpp>)

#include <asio2/rpc/rpc_session.hpp>

namespace asio2
{
	namespace detail
	{
		template<asio2::net_protocol np> struct template_args_rpcs_session;

		template<>
		struct template_args_rpcs_session<asio2::net_protocol::tcps> : public template_args_tcp_session
		{
			static constexpr asio2::net_protocol net_protocol = asio2::net_protocol::tcps;

			static constexpr bool rdc_call_cp_enabled = false;

			static constexpr std::size_t function_storage_size = 72;
		};

		template<>
		struct template_args_rpcs_session<asio2::net_protocol::wss> : public template_args_wss_session
		{
			static constexpr asio2::net_protocol net_protocol = asio2::net_protocol::wss;

			static constexpr bool rdc_call_cp_enabled = false;

			static constexpr std::size_t function_storage_size = 72;
		};
	}


	using rpcs_session_args_tcp = detail::template_args_rpcs_session<asio2::net_protocol::tcps>;
	using rpcs_session_args_ws  = detail::template_args_rpcs_session<asio2::net_protocol::wss >;


	template<class derived_t, asio2::net_protocol np> class rpcs_session_t;

	template<class derived_t>
	class rpcs_session_t<derived_t, asio2::net_protocol::tcps> : public detail::rpc_session_impl_t<derived_t,
		detail::tcps_session_impl_t<derived_t, detail::template_args_rpcs_session<asio2::net_protocol::tcps>>>
	{
	public:
		using detail::rpc_session_impl_t<derived_t, detail::tcps_session_impl_t<
			derived_t, detail::template_args_rpcs_session<asio2::net_protocol::tcps>>>::rpc_session_impl_t;
	};

	template<class derived_t>
	class rpcs_session_t<derived_t, asio2::net_protocol::wss> : public detail::rpc_session_impl_t<derived_t,
		detail::wss_session_impl_t<derived_t, detail::template_args_rpcs_session<asio2::net_protocol::wss>>>
	{
	public:
		using detail::rpc_session_impl_t<derived_t, detail::wss_session_impl_t<
			derived_t, detail::template_args_rpcs_session<asio2::net_protocol::wss>>>::rpc_session_impl_t;
	};

	template<asio2::net_protocol np>
	class rpcs_session_use : public rpcs_session_t<rpcs_session_use<np>, np>
	{
	public:
		using rpcs_session_t<rpcs_session_use<np>, np>::rpcs_session_t;
	};

#if !defined(ASIO2_USE_WEBSOCKET_RPC)
	/// Using tcp dgram mode as the underlying communication support
	using rpcs_session = rpcs_session_use<asio2::net_protocol::tcps>;
#else
	/// Using websocket as the underlying communication support
	using rpcs_session = rpcs_session_use<asio2::net_protocol::wss>;
#endif
}

#if defined(ASIO2_INCLUDE_RATE_LIMIT)
#include <asio2/tcp/tcp_stream.hpp>
namespace asio2
{
	struct rpcs_rate_session_args_tcp : public rpcs_session_args_tcp
	{
		using socket_t = asio2::tcp_stream<asio2::simple_rate_policy>;
	};
	struct rpcs_rate_session_args_ws : public rpcs_session_args_ws
	{
		using socket_t = asio2::tcp_stream<asio2::simple_rate_policy>;
		using stream_t = websocket::stream<asio::ssl::stream<socket_t&>&>;
	};

	template<class derived_t, asio2::net_protocol np> class rpcs_rate_session_t;

	template<class derived_t>
	class rpcs_rate_session_t<derived_t, asio2::net_protocol::tcps> : public detail::rpc_session_impl_t<derived_t,
		detail::tcps_session_impl_t<derived_t, rpcs_rate_session_args_tcp>>
	{
	public:
		using detail::rpc_session_impl_t<derived_t,
			detail::tcps_session_impl_t<derived_t, rpcs_rate_session_args_tcp>>::rpc_session_impl_t;
	};

	template<class derived_t>
	class rpcs_rate_session_t<derived_t, asio2::net_protocol::wss> : public detail::rpc_session_impl_t<derived_t,
		detail::wss_session_impl_t<derived_t, rpcs_rate_session_args_ws>>
	{
	public:
		using detail::rpc_session_impl_t<derived_t,
			detail::wss_session_impl_t<derived_t, rpcs_rate_session_args_ws>>::rpc_session_impl_t;
	};

	template<asio2::net_protocol np>
	class rpcs_rate_session_use : public rpcs_rate_session_t<rpcs_rate_session_use<np>, np>
	{
	public:
		using rpcs_rate_session_t<rpcs_rate_session_use<np>, np>::rpcs_rate_session_t;
	};

#if !defined(ASIO2_USE_WEBSOCKET_RPC)
	/// Using tcp dgram mode as the underlying communication support
	using rpcs_rate_session = rpcs_rate_session_use<asio2::net_protocol::tcps>;
#else
	/// Using websocket as the underlying communication support
	using rpcs_rate_session = rpcs_rate_session_use<asio2::net_protocol::wss>;
#endif
}
#endif

#endif

#endif // !__ASIO2_RPCS_SESSION_HPP__

#endif
