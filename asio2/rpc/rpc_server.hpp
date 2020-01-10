/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_RPC_SERVER_HPP__
#define __ASIO2_RPC_SERVER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/tcp/tcp_server.hpp>
#include <asio2/tcp/tcps_server.hpp>
#include <asio2/http/ws_server.hpp>
#include <asio2/http/wss_server.hpp>

#include <asio2/rpc/rpc_session.hpp>

namespace asio2::detail
{
	template<class derived_t, class executor_t>
	class rpc_server_impl_t
		: public executor_t
		, public invoker_t<typename executor_t::session_type>
	{
		friend executor_t;
		template <class>        friend class invoker_t;
		template <class, bool>  friend class user_timer_cp;
		template <class, class> friend class server_impl_t;
		template <class, class> friend class tcp_server_impl_t;
		template <class, class> friend class tcps_server_impl_t;
		template <class, class> friend class ws_server_impl_t;
		template <class, class> friend class wss_server_impl_t;

	public:
		using self = rpc_server_impl_t<derived_t, executor_t>;
		using super = executor_t;
		using executor_type = executor_t;
		using session_type = typename super::session_type;

	protected:
		using super::send;

	public:
		/**
		 * @constructor
		 */
		template<class ...Args>
		explicit rpc_server_impl_t(
			Args&&... args
		)
			: super(std::forward<Args>(args)...)
			, invoker_t<typename executor_t::session_type>()
		{
		}

		/**
		 * @destructor
		 */
		~rpc_server_impl_t()
		{
			this->stop();
		}

	protected:
		template<typename... Args>
		inline std::shared_ptr<session_type> _make_session(Args&&... args)
		{
			return super::_make_session(*this, std::forward<Args>(args)...);
		}

	protected:
	};
}

namespace asio2
{
#if 1
	/// Using tcp dgram mode as the underlying communication support
	class rpc_server : public detail::rpc_server_impl_t<rpc_server, detail::tcp_server_impl_t<rpc_server, rpc_session>>
	{
	public:
		using detail::rpc_server_impl_t<rpc_server, detail::tcp_server_impl_t<rpc_server, rpc_session>>::rpc_server_impl_t;
	};

	#if defined(ASIO2_USE_SSL)
	class rpcs_server : public detail::rpc_server_impl_t<rpcs_server, detail::tcps_server_impl_t<rpcs_server, rpcs_session>>
	{
	public:
		using detail::rpc_server_impl_t<rpcs_server, detail::tcps_server_impl_t<rpcs_server, rpcs_session>>::rpc_server_impl_t;
	};
	#endif
#else
	/// Using websocket as the underlying communication support
	#ifndef ASIO_STANDALONE
	class rpc_server : public detail::rpc_server_impl_t<rpc_server, detail::ws_server_impl_t<rpc_server, rpc_session>>
	{
	public:
		using detail::rpc_server_impl_t<rpc_server, detail::ws_server_impl_t<rpc_server, rpc_session>>::rpc_server_impl_t;
	};

	#if defined(ASIO2_USE_SSL)
	class rpcs_server : public detail::rpc_server_impl_t<rpcs_server, detail::wss_server_impl_t<rpcs_server, rpcs_session>>
	{
	public:
		using detail::rpc_server_impl_t<rpcs_server, detail::wss_server_impl_t<rpcs_server, rpcs_session>>::rpc_server_impl_t;
	};
	#endif
	#endif
#endif
}

#endif // !__ASIO2_RPC_SERVER_HPP__
