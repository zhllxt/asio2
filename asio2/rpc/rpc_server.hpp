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

	public:
		/**
		 * @function : call a rpc function for each session
		 */
		template<class T, class Rep, class Period, class ...Args>
		inline T call(std::chrono::duration<Rep, Period> timeout, const std::string& name, const Args&... args)
		{
			this->sessions_.foreach([&](std::shared_ptr<session_type>& session_ptr) mutable
			{
				session_ptr->template call<T>(timeout, name, args...);
			});
			if constexpr (std::is_void_v<T>)
				std::ignore = true;
			else
				return T{};
		}

		/**
		 * @function : call a rpc function for each session
		 */
		template<class T, class Rep, class Period, class ...Args>
		inline T call(error_code& ec, std::chrono::duration<Rep, Period> timeout, std::string name, const Args&... args)
		{
			this->sessions_.foreach([&](std::shared_ptr<session_type>& session_ptr) mutable
			{
				session_ptr->template call<T>(ec, timeout, name, args...);
			});
			if constexpr (std::is_void_v<T>)
				std::ignore = true;
			else
				return T{};
		}

		/**
		 * @function : call a rpc function for each session
		 */
		template<class T, class ...Args>
		inline T call(std::string name, const Args&... args)
		{
			this->sessions_.foreach([&](std::shared_ptr<session_type>& session_ptr) mutable
			{
				session_ptr->template call<T>(name, args...);
			});
			if constexpr (std::is_void_v<T>)
				std::ignore = true;
			else
				return T{};
		}

		/**
		 * @function : call a rpc function for each session
		 */
		template<class T, class ...Args>
		inline T call(error_code& ec, std::string name, const Args&... args)
		{
			this->sessions_.foreach([&](std::shared_ptr<session_type>& session_ptr) mutable
			{
				session_ptr->template call<T>(ec, name, args...);
			});
			if constexpr (std::is_void_v<T>)
				std::ignore = true;
			else
				return T{};
		}

		/**
		 * @function : asynchronous call a rpc function for each session
		 * Callback signature : void(error_code ec, int result)
		 * if result type is void, the Callback signature is : void(error_code ec)
		 * Because the result value type is not specified in the first template parameter,
		 * so the result value type must be specified in the Callback lambda.
		 * You must guarantee that the parameter args remain valid until the send operation is called.
		 */
		template<class Callback, class ...Args>
		inline typename std::enable_if_t<is_callable_v<Callback>, void>
			async_call(const Callback& fn, std::string name, const Args&... args)
		{
			this->sessions_.foreach([&](std::shared_ptr<session_type>& session_ptr) mutable
			{
				session_ptr->async_call(fn, name, args...);
			});
		}

		/**
		 * @function : asynchronous call a rpc function for each session
		 * Callback signature : void(error_code ec, int result)
		 * if result type is void, the Callback signature is : void(error_code ec)
		 * Because the result value type is not specified in the first template parameter,
		 * so the result value type must be specified in the Callback lambda
		 * You must guarantee that the parameter args remain valid until the send operation is called.
		 */
		template<class Callback, class Rep, class Period, class ...Args>
		inline typename std::enable_if_t<is_callable_v<Callback>, void>
		async_call(const Callback& fn, std::chrono::duration<Rep, Period> timeout, std::string name, const Args&... args)
		{
			this->sessions_.foreach([&](std::shared_ptr<session_type>& session_ptr) mutable
			{
				session_ptr->async_call(fn, timeout, name, args...);
			});
		}

		/**
		 * @function : asynchronous call a rpc function for each session
		 * Callback signature : void(error_code ec, T result) the T is the first template parameter.
		 * if result type is void, the Callback signature is : void(error_code ec)
		 * You must guarantee that the parameter args remain valid until the send operation is called.
		 */
		template<class T, class Callback, class ...Args>
		inline void async_call(const Callback& fn, std::string name, const Args&... args)
		{
			this->sessions_.foreach([&](std::shared_ptr<session_type>& session_ptr) mutable
			{
				session_ptr->template async_call<T>(fn, name, args...);
			});
		}

		/**
		 * @function : asynchronous call a rpc function for each session
		 * Callback signature : void(error_code ec, T result) the T is the first template parameter.
		 * if result type is void, the Callback signature is : void(error_code ec)
		 * You must guarantee that the parameter args remain valid until the send operation is called.
		 */
		template<class T, class Callback, class Rep, class Period, class ...Args>
		inline void async_call(const Callback& fn, std::chrono::duration<Rep, Period> timeout, std::string name, const Args&... args)
		{
			this->sessions_.foreach([&](std::shared_ptr<session_type>& session_ptr) mutable
			{
				session_ptr->template async_call<T>(fn, timeout, name, args...);
			});
		}

		/**
		 * @function : asynchronous call a rpc function for each session
		 * Don't care whether the call succeeds
		 */
		template<class String, class ...Args>
		inline typename std::enable_if_t<!is_callable_v<String>, void>
		async_call(const String& name, const Args&... args)
		{
			this->sessions_.foreach([&](std::shared_ptr<session_type>& session_ptr) mutable
			{
				session_ptr->async_call(name, args...);
			});
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
