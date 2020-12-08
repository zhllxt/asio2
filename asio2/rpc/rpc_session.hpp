/*
 * COPYRIGHT (C) 2017-2019, zhllxt
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
		, public rpc_call_cp<derived_t>
		, public rpc_recv_op<derived_t>
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
		using super::_handle_recv;

	public:
		/**
		 * @constructor
		 */
		template<class ...Args>
		explicit rpc_session_impl_t(
			rpc_invoker_t<derived_t>& invoker,
			Args&&... args
		)
			: super(std::forward<Args>(args)...)
			, rpc_call_cp<derived_t>(this->io_, this->serializer_, this->deserializer_)
			, rpc_recv_op<derived_t>()
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
		inline rpc_invoker_t<derived_t>& _invoker()
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
			this->listener_.notify(event_type::recv, this_ptr, s);

			this->derived()._rpc_handle_recv(this_ptr, s, condition);
		}

	protected:
		rpc_serializer                          serializer_;
		rpc_deserializer                        deserializer_;
		rpc_header                              header_;
		rpc_invoker_t<derived_t>              & invoker_;
	};
}

namespace asio2
{
	namespace detail
	{
		struct template_args_rpc_tcp_session : public template_args_tcp_session
		{
			static constexpr bool rdc_call_cp_enabled = false;
		};

		struct template_args_rpc_ws_session : public template_args_ws_session
		{
			static constexpr bool rdc_call_cp_enabled = false;
		};
	}

	template<class>
	class rpc_session_t;

	/// Using tcp dgram mode as the underlying communication support
	template<>
	class rpc_session_t<detail::use_tcp> : public detail::rpc_session_impl_t<rpc_session_t<detail::use_tcp>,
		detail::tcp_session_impl_t<rpc_session_t<detail::use_tcp>, detail::template_args_rpc_tcp_session>>
	{
	public:
		using detail::rpc_session_impl_t<rpc_session_t<detail::use_tcp>, detail::tcp_session_impl_t<
			rpc_session_t<detail::use_tcp>, detail::template_args_rpc_tcp_session>>::rpc_session_impl_t;
	};

	/// Using websocket as the underlying communication support
	template<>
	class rpc_session_t<detail::use_websocket> : public detail::rpc_session_impl_t<
		rpc_session_t<detail::use_websocket>, detail::ws_session_impl_t<
		rpc_session_t<detail::use_websocket>, detail::template_args_rpc_ws_session>>
	{
	public:
		using detail::rpc_session_impl_t<rpc_session_t<detail::use_websocket>,
			detail::ws_session_impl_t<rpc_session_t<detail::use_websocket>,
			detail::template_args_rpc_ws_session>>::rpc_session_impl_t;
	};

#if !defined(ASIO2_USE_WEBSOCKET_RPC)
	using rpc_session = rpc_session_t<detail::use_tcp>;
#else
	using rpc_session = rpc_session_t<detail::use_websocket>;
#endif

#if defined(ASIO2_USE_SSL)
	namespace detail
	{
		struct template_args_rpc_wss_session : public template_args_wss_session
		{
			static constexpr bool rdc_call_cp_enabled = false;
		};
	}

	template<class>
	class rpcs_session_t;

	template<>
	class rpcs_session_t<detail::use_tcp> : public detail::rpc_session_impl_t<rpcs_session_t<detail::use_tcp>,
		detail::tcps_session_impl_t<rpcs_session_t<detail::use_tcp>, detail::template_args_rpc_tcp_session>>
	{
	public:
		using detail::rpc_session_impl_t<rpcs_session_t<detail::use_tcp>, detail::tcps_session_impl_t<
			rpcs_session_t<detail::use_tcp>, detail::template_args_rpc_tcp_session>>::rpc_session_impl_t;
	};

	template<>
	class rpcs_session_t<detail::use_websocket> : public detail::rpc_session_impl_t<
		rpcs_session_t<detail::use_websocket>,
		detail::wss_session_impl_t<rpcs_session_t<detail::use_websocket>,
		detail::template_args_rpc_wss_session>>
	{
	public:
		using detail::rpc_session_impl_t<rpcs_session_t<detail::use_websocket>,
			detail::wss_session_impl_t<rpcs_session_t<detail::use_websocket>,
			detail::template_args_rpc_wss_session>>::rpc_session_impl_t;
	};

#if !defined(ASIO2_USE_WEBSOCKET_RPC)
	using rpcs_session = rpcs_session_t<detail::use_tcp>;
#else
	using rpcs_session = rpcs_session_t<detail::use_websocket>;
#endif
#endif
}

#endif // !__ASIO2_RPC_SESSION_HPP__
