/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_RPC_CLIENT_HPP__
#define __ASIO2_RPC_CLIENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/tcp/tcp_client.hpp>
#include <asio2/tcp/tcps_client.hpp>
#include <asio2/http/ws_client.hpp>
#include <asio2/http/wss_client.hpp>

#include <asio2/rpc/detail/protocol.hpp>
#include <asio2/rpc/detail/invoker.hpp>
#include <asio2/rpc/component/rpc_call_cp.hpp>
#include <asio2/rpc/impl/rpc_recv_op.hpp>

namespace asio2::detail
{
	template<class derived_t, class executor_t>
	class rpc_client_impl_t
		: public executor_t
		, public invoker<derived_t>
		, public rpc_call_cp<derived_t, false>
		, public rpc_recv_op<derived_t, false>
		, protected id_maker<typename header::id_type>
	{
		template <class>                             friend class invoker;
		template <class, bool>                       friend class user_timer_cp;
		template <class, bool>                       friend class connect_timeout_cp;
		template <class, class>                      friend class connect_cp;
		template <class, bool>                       friend class send_queue_cp;
		template <class, bool>                       friend class send_cp;
		template <class, bool>                       friend class tcp_send_op;
		template <class, bool>                       friend class tcp_recv_op;
		template <class, class, bool>                friend class ssl_stream_cp;
		template <class, class, bool>                friend class ws_stream_cp;
		template <class, bool>                       friend class ws_send_op;
		template <class, bool>                       friend class rpc_call_cp;
		template <class, bool>                       friend class rpc_recv_op;
		template <class, class, class>               friend class client_impl_t;
		template <class, class, class>               friend class tcp_client_impl_t;
		template <class, class, class>               friend class tcps_client_impl_t;
		template <class, class, class, class, class> friend class ws_client_impl_t;
		template <class, class, class, class, class> friend class wss_client_impl_t;

	public:
		using self = rpc_client_impl_t<derived_t, executor_t>;
		using super = executor_t;
		using executor_type = executor_t;

	protected:
		using super::send;
		using super::_handle_recv;

	public:
		/**
		 * @constructor
		 */
		template<class ...Args>
		explicit rpc_client_impl_t(
			Args&&... args
		)
			: super(std::forward<Args>(args)...)
			, invoker<derived_t>()
			, rpc_call_cp<derived_t, false>(this->io_, this->serializer_, this->deserializer_)
			, rpc_recv_op<derived_t, false>()
			, id_maker<typename header::id_type>()
		{
		}

		/**
		 * @destructor
		 */
		~rpc_client_impl_t()
		{
			this->stop();
			this->iopool_.stop();
		}

		/**
		 * @function : start the client, blocking connect to server
		 * @param args The arguments to be passed to start the underlying worker.
		 */
		template<class ...Args>
		inline bool start(Args&&... args)
		{
			try
			{
				state_t expected = state_t::stopped;
				if (!this->state_.compare_exchange_strong(expected, state_t::stopped))
					asio::detail::throw_error(asio::error::already_started);

				if (!this->listener_.find(event::recv))
				{
					super::bind_recv([this](std::string_view s)
					{
						this->derived()._handle_recv(std::shared_ptr<derived_t>{}, s);
					});
				}

				return super::start(std::forward<Args>(args)...);
			}
			catch (system_error & e) { set_last_error(e); }
			return false;
		}

		/**
		 * @function : start the client, asynchronous connect to server
		 * @param args The arguments to be passed to async_start the underlying worker.
		 */
		template<class ...Args>
		inline void async_start(Args&&... args)
		{
			try
			{
				state_t expected = state_t::stopped;
				if (!this->state_.compare_exchange_strong(expected, state_t::stopped))
					asio::detail::throw_error(asio::error::already_started);

				if (!this->listener_.find(event::recv))
				{
					super::bind_recv([this](std::string_view s)
					{
						this->derived()._handle_recv(std::shared_ptr<derived_t>{}, s);
					});
				}

				super::async_start(std::forward<Args>(args)...);
			}
			catch (system_error & e) { set_last_error(e); }
		}

		/**
		 * @function : set call rpc function timeout duration value
		 */
		template<class Rep, class Period>
		inline derived_t & timeout(std::chrono::duration<Rep, Period> duration)
		{
			this->timeout_ = duration;
			return (this->derived());
		}

		/**
		 * @function : get call rpc function timeout duration value
		 */
		inline std::chrono::steady_clock::duration timeout()
		{
			return this->timeout_;
		}

	public:
		/**
		 * @function : bind recv listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * Function signature : void(std::string_view s)
		 */
		template<class F, class ...C>
		inline derived_t & bind_recv(F&& fun, C&&... obj)
		{
			auto user_fn = std::move(observer_t<std::string_view>(
				std::forward<F>(fun), std::forward<C>(obj)...).move());
			auto fn = [this, user_fun = std::move(user_fn)](std::string_view s)
			{
				user_fun(s);
				this->derived()._handle_recv(std::shared_ptr<derived_t>{}, s);
			};
			return super::bind_recv(std::move(fn));
		}

		/**
		 * @function : bind send listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * Function signature : void(std::string_view s)
		 */
		template<class F, class ...C>
		inline derived_t & bind_send(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::send,
				observer_t<std::string_view>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	protected:
		inline void _handle_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			while (!this->reqs_.empty())
			{
				auto& fn = this->reqs_.begin()->second;
				fn(asio::error::operation_aborted, std::string_view{});
			}

			super::_handle_stop(ec, std::move(this_ptr));
		}

		inline void _handle_recv(std::shared_ptr<derived_t> this_ptr, std::string_view s)
		{
			this->derived()._rpc_handle_recv(this_ptr, s);
		}

		inline void _fire_send(detail::ignore, std::string_view s)
		{
			this->listener_.notify(event::send, s);
		}

	protected:
		detail::serializer serializer_;
		detail::deserializer deserializer_;
		detail::header header_;
		std::chrono::steady_clock::duration timeout_ = std::chrono::milliseconds(http_execute_timeout);
	};
}

namespace asio2
{
#if 1
	/// Using tcp dgram mode as the underlying communication support
	class rpc_client : public detail::rpc_client_impl_t<rpc_client,
		detail::tcp_client_impl_t<rpc_client, asio::ip::tcp::socket, asio::streambuf>>
	{
	public:
		using detail::rpc_client_impl_t<rpc_client, detail::tcp_client_impl_t<rpc_client,
			asio::ip::tcp::socket, asio::streambuf>>::rpc_client_impl_t;
	};

	#if defined(ASIO2_USE_SSL)
	class rpcs_client : public detail::rpc_client_impl_t<rpcs_client,
		detail::tcps_client_impl_t<rpcs_client, asio::ip::tcp::socket, asio::streambuf>>
	{
	public:
		using detail::rpc_client_impl_t<rpcs_client, detail::tcps_client_impl_t<rpcs_client,
			asio::ip::tcp::socket, asio::streambuf>>::rpc_client_impl_t;
	};
	#endif
#else
	/// Using websocket as the underlying communication support
	#ifndef ASIO_STANDALONE
	class rpc_client : public detail::rpc_client_impl_t<rpc_client,
		detail::ws_client_impl_t<rpc_client, asio::ip::tcp::socket,
		websocket::stream<asio::ip::tcp::socket&>, http::string_body, beast::flat_buffer>>
	{
	public:
		using detail::rpc_client_impl_t<rpc_client, detail::ws_client_impl_t<rpc_client,
			asio::ip::tcp::socket, websocket::stream<asio::ip::tcp::socket&>,
			http::string_body, beast::flat_buffer>>::rpc_client_impl_t;
	};

	#if defined(ASIO2_USE_SSL)
	class rpcs_client : public detail::rpc_client_impl_t<rpcs_client,
		detail::wss_client_impl_t<rpcs_client, asio::ip::tcp::socket,
		websocket::stream<asio::ssl::stream<asio::ip::tcp::socket&>&>, http::string_body, beast::flat_buffer>>
	{
	public:
		using detail::rpc_client_impl_t<rpcs_client, detail::wss_client_impl_t<rpcs_client,
			asio::ip::tcp::socket, websocket::stream<asio::ssl::stream<asio::ip::tcp::socket&>&>,
			http::string_body, beast::flat_buffer>>::rpc_client_impl_t;
	};
	#endif
	#endif
#endif
}

#endif // !__ASIO2_RPC_CLIENT_HPP__
