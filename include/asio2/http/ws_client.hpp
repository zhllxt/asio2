/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_WS_CLIENT_HPP__
#define __ASIO2_WS_CLIENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/tcp/tcp_client.hpp>
#include <asio2/http/detail/http_util.hpp>
#include <asio2/http/component/ws_stream_cp.hpp>
#include <asio2/http/impl/ws_send_op.hpp>

namespace asio2::detail
{
	struct template_args_ws_client : public template_args_tcp_client
	{
		using stream_t    = websocket::stream<asio::ip::tcp::socket&>;
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

		using body_type   = typename args_t::body_t;
		using buffer_type = typename args_t::buffer_t;

		using ws_stream_comp = ws_stream_cp<derived_t, args_t>;

		using super::send;
		using super::async_send;

	public:
		/**
		 * @constructor
		 */
		template<class... Args>
		explicit ws_client_impl_t(Args&&... args)
			: super(std::forward<Args>(args)...)
			, ws_stream_cp<derived_t, args_t>()
			, ws_send_op  <derived_t, args_t>()
		{
		}

		/**
		 * @destructor
		 */
		~ws_client_impl_t()
		{
			this->stop();
		}

		/**
		 * @function : start the client, blocking connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
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
					condition_helper::make_condition('0', std::forward<Args>(args)...));
		}

		/**
		 * @function : start the client, asynchronous connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
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
					condition_helper::make_condition('0', std::forward<Args>(args)...));
		}

		/**
		 * @function : get the websocket upgraged response object
		 */
		inline const http::response_t<body_type>& upgrade_response() noexcept { return this->upgrade_rep_; }

		/**
		 * @function : get the websocket upgraged target
		 */
		inline const std::string& upgrade_target() noexcept { return this->upgrade_target_; }

		/**
		 * @function : set the websocket upgraged target
		 */
		inline derived_t & upgrade_target(std::string target)
		{
			this->upgrade_target_ = std::move(target);
			return (this->derived());
		}

	public:
		/**
		 * @function : bind websocket upgrade listener
		 * @param    : fun - a user defined callback function
		 * Function signature : void(asio::error_code ec)
		 */
		template<class F, class ...C>
		inline derived_t & bind_upgrade(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::upgrade,
				observer_t<error_code>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	protected:
		template<bool IsAsync, typename String, typename StrOrInt, typename Arg1, typename... Args>
		bool _do_connect_with_target(String&& host, StrOrInt&& port, Arg1&& arg1, Args&&... args)
		{
			if constexpr (detail::can_convert_to_string<detail::remove_cvref_t<Arg1>>::value)
			{
				this->derived().upgrade_target(std::forward<Arg1>(arg1));

				return this->derived().template _do_connect<IsAsync>(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					condition_helper::make_condition('0', std::forward<Args>(args)...));
			}
			else
			{
				return this->derived().template _do_connect<IsAsync>(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					condition_helper::make_condition('0',
						std::forward<Arg1>(arg1), std::forward<Args>(args)...));
			}
		}

		template<typename MatchCondition>
		inline void _do_init(condition_wrap<MatchCondition> condition)
		{
			super::_do_init(condition);

			this->derived()._ws_init(condition, this->socket_);
		}

		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			this->derived()._ws_stop(this_ptr, [this, ec, this_ptr]() mutable
			{
				super::_handle_disconnect(ec, std::move(this_ptr));
			});
		}

		template<typename MatchCondition>
		inline void _handle_connect(const error_code & ec, std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition)
		{
			set_last_error(ec);

			if (ec)
				return this->derived()._done_connect(ec, std::move(this_ptr), std::move(condition));

			this->derived()._ws_start(this_ptr, condition, this->socket_);

			this->derived()._post_control_callback(this_ptr, condition);
			this->derived()._post_upgrade(std::move(this_ptr), std::move(condition), this->upgrade_rep_);
		}

		template<class Data, class Callback>
		inline bool _do_send(Data& data, Callback&& callback)
		{
			return this->derived()._ws_send(data, std::forward<Callback>(callback));
		}

	protected:
		template<typename MatchCondition>
		inline void _post_recv(std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition)
		{
			this->derived()._ws_post_recv(std::move(this_ptr), std::move(condition));
		}

		template<typename MatchCondition>
		inline void _handle_recv(const error_code & ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived()._ws_handle_recv(ec, bytes_recvd, std::move(this_ptr), std::move(condition));
		}

		inline void _fire_upgrade(std::shared_ptr<derived_t>& this_ptr, error_code ec)
		{
			// the _fire_upgrade must be executed in the thread 0.
			ASIO2_ASSERT(this->derived().io().strand().running_in_this_thread());

			detail::ignore_unused(this_ptr);

			this->listener_.notify(event_type::upgrade, ec);
		}

	protected:
		http::response_t<body_type> upgrade_rep_;

		std::string                 upgrade_target_ = "/";
	};
}

namespace asio2
{
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

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_WS_CLIENT_HPP__
