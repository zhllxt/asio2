/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#if defined(ASIO2_USE_SSL)

#ifndef __ASIO2_WSS_CLIENT_HPP__
#define __ASIO2_WSS_CLIENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/tcp/tcps_client.hpp>

#include <asio2/http/ws_client.hpp>

namespace asio2::detail
{
	struct template_args_wss_client : public template_args_ws_client
	{
		using stream_t    = websocket::stream<asio::ssl::stream<asio::ip::tcp::socket&>&>;
	};

	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;

	template<class derived_t, class args_t = template_args_wss_client>
	class wss_client_impl_t
		: public tcps_client_impl_t<derived_t, args_t>
		, public ws_stream_cp      <derived_t, args_t>
		, public ws_send_op        <derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;

	public:
		using super = tcps_client_impl_t<derived_t, args_t>;
		using self  = wss_client_impl_t <derived_t, args_t>;

		using args_type   = args_t;
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
		explicit wss_client_impl_t(
			asio::ssl::context::method method = asio::ssl::context::sslv23,
			Args&&... args
		)
			: super(method, std::forward<Args>(args)...)
			, ws_stream_cp<derived_t, args_t>()
			, ws_send_op  <derived_t, args_t>()
		{
		}

		/**
		 * @destructor
		 */
		~wss_client_impl_t()
		{
			this->stop();
		}

		/**
		 * @function : start the client, blocking connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 * @param args The args can be include the upgraged target.
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
					condition_helper::make_condition('0', std::forward<Args>(args)...));
		}

		/**
		 * @function : start the client, asynchronous connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 * @param args The args can be include the upgraged target.
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
					condition_helper::make_condition('0', std::forward<Args>(args)...));
		}

		/**
		 * @function : get the websocket upgraged response object
		 */
		inline const http::response<body_type>& get_upgrade_response() noexcept { return this->upgrade_rep_; }

		/**
		 * @function : get the websocket upgraged target
		 */
		inline const std::string& get_upgrade_target() noexcept { return this->upgrade_target_; }

		/**
		 * @function : set the websocket upgraged target
		 */
		inline derived_t & set_upgrade_target(std::string target)
		{
			this->upgrade_target_ = std::move(target);
			return (this->derived());
		}

	public:
		/**
		 * @function : bind websocket upgrade listener
		 * @param    : fun - a user defined callback function
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

			this->derived()._ws_init(condition, this->ssl_stream());
		}

		template<typename DeferEvent>
		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			this->derived()._ws_stop(this_ptr,
				defer_event
				{
					[this, ec, this_ptr, e = chain.move_event()](event_queue_guard<derived_t> g) mutable
					{
						super::_handle_disconnect(ec, std::move(this_ptr), defer_event(std::move(e), std::move(g)));
					}, chain.move_guard()
				}
			);
		}

		template<typename MatchCondition, typename DeferEvent>
		inline void _handle_connect(const error_code & ec, std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition, DeferEvent chain)
		{
			set_last_error(ec);

			if (ec)
				return this->derived()._done_connect(ec, std::move(this_ptr), std::move(condition), std::move(chain));

			this->derived()._ssl_start(this_ptr, condition, this->socket_, *this);

			this->derived()._ws_start(this_ptr, condition, this->ssl_stream());

			this->derived()._post_handshake(std::move(this_ptr), std::move(condition), std::move(chain));
		}

		template<typename MatchCondition, typename DeferEvent>
		inline void _handle_handshake(const error_code & ec, std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition, DeferEvent chain)
		{
			set_last_error(ec);

			this->derived()._fire_handshake(this_ptr);

			if (ec)
			{
				return this->derived()._done_connect(ec, std::move(this_ptr), std::move(condition), std::move(chain));
			}

			this->derived()._post_control_callback(this_ptr, condition);
			this->derived()._post_upgrade(std::move(this_ptr), std::move(condition), this->upgrade_rep_, std::move(chain));
		}

		template<class Data, class Callback>
		inline bool _do_send(Data& data, Callback&& callback)
		{
			return this->derived()._ws_send(data, std::forward<Callback>(callback));
		}

	protected:
		template<typename MatchCondition>
		inline void _post_recv(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived()._ws_post_recv(std::move(this_ptr), std::move(condition));
		}

		template<typename MatchCondition>
		inline void _handle_recv(const error_code & ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived()._ws_handle_recv(ec, bytes_recvd, std::move(this_ptr), std::move(condition));
		}

		inline void _fire_upgrade(std::shared_ptr<derived_t>& this_ptr)
		{
			// the _fire_upgrade must be executed in the thread 0.
			ASIO2_ASSERT(this->derived().io().running_in_this_thread());

			detail::ignore_unused(this_ptr);

			this->listener_.notify(event_type::upgrade);
		}

	protected:
		http::response<body_type> upgrade_rep_;

		std::string               upgrade_target_ = "/";
	};
}

namespace asio2
{
	template<class derived_t>
	class wss_client_t : public detail::wss_client_impl_t<derived_t, detail::template_args_wss_client>
	{
	public:
		using detail::wss_client_impl_t<derived_t, detail::template_args_wss_client>::wss_client_impl_t;
	};

	class wss_client : public wss_client_t<wss_client>
	{
	public:
		using wss_client_t<wss_client>::wss_client_t;
	};
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_WSS_CLIENT_HPP__

#endif
