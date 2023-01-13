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

#ifndef __ASIO2_RPCS_SERVER_HPP__
#define __ASIO2_RPCS_SERVER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if __has_include(<cereal/cereal.hpp>)

#include <asio2/rpc/rpc_server.hpp>
#include <asio2/rpc/rpcs_session.hpp>

namespace asio2
{
	template<class derived_t, class session_t, asio2::net_protocol np = session_t::net_protocol>
	class rpcs_server_impl_t;

	template<class derived_t, class session_t>
	class rpcs_server_impl_t<derived_t, session_t, asio2::net_protocol::tcps>
		: public detail::rpc_server_impl_t<derived_t, detail::tcps_server_impl_t<derived_t, session_t>>
	{
	public:
		using detail::rpc_server_impl_t<derived_t, detail::tcps_server_impl_t<derived_t, session_t>>::
			rpc_server_impl_t;
	};

	template<class derived_t, class session_t>
	class rpcs_server_impl_t<derived_t, session_t, asio2::net_protocol::wss>
		: public detail::rpc_server_impl_t<derived_t, detail::wss_server_impl_t<derived_t, session_t>>
	{
	public:
		using detail::rpc_server_impl_t<derived_t, detail::wss_server_impl_t<derived_t, session_t>>::
			rpc_server_impl_t;
	};

	template<class session_t>
	class rpcs_server_t : public rpcs_server_impl_t<rpcs_server_t<session_t>, session_t>
	{
	public:
		using rpcs_server_impl_t<rpcs_server_t<session_t>, session_t>::rpcs_server_impl_t;
	};

	template<asio2::net_protocol np>
	class rpcs_server_use : public rpcs_server_t<rpcs_session_use<np>>
	{
	public:
		using rpcs_server_t<rpcs_session_use<np>>::rpcs_server_t;
	};

#if !defined(ASIO2_USE_WEBSOCKET_RPC)
	/// Using tcp dgram mode as the underlying communication support
	using rpcs_server = rpcs_server_use<asio2::net_protocol::tcps>;
#else
	/// Using websocket as the underlying communication support
	using rpcs_server = rpcs_server_use<asio2::net_protocol::wss>;
#endif
}

#if defined(ASIO2_INCLUDE_RATE_LIMIT)
#include <asio2/tcp/tcp_stream.hpp>
namespace asio2
{
	template<class derived_t, class session_t, asio2::net_protocol np = session_t::net_protocol>
	class rpcs_rate_server_impl_t;

	template<class derived_t, class session_t>
	class rpcs_rate_server_impl_t<derived_t, session_t, asio2::net_protocol::tcps>
		: public detail::rpc_server_impl_t<derived_t, detail::tcps_server_impl_t<derived_t, session_t>>
	{
	public:
		using detail::rpc_server_impl_t<derived_t, detail::tcps_server_impl_t<derived_t, session_t>>::
			rpc_server_impl_t;
	};

	template<class derived_t, class session_t>
	class rpcs_rate_server_impl_t<derived_t, session_t, asio2::net_protocol::wss>
		: public detail::rpc_server_impl_t<derived_t, detail::wss_server_impl_t<derived_t, session_t>>
	{
	public:
		using detail::rpc_server_impl_t<derived_t, detail::wss_server_impl_t<derived_t, session_t>>::
			rpc_server_impl_t;
	};

	template<class session_t>
	class rpcs_rate_server_t : public rpcs_rate_server_impl_t<rpcs_rate_server_t<session_t>, session_t>
	{
	public:
		using rpcs_rate_server_impl_t<rpcs_rate_server_t<session_t>, session_t>::rpcs_rate_server_impl_t;
	};

	template<asio2::net_protocol np>
	class rpcs_rate_server_use : public rpcs_rate_server_t<rpcs_rate_session_use<np>>
	{
	public:
		using rpcs_rate_server_t<rpcs_rate_session_use<np>>::rpcs_rate_server_t;
	};

#if !defined(ASIO2_USE_WEBSOCKET_RPC)
	/// Using tcp dgram mode as the underlying communication support
	using rpcs_rate_server = rpcs_rate_server_use<asio2::net_protocol::tcps>;
#else
	/// Using websocket as the underlying communication support
	using rpcs_rate_server = rpcs_rate_server_use<asio2::net_protocol::wss>;
#endif
}
#endif

#endif

#endif // !__ASIO2_RPCS_SERVER_HPP__

#endif
