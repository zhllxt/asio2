/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_HTTP_CLIENT_HPP__
#define __ASIO2_HTTP_CLIENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <fstream>

#include <asio2/base/detail/push_options.hpp>

#include <asio2/tcp/tcp_client.hpp>

#include <asio2/http/http_execute.hpp>
#include <asio2/http/http_download.hpp>

#include <asio2/http/impl/http_send_op.hpp>
#include <asio2/http/impl/http_recv_op.hpp>

namespace asio2::detail
{
	struct template_args_http_client : public template_args_tcp_client
	{
		using body_t      = http::string_body;
		using buffer_t    = beast::flat_buffer;
		using send_data_t = http::web_request&;
		using recv_data_t = http::web_response&;
	};

	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;

	template<class derived_t, class args_t = template_args_http_client>
	class http_client_impl_t
		: public tcp_client_impl_t <derived_t, args_t>
		, public http_send_op      <derived_t, args_t>
		, public http_recv_op      <derived_t, args_t>
		, public http_execute_impl <derived_t, args_t>
		, public http_download_impl<derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;

	public:
		using super = tcp_client_impl_t <derived_t, args_t>;
		using self  = http_client_impl_t<derived_t, args_t>;

		using args_type   = args_t;
		using body_type   = typename args_t::body_t;
		using buffer_type = typename args_t::buffer_t;
		using send_data_t = typename args_t::send_data_t;
		using recv_data_t = typename args_t::recv_data_t;

		using super::send;
		using super::async_send;

	public:
		/**
		 * @brief constructor
		 */
		template<class... Args>
		explicit http_client_impl_t(Args&&... args)
			: super(std::forward<Args>(args)...)
			, http_send_op<derived_t, args_t>()
			, req_()
			, rep_()
		{
		}

		/**
		 * @brief destructor
		 */
		~http_client_impl_t()
		{
			this->stop();
		}

		/**
		 * @brief start the client, blocking connect to server
		 * @param host - A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port - A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt, typename... Args>
		inline bool start(String&& host, StrOrInt&& port, Args&&... args)
		{
			if constexpr (ecs_helper::args_has_rdc<Args...>())
			{
				return this->derived().template _do_connect<false>(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					ecs_helper::make_ecs('0', std::forward<Args>(args)...));
			}
			else
			{
				asio2::rdc::option rdc_option
				{
					[](http::web_request &) { return 0; },
					[](http::web_response&) { return 0; }
				};

				return this->derived().template _do_connect<false>(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					ecs_helper::make_ecs('0', std::forward<Args>(args)..., std::move(rdc_option)));
			}
		}

		/**
		 * @brief start the client, asynchronous connect to server
		 * @param host - A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port - A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt, typename... Args>
		inline bool async_start(String&& host, StrOrInt&& port, Args&&... args)
		{
			if constexpr (ecs_helper::args_has_rdc<Args...>())
			{
				return this->derived().template _do_connect<true>(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					ecs_helper::make_ecs('0', std::forward<Args>(args)...));
			}
			else
			{
				asio2::rdc::option rdc_option
				{
					[](http::web_request &) { return 0; },
					[](http::web_response&) { return 0; }
				};

				return this->derived().template _do_connect<true>(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					ecs_helper::make_ecs('0', std::forward<Args>(args)..., std::move(rdc_option)));
			}
		}

	public:
		/**
		 * @brief get the request object, same as get_request
		 */
		inline       http::web_request & request()      noexcept { return this->req_; }

		/**
		 * @brief get the request object, same as get_request
		 */
		inline const http::web_request & request() const noexcept { return this->req_; }

		/**
		 * @brief get the response object, same as get_response
		 */
		inline      http::web_response& response()      noexcept { return this->rep_; }

		/**
		 * @brief get the response object, same as get_response
		 */
		inline const http::web_response& response() const noexcept { return this->rep_; }

		/**
		 * @brief get the request object
		 */
		inline       http::web_request & get_request()      noexcept { return this->req_; }

		/**
		 * @brief get the request object
		 */
		inline const http::web_request & get_request() const noexcept { return this->req_; }

		/**
		 * @brief get the response object
		 */
		inline       http::web_response& get_response()      noexcept { return this->rep_; }

		/**
		 * @brief get the response object
		 */
		inline const http::web_response& get_response() const noexcept { return this->rep_; }

	public:
		/**
		 * @brief bind recv listener
		 * @param fun - a user defined callback function
		 * Function signature : void(http::web_request& req, http::web_response& rep)
		 */
		template<class F, class ...C>
		inline derived_t & bind_recv(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::recv,
				observer_t<http::web_request&, http::web_response&>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	protected:
		template<class Data, class Callback>
		inline bool _do_send(Data& data, Callback&& callback)
		{
			if constexpr (
				detail::is_template_instance_of_v<http::request, detail::remove_cvref_t<Data>> ||
				detail::is_template_instance_of_v<detail::http_request_impl_t, detail::remove_cvref_t<Data>>)
			{
				this->req_ = std::move(data);

				return this->derived()._http_send(this->req_, std::forward<Callback>(callback));
			}
			else
			{
				return this->derived()._http_send(data, std::forward<Callback>(callback));
			}
		}

		template<class Data>
		inline send_data_t _rdc_convert_to_send_data(Data& data)
		{
			return data;
		}

		template<class Invoker>
		inline void _rdc_invoke_with_none(const error_code& ec, Invoker& invoker)
		{
			if (invoker)
				invoker(ec, this->req_, this->rep_);
		}

		template<class Invoker>
		inline void _rdc_invoke_with_recv(const error_code& ec, Invoker& invoker, recv_data_t data)
		{
			detail::ignore_unused(data);
			if (invoker)
				invoker(ec, this->req_, this->rep_);
		}

		template<class Invoker>
		inline void _rdc_invoke_with_send(const error_code& ec, Invoker& invoker, send_data_t data)
		{
			if (invoker)
				invoker(ec, data, this->rep_);
		}

	protected:
		template<typename C>
		inline void _post_recv(std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			this->derived()._http_post_recv(std::move(this_ptr), std::move(ecs));
		}

		template<typename C>
		inline void _handle_recv(
			const error_code & ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			this->derived()._http_handle_recv(ec, bytes_recvd, std::move(this_ptr), std::move(ecs));
		}

		template<typename C>
		inline void _fire_recv(std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs)
		{
			this->listener_.notify(event_type::recv, this->req_, this->rep_);

			this->derived()._rdc_handle_recv(this_ptr, ecs, this->rep_);
		}

	protected:
		http::web_request             req_;

		http::web_response            rep_;
	};
}

namespace asio2
{
	using http_client_args = detail::template_args_http_client;

	template<class derived_t, class args_t>
	using http_client_impl_t = detail::http_client_impl_t<derived_t, args_t>;

	template<class derived_t>
	class http_client_t : public detail::http_client_impl_t<derived_t, detail::template_args_http_client>
	{
	public:
		using detail::http_client_impl_t<derived_t, detail::template_args_http_client>::http_client_impl_t;
	};

	class http_client : public http_client_t<http_client>
	{
	public:
		using http_client_t<http_client>::http_client_t;
	};
}

#if defined(ASIO2_INCLUDE_RATE_LIMIT)
#include <asio2/tcp/tcp_stream.hpp>
namespace asio2
{
	struct http_rate_client_args : public http_client_args
	{
		using socket_t = asio2::tcp_stream<asio2::simple_rate_policy>;
	};

	template<class derived_t>
	class http_rate_client_t : public asio2::http_client_impl_t<derived_t, http_rate_client_args>
	{
	public:
		using asio2::http_client_impl_t<derived_t, http_rate_client_args>::http_client_impl_t;
	};

	class http_rate_client : public asio2::http_rate_client_t<http_rate_client>
	{
	public:
		using asio2::http_rate_client_t<http_rate_client>::http_rate_client_t;
	};
}
#endif

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_HTTP_CLIENT_HPP__
