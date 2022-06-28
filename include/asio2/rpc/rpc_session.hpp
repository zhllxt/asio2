/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_RPC_SESSION_HPP__
#define __ASIO2_RPC_SESSION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if __has_include(<cereal/cereal.hpp>)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/config.hpp>

#include <asio2/tcp/tcp_session.hpp>
#include <asio2/tcp/tcps_session.hpp>
#include <asio2/http/ws_session.hpp>
#include <asio2/http/wss_session.hpp>

#include <asio2/rpc/detail/rpc_serialization.hpp>
#include <asio2/rpc/detail/rpc_protocol.hpp>
#include <asio2/rpc/detail/rpc_invoker.hpp>
#include <asio2/rpc/impl/rpc_recv_op.hpp>
#include <asio2/rpc/component/rpc_call_cp.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
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
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;

	public:
		using super = executor_t;
		using self  = rpc_session_impl_t<derived_t, executor_t>;

		using executor_type = executor_t;

	protected:
		using super::send;
		using super::async_send;

	public:
		/**
		 * @constructor
		 */
		template<class ...Args>
		explicit rpc_session_impl_t(
			rpc_invoker_t<derived_t, typename executor_t::args_type>& invoker,
			Args&&... args
		)
			: super(std::forward<Args>(args)...)
			, rpc_call_cp<derived_t, typename executor_t::args_type>(this->io_, this->serializer_, this->deserializer_)
			, rpc_recv_op<derived_t, typename executor_t::args_type>()
			, id_maker<typename rpc_header::id_type>()
			, invoker_(invoker)
		{
		}

		/**
		 * @destructor
		 */
		~rpc_session_impl_t()
		{
		}

	protected:
		inline rpc_invoker_t<derived_t, typename executor_t::args_type>& _invoker() noexcept
		{
			return (this->invoker_);
		}

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
			this->listener_.notify(event_type::recv, this_ptr, data);

			this->derived()._rpc_handle_recv(this_ptr, data, condition);
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
		struct template_args_rpc_session<asio2::net_protocol::tcp> : public template_args_tcp_session
		{
			static constexpr bool rdc_call_cp_enabled = false;
		};

		template<>
		struct template_args_rpc_session<asio2::net_protocol::ws> : public template_args_ws_session
		{
			static constexpr bool rdc_call_cp_enabled = false;
		};
	}

	template<class derived_t, asio2::net_protocol np> class rpc_session_t;

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

	template<asio2::net_protocol np> class rpc_session_use;

	template<>
	class rpc_session_use<asio2::net_protocol::tcp>
		: public rpc_session_t<rpc_session_use<asio2::net_protocol::tcp>, asio2::net_protocol::tcp>
	{
	public:
		using rpc_session_t<rpc_session_use<asio2::net_protocol::tcp>, asio2::net_protocol::tcp>::rpc_session_t;
	};

	template<>
	class rpc_session_use<asio2::net_protocol::ws>
		: public rpc_session_t<rpc_session_use<asio2::net_protocol::ws>, asio2::net_protocol::ws>
	{
	public:
		using rpc_session_t<rpc_session_use<asio2::net_protocol::ws>, asio2::net_protocol::ws>::rpc_session_t;
	};

#if defined(ASIO2_USE_SSL)
	namespace detail
	{
		template<asio2::net_protocol np> struct template_args_rpcs_session;

		template<>
		struct template_args_rpcs_session<asio2::net_protocol::tcps> : public template_args_tcp_session
		{
			static constexpr bool rdc_call_cp_enabled = false;
		};

		template<>
		struct template_args_rpcs_session<asio2::net_protocol::wss> : public template_args_wss_session
		{
			static constexpr bool rdc_call_cp_enabled = false;
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
#endif

#if !defined(ASIO2_USE_WEBSOCKET_RPC)
	/// Using tcp dgram mode as the underlying communication support
	using rpc_session = rpc_session_use<asio2::net_protocol::tcp>;
#else
	/// Using websocket as the underlying communication support
	using rpc_session = rpc_session_use<asio2::net_protocol::ws>;
#endif

#if defined(ASIO2_USE_SSL)
#if !defined(ASIO2_USE_WEBSOCKET_RPC)
	/// Using tcp dgram mode as the underlying communication support
	using rpcs_session = rpcs_session_use<asio2::net_protocol::tcps>;
#else
	/// Using websocket as the underlying communication support
	using rpcs_session = rpcs_session_use<asio2::net_protocol::wss>;
#endif
#endif
}

#include <asio2/base/detail/pop_options.hpp>

#endif

#endif // !__ASIO2_RPC_SESSION_HPP__
