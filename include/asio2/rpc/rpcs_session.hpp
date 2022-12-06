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
			static constexpr bool rdc_call_cp_enabled = false;

			static constexpr std::size_t function_storage_size = 72;
		};

		template<>
		struct template_args_rpcs_session<asio2::net_protocol::wss> : public template_args_wss_session
		{
			static constexpr bool rdc_call_cp_enabled = false;

			static constexpr std::size_t function_storage_size = 72;
		};
	}

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

	template<asio2::net_protocol np> class rpcs_session_use;

	template<>
	class rpcs_session_use<asio2::net_protocol::tcps>
		: public rpcs_session_t<rpcs_session_use<asio2::net_protocol::tcps>, asio2::net_protocol::tcps>
	{
	public:
		using rpcs_session_t<rpcs_session_use<asio2::net_protocol::tcps>, asio2::net_protocol::tcps>::rpcs_session_t;
	};

	template<>
	class rpcs_session_use<asio2::net_protocol::wss>
		: public rpcs_session_t<rpcs_session_use<asio2::net_protocol::wss>, asio2::net_protocol::wss>
	{
	public:
		using rpcs_session_t<rpcs_session_use<asio2::net_protocol::wss>, asio2::net_protocol::wss>::rpcs_session_t;
	};

#if !defined(ASIO2_USE_WEBSOCKET_RPC)
	/// Using tcp dgram mode as the underlying communication support
	using rpcs_session = rpcs_session_use<asio2::net_protocol::tcps>;
#else
	/// Using websocket as the underlying communication support
	using rpcs_session = rpcs_session_use<asio2::net_protocol::wss>;
#endif
}

#endif

#endif // !__ASIO2_RPCS_SESSION_HPP__

#endif
