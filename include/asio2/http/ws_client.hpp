/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_WS_CLIENT_HPP__
#define __ASIO2_WS_CLIENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/tcp/tcp_client.hpp>
#include <asio2/http/detail/http_util.hpp>
#include <asio2/http/impl/ws_stream_cp.hpp>
#include <asio2/http/impl/ws_send_op.hpp>

namespace asio2::detail
{
	struct template_args_ws_client : public template_args_tcp_client
	{
		using stream_t    = websocket::stream<typename template_args_tcp_client::socket_t&>;
		using body_t      = http::string_body;
		using buffer_t    = beast::flat_buffer;
	};

	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;

	template<class derived_t, class args_t = template_args_ws_client>
	class ws_client_impl_t
		: public tcp_client_impl_t<derived_t, args_t>
		, public ws_stream_cp     <derived_t, args_t>
		, public ws_send_op       <derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;

	public:
		using super = tcp_client_impl_t<derived_t, args_t>;
		using self  = ws_client_impl_t <derived_t, args_t>;

		using args_type   = args_t;
		using body_type   = typename args_t::body_t;
		using buffer_type = typename args_t::buffer_t;

		using ws_stream_comp = ws_stream_cp<derived_t, args_t>;

		using super::send;
		using super::async_send;

	public:
		/**
		 * @brief constructor
		 */
		template<class... Args>
		explicit ws_client_impl_t(Args&&... args)
			: super(std::forward<Args>(args)...)
			, ws_stream_cp<derived_t, args_t>()
			, ws_send_op  <derived_t, args_t>()
		{
		}

		/**
		 * @brief destructor
		 */
		~ws_client_impl_t()
		{
			this->stop();
		}

		/**
		 * @brief return the websocket stream object reference
		 */
		inline typename args_t::stream_t & stream() noexcept
		{
			return this->derived().ws_stream();
		}

		/**
		 * @brief return the websocket stream object reference
		 */
		inline typename args_t::stream_t const& stream() const noexcept
		{
			return this->derived().ws_stream();
		}

		/**
		 * @brief start the client, blocking connect to server
		 * @param host - A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port - A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 * @param args - The args can be include the upgraged target.
		 * eg: start("127.0.0.1", 8883); start("127.0.0.1", 8883, "/admin");
		 * the "/admin" is the websocket upgraged target
		 */
		template<typename String, typename StrOrInt, typename... Args>
		inline bool start(String&& host, StrOrInt&& port, Args&&... args)
		{
			if constexpr (sizeof...(Args) > std::size_t(0))
				return this->derived().template _do_connect_with_target<false>(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					std::forward<Args>(args)...);
			else
				return this->derived().template _do_connect<false>(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					ecs_helper::make_ecs('0', std::forward<Args>(args)...));
		}

		/**
		 * @brief start the client, asynchronous connect to server
		 * @param host - A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port - A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 * @param args - The args can be include the upgraged target.
		 * eg: async_start("127.0.0.1", 8883); async_start("127.0.0.1", 8883, "/admin");
		 * the "/admin" is the websocket upgraged target
		 */
		template<typename String, typename StrOrInt, typename... Args>
		inline bool async_start(String&& host, StrOrInt&& port, Args&&... args)
		{
			if constexpr (sizeof...(Args) > std::size_t(0))
				return this->derived().template _do_connect_with_target<true>(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					std::forward<Args>(args)...);
			else
				return this->derived().template _do_connect<true>(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					ecs_helper::make_ecs('0', std::forward<Args>(args)...));
		}

		/**
		 * @brief destroy the content of all member variables, this is used for solve the memory leaks.
		 * After this function is called, this class object cannot be used again.
		 */
		inline void destroy()
		{
			derived_t& derive = this->derived();

			derive.ws_stream_.reset();

			super::destroy();
		}

		/**
		 * @brief get the websocket upgraged response object
		 */
		inline       websocket::response_type& get_upgrade_response()      noexcept { return this->upgrade_rep_; }

		/**
		 * @brief get the websocket upgraged response object
		 */
		inline const websocket::response_type& get_upgrade_response() const noexcept { return this->upgrade_rep_; }

		/**
		 * @brief get the websocket upgraged target
		 */
		inline const std::string& get_upgrade_target() const noexcept { return this->upgrade_target_; }

		/**
		 * @brief set the websocket upgraged target
		 */
		inline derived_t & set_upgrade_target(std::string target)
		{
			this->upgrade_target_ = std::move(target);
			return (this->derived());
		}

	public:
		/**
		 * @brief bind websocket upgrade listener
		 * @param fun - a user defined callback function.
		 * Function signature : void()
		 */
		template<class F, class ...C>
		inline derived_t & bind_upgrade(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::upgrade,
				observer_t<>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	protected:
		template<bool IsAsync, typename String, typename StrOrInt, typename Arg1, typename... Args>
		bool _do_connect_with_target(String&& host, StrOrInt&& port, Arg1&& arg1, Args&&... args)
		{
			if constexpr (detail::can_convert_to_string<detail::remove_cvref_t<Arg1>>::value)
			{
				this->derived().set_upgrade_target(std::forward<Arg1>(arg1));

				return this->derived().template _do_connect<IsAsync>(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					ecs_helper::make_ecs('0', std::forward<Args>(args)...));
			}
			else
			{
				return this->derived().template _do_connect<IsAsync>(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					ecs_helper::make_ecs('0',
						std::forward<Arg1>(arg1), std::forward<Args>(args)...));
			}
		}

		template<typename C>
		inline void _do_init(std::shared_ptr<ecs_t<C>>& ecs)
		{
			super::_do_init(ecs);

			this->derived()._ws_init(ecs, this->derived().socket());
		}

		template<typename DeferEvent>
		inline void _post_shutdown(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			ASIO2_LOG_DEBUG("ws_client::_post_shutdown: {} {}", ec.value(), ec.message());

			this->derived()._ws_stop(this_ptr, defer_event
			{
				[this, ec, this_ptr, e = chain.move_event()] (event_queue_guard<derived_t> g) mutable
				{
					super::_post_shutdown(ec, std::move(this_ptr), defer_event(std::move(e), std::move(g)));
				}, chain.move_guard()
			});
		}

		template<typename C, typename DeferEvent>
		inline void _handle_connect(
			const error_code& ec,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			set_last_error(ec);

			derived_t& derive = this->derived();

			if (ec)
			{
				return derive._done_connect(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
			}

			derive._ws_start(this_ptr, ecs, derive.socket());

			derive._post_control_callback(this_ptr, ecs);
			derive._post_upgrade(std::move(this_ptr), std::move(ecs), this->upgrade_rep_, std::move(chain));
		}

		template<class Data, class Callback>
		inline bool _do_send(Data& data, Callback&& callback)
		{
			return this->derived()._ws_send(data, std::forward<Callback>(callback));
		}

	protected:
		template<typename C>
		inline void _post_recv(std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			this->derived()._ws_post_recv(std::move(this_ptr), std::move(ecs));
		}

		template<typename C>
		inline void _handle_recv(
			const error_code& ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			this->derived()._ws_handle_recv(ec, bytes_recvd, std::move(this_ptr), std::move(ecs));
		}

		inline void _fire_upgrade(std::shared_ptr<derived_t>& this_ptr)
		{
			// the _fire_upgrade must be executed in the thread 0.
			ASIO2_ASSERT(this->derived().io_->running_in_this_thread());

			detail::ignore_unused(this_ptr);

			this->listener_.notify(event_type::upgrade);
		}

	protected:
		websocket::response_type  upgrade_rep_;

		std::string               upgrade_target_ = "/";
	};
}

namespace asio2
{
	using ws_client_args = detail::template_args_ws_client;

	template<class derived_t, class args_t>
	using ws_client_impl_t = detail::ws_client_impl_t<derived_t, args_t>;

	template<class derived_t>
	class ws_client_t : public detail::ws_client_impl_t<derived_t, detail::template_args_ws_client>
	{
	public:
		using detail::ws_client_impl_t<derived_t, detail::template_args_ws_client>::ws_client_impl_t;
	};

	class ws_client : public ws_client_t<ws_client>
	{
	public:
		using ws_client_t<ws_client>::ws_client_t;
	};
}

#if defined(ASIO2_INCLUDE_RATE_LIMIT)
#include <asio2/tcp/tcp_stream.hpp>
namespace asio2
{
	struct ws_rate_client_args : public ws_client_args
	{
		using socket_t = asio2::tcp_stream<asio2::simple_rate_policy>;
		using stream_t = websocket::stream<socket_t&>;
	};

	template<class derived_t>
	class ws_rate_client_t : public asio2::ws_client_impl_t<derived_t, ws_rate_client_args>
	{
	public:
		using asio2::ws_client_impl_t<derived_t, ws_rate_client_args>::ws_client_impl_t;
	};

	class ws_rate_client : public asio2::ws_rate_client_t<ws_rate_client>
	{
	public:
		using asio2::ws_rate_client_t<ws_rate_client>::ws_rate_client_t;
	};
}
#endif

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_WS_CLIENT_HPP__
