/*
 * COPYRIGHT (C) 2017-2019, zhllxt
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

#include <asio2/tcp/tcps_client.hpp>
#include <asio2/http/detail/http_util.hpp>
#include <asio2/http/component/ws_stream_cp.hpp>
#include <asio2/http/impl/ws_send_op.hpp>

namespace asio2::detail
{
	struct template_args_wss_client : public template_args_ws_client
	{
		using stream_t    = websocket::stream<asio::ssl::stream<asio::ip::tcp::socket&>&>;
	};

	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;

	template<class derived_t, class args_t>
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

		using body_type   = typename args_t::body_t;
		using buffer_type = typename args_t::buffer_t;

		using ws_stream_comp = ws_stream_cp<derived_t, args_t>;

		using super::send;

	public:
		/**
		 * @constructor
		 */
		explicit wss_client_impl_t(
			asio::ssl::context::method method = asio::ssl::context::sslv23,
			std::size_t init_buffer_size      = tcp_frame_size,
			std::size_t max_buffer_size       = (std::numeric_limits<std::size_t>::max)()
		)
			: super(method, init_buffer_size, max_buffer_size)
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
		 */
		template<typename String, typename StrOrInt>
		bool start(String&& host, StrOrInt&& port)
		{
			return this->derived().template _do_connect<false>(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				condition_wrap<void>{});
		}

		/**
		 * @function : start the client, blocking connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt, typename ParserFun>
		bool start(String&& host, StrOrInt&& port, ParserFun&& parser)
		{
			using fun_traits_type = function_traits<std::remove_cv_t<std::remove_reference_t<ParserFun>>>;
			using IdT = typename fun_traits_type::return_type;
			using SendDataT = typename fun_traits_type::template args<0>::type;
			using RecvDataT = typename fun_traits_type::template args<0>::type;

			return this->derived().template _do_connect<false>(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				condition_wrap<use_rdc_t<void, IdT, SendDataT, RecvDataT>>(
					std::in_place,
					std::forward<ParserFun>(parser)));
		}

		/**
		 * @function : start the client, blocking connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt, typename SendParserFun, typename RecvParserFun>
		bool start(String&& host, StrOrInt&& port, SendParserFun&& send_parser, RecvParserFun&& recv_parser)
		{
			using send_fun_traits_type = function_traits<std::remove_cv_t<std::remove_reference_t<SendParserFun>>>;
			using recv_fun_traits_type = function_traits<std::remove_cv_t<std::remove_reference_t<RecvParserFun>>>;
			using SendIdT = typename send_fun_traits_type::return_type;
			using RecvIdT = typename recv_fun_traits_type::return_type;
			using SendDataT = typename send_fun_traits_type::template args<0>::type;
			using RecvDataT = typename recv_fun_traits_type::template args<0>::type;

			static_assert(std::is_same_v<SendIdT, RecvIdT>);

			return this->derived().template _do_connect<false>(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				condition_wrap<use_rdc_t<void, SendIdT, SendDataT, RecvDataT>>(
					std::in_place,
					std::forward<SendParserFun>(send_parser),
					std::forward<RecvParserFun>(recv_parser)));
		}

		/**
		 * @function : start the client, asynchronous connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt>
		bool async_start(String&& host, StrOrInt&& port)
		{
			return this->derived().template _do_connect<true>(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				condition_wrap<void>{});
		}

		/**
		 * @function : start the client, asynchronous connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt, typename ParserFun>
		bool async_start(String&& host, StrOrInt&& port, ParserFun&& parser)
		{
			using fun_traits_type = function_traits<std::remove_cv_t<std::remove_reference_t<ParserFun>>>;
			using IdT = typename fun_traits_type::return_type;
			using SendDataT = typename fun_traits_type::template args<0>::type;
			using RecvDataT = typename fun_traits_type::template args<0>::type;

			return this->derived().template _do_connect<true>(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				condition_wrap<use_rdc_t<void, IdT, SendDataT, RecvDataT>>(
					std::in_place,
					std::forward<ParserFun>(parser)));
		}

		/**
		 * @function : start the client, asynchronous connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt, typename SendParserFun, typename RecvParserFun>
		bool async_start(String&& host, StrOrInt&& port, SendParserFun&& send_parser, RecvParserFun&& recv_parser)
		{
			using send_fun_traits_type = function_traits<std::remove_cv_t<std::remove_reference_t<SendParserFun>>>;
			using recv_fun_traits_type = function_traits<std::remove_cv_t<std::remove_reference_t<RecvParserFun>>>;
			using SendIdT = typename send_fun_traits_type::return_type;
			using RecvIdT = typename recv_fun_traits_type::return_type;
			using SendDataT = typename send_fun_traits_type::template args<0>::type;
			using RecvDataT = typename recv_fun_traits_type::template args<0>::type;

			static_assert(std::is_same_v<SendIdT, RecvIdT>);

			return this->derived().template _do_connect<true>(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				condition_wrap<use_rdc_t<void, SendIdT, SendDataT, RecvDataT>>(
					std::in_place,
					std::forward<SendParserFun>(send_parser),
					std::forward<RecvParserFun>(recv_parser)));
		}

		/**
		 * @function : get the websocket upgraged response object
		 */
		inline const http::response_t<body_type>& upgrade_response() { return this->upgrade_rep_; }

		/**
		 * @function : get the websocket upgraged target
		 */
		inline const std::string& upgrade_target() { return this->upgrade_target_; }

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
		template<typename MatchCondition>
		inline void _do_init(condition_wrap<MatchCondition> condition)
		{
			super::_do_init(condition);

			this->derived()._ws_init(condition, this->ssl_stream());
		}

		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			this->derived()._ws_stop(this_ptr, [this, ec, this_ptr]()
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

			this->derived()._ssl_start(this_ptr, condition, this->socket_, *this);

			this->derived()._ws_start(this_ptr, condition, this->ssl_stream());

			this->derived()._post_handshake(std::move(this_ptr), std::move(condition));
		}

		template<typename MatchCondition>
		inline void _handle_handshake(const error_code & ec, std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition)
		{
			set_last_error(ec);

			this->derived()._fire_handshake(this_ptr, ec);

			if (ec)
				return this->derived()._done_connect(ec, std::move(this_ptr), std::move(condition));

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

		inline void _fire_upgrade(std::shared_ptr<derived_t>& this_ptr, error_code ec)
		{
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
	class wss_client : public detail::wss_client_impl_t<wss_client, detail::template_args_wss_client>
	{
	public:
		using wss_client_impl_t<wss_client, detail::template_args_wss_client>::wss_client_impl_t;
	};
}

#endif // !__ASIO2_WSS_CLIENT_HPP__

#endif
