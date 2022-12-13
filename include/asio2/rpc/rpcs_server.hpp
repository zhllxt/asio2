/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
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
	template<class session_t, asio2::net_protocol np> class rpcs_server_t;

	template<class session_t>
	class rpcs_server_t<session_t, asio2::net_protocol::tcps> : public detail::rpc_server_impl_t<
		rpcs_server_t<session_t, asio2::net_protocol::tcps>, detail::tcps_server_impl_t<
		rpcs_server_t<session_t, asio2::net_protocol::tcps>, session_t>>
	{
	public:
		using detail::rpc_server_impl_t<
			rpcs_server_t<session_t, asio2::net_protocol::tcps>, detail::tcps_server_impl_t<
			rpcs_server_t<session_t, asio2::net_protocol::tcps>, session_t>>::rpc_server_impl_t;
	};

	template<class session_t>
	class rpcs_server_t<session_t, asio2::net_protocol::wss> : public detail::rpc_server_impl_t<
		rpcs_server_t<session_t, asio2::net_protocol::wss>, detail::wss_server_impl_t<
		rpcs_server_t<session_t, asio2::net_protocol::wss>, session_t>>
	{
	public:
		using detail::rpc_server_impl_t<
			rpcs_server_t<session_t, asio2::net_protocol::wss>, detail::wss_server_impl_t<
			rpcs_server_t<session_t, asio2::net_protocol::wss>, session_t>>::rpc_server_impl_t;
	};

	template<asio2::net_protocol np> class rpcs_server_use;

	template<>
	class rpcs_server_use<asio2::net_protocol::tcps>
		: public rpcs_server_t<rpcs_session_use<asio2::net_protocol::tcps>, asio2::net_protocol::tcps>
	{
	public:
		using rpcs_server_t<rpcs_session_use<asio2::net_protocol::tcps>, asio2::net_protocol::tcps>::rpcs_server_t;
	};

	template<>
	class rpcs_server_use<asio2::net_protocol::wss>
		: public rpcs_server_t<rpcs_session_use<asio2::net_protocol::wss>, asio2::net_protocol::wss>
	{
	public:
		using rpcs_server_t<rpcs_session_use<asio2::net_protocol::wss>, asio2::net_protocol::wss>::rpcs_server_t;
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
	template<class session_t, asio2::net_protocol np> class rpcs_rate_server_t;

	template<class session_t>
	class rpcs_rate_server_t<session_t, asio2::net_protocol::tcps> : public detail::rpc_server_impl_t<
		rpcs_rate_server_t<session_t, asio2::net_protocol::tcps>, detail::tcps_server_impl_t<
		rpcs_rate_server_t<session_t, asio2::net_protocol::tcps>, session_t>>
	{
	public:
		using detail::rpc_server_impl_t<
			rpcs_rate_server_t<session_t, asio2::net_protocol::tcps>, detail::tcps_server_impl_t<
			rpcs_rate_server_t<session_t, asio2::net_protocol::tcps>, session_t>>::rpc_server_impl_t;
	};

	template<class session_t>
	class rpcs_rate_server_t<session_t, asio2::net_protocol::wss> : public detail::rpc_server_impl_t<
		rpcs_rate_server_t<session_t, asio2::net_protocol::wss>, detail::wss_server_impl_t<
		rpcs_rate_server_t<session_t, asio2::net_protocol::wss>, session_t>>
	{
	public:
		using detail::rpc_server_impl_t<
			rpcs_rate_server_t<session_t, asio2::net_protocol::wss>, detail::wss_server_impl_t<
			rpcs_rate_server_t<session_t, asio2::net_protocol::wss>, session_t>>::rpc_server_impl_t;
	};

	template<asio2::net_protocol np> class rpcs_rate_server_use;

	template<>
	class rpcs_rate_server_use<asio2::net_protocol::tcps>
		: public rpcs_rate_server_t<rpcs_rate_session_use<asio2::net_protocol::tcps>, asio2::net_protocol::tcps>
	{
	public:
		using rpcs_rate_server_t<rpcs_rate_session_use<asio2::net_protocol::tcps>,
			asio2::net_protocol::tcps>::rpcs_rate_server_t;
	};

	template<>
	class rpcs_rate_server_use<asio2::net_protocol::wss>
		: public rpcs_rate_server_t<rpcs_rate_session_use<asio2::net_protocol::wss>, asio2::net_protocol::wss>
	{
	public:
		using rpcs_rate_server_t<rpcs_rate_session_use<asio2::net_protocol::wss>,
			asio2::net_protocol::wss>::rpcs_rate_server_t;
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
