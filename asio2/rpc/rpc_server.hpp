/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
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
		, public invoker<typename executor_t::session_type>
	{
		friend executor_t;
		template <class>        friend class invoker;
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
			, invoker<typename executor_t::session_type>()
		{
		}

		/**
		 * @destructor
		 */
		~rpc_server_impl_t()
		{
			this->stop();
			this->iopool_.stop();
		}

		/**
		 * @function : start the server
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
					super::bind_recv([this](std::shared_ptr<session_type>& session_ptr, std::string_view s)
					{
						session_ptr->_handle_recv(session_ptr, s);
					});
				}

				return super::start(std::forward<Args>(args)...);
			}
			catch (system_error & e) { set_last_error(e); }
			return false;
		}

	public:
		/**
		 * @function : bind recv listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * Function signature : void(auto& session_ptr, std::string_view s)
		 */
		template<class F, class ...C>
		inline derived_t & bind_recv(F&& fun, C&&... obj)
		{
			auto user_fn = std::move(observer_t<std::shared_ptr<session_type>&, std::string_view>(
				std::forward<F>(fun), std::forward<C>(obj)...).move());
			auto fn = [this, user_fun = std::move(user_fn)]
			(std::shared_ptr<session_type>& session_ptr, std::string_view s)
			{
				user_fun(session_ptr, s);
				session_ptr->_handle_recv(session_ptr, s);
			};
			return super::bind_recv(std::move(fn));
		}

		/**
		 * @function : bind send listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * Function signature : void(auto& session_ptr, std::string_view s)
		 */
		template<class F, class ...C>
		inline derived_t & bind_send(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::send,
				observer_t<std::shared_ptr<session_type>&, std::string_view>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	protected:
		inline std::shared_ptr<session_type> _make_session()
		{
			std::shared_ptr<session_type> ptr = super::_make_session();
			ptr->_invoker(this);
			return ptr;
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
