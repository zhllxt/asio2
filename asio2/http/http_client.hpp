/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_HTTP_CLIENT_HPP__
#define __ASIO2_HTTP_CLIENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/tcp/tcp_client.hpp>
#include <asio2/http/detail/http_util.hpp>
#include <asio2/http/component/http_send_cp.hpp>
#include <asio2/http/impl/http_send_op.hpp>
#include <asio2/http/impl/http_recv_op.hpp>

namespace asio2::detail
{
	struct template_args_http_client : public template_args_tcp_client
	{
		using body_t      = http::string_body;
		using buffer_t    = beast::flat_buffer;
		using send_data_t = http::request&;
		using recv_data_t = http::response&;
	};

	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;

	template<class derived_t, class args_t>
	class http_client_impl_t
		: public tcp_client_impl_t<derived_t, args_t>
		, public http_send_cp     <derived_t, args_t>
		, public http_send_op     <derived_t, args_t>
		, public http_recv_op     <derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;

	public:
		using super = tcp_client_impl_t <derived_t, args_t>;
		using self  = http_client_impl_t<derived_t, args_t>;

		using body_type   = typename args_t::body_t;
		using buffer_type = typename args_t::buffer_t;
		using send_data_t = typename args_t::send_data_t;
		using recv_data_t = typename args_t::recv_data_t;

		using super::send;
		using http_send_cp       <derived_t, args_t>::send;
		using data_persistence_cp<derived_t, args_t>::_data_persistence;

	public:
		/**
		 * @constructor
		 */
		explicit http_client_impl_t(
			std::size_t init_buffer_size = tcp_frame_size,
			std::size_t max_buffer_size  = (std::numeric_limits<std::size_t>::max)()
		)
			: super(init_buffer_size, max_buffer_size)
			, http_send_cp<derived_t, args_t>(this->io_)
			, http_send_op<derived_t, args_t>()
			, req_()
			, rep_()
		{
		}

		/**
		 * @destructor
		 */
		~http_client_impl_t()
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

	public:
		/**
		 * @function : get the request object
		 */
		inline const http::request & request()  { return this->req_; }

		/**
		 * @function : get the response object
		 */
		inline const http::response& response() { return this->rep_; }

	public:
		template<typename String, typename StrOrInt, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields, class Buffer = beast::flat_buffer>
		static inline http::response_t<Body, Fields> execute(String&& host, StrOrInt&& port,
			http::request_t<Body, Fields>& req, std::chrono::duration<Rep, Period> timeout, error_code& ec)
		{
			http::parser<false, Body, typename Fields::allocator_type> parser;
			try
			{
				// set default result to unknown
				parser.get().result(http::status::unknown);
				parser.eager(true);

				// First assign default value timed_out to ec
				ec = asio::error::timed_out;

				// The io_context is required for all I/O
				asio::io_context ioc;

				// These objects perform our I/O
				asio::ip::tcp::resolver resolver{ ioc };
				asio::ip::tcp::socket socket{ ioc };

				// This buffer is used for reading and must be persisted
				Buffer buffer;

				// Look up the domain name
				resolver.async_resolve(std::forward<String>(host), to_string(std::forward<StrOrInt>(port)),
					[&](const error_code& ec1, const asio::ip::tcp::resolver::results_type& endpoints) mutable
				{
					if (ec1) { ec = ec1; return; }

					// Make the connection on the IP address we get from a lookup
					asio::async_connect(socket, endpoints,
						[&](const error_code& ec2, const asio::ip::tcp::endpoint&) mutable
					{
						if (ec2) { ec = ec2; return; }

						http::async_write(socket, req, [&](const error_code & ec3, std::size_t) mutable
						{
							if (ec3) { ec = ec3; return; }

							// Then start asynchronous reading
							http::async_read(socket, buffer, parser,
								[&](const error_code& ec4, std::size_t) mutable
							{
								// Reading completed, assign the read the result to ec
								// If the code does not execute into here, the ec value
								// is the default value timed_out.
								ec = ec4;
							});
						});
					});
				});

				// timedout run
				ioc.run_for(timeout);

				// Gracefully close the socket
				socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec_ignore);
				socket.close(ec_ignore);
			}
			catch (system_error & e)
			{
				ec = e.code();
			}

			return parser.release();
		}

		template<typename String, typename StrOrInt, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields, class Buffer = beast::flat_buffer>
		static inline http::response_t<Body, Fields> execute(String&& host, StrOrInt&& port,
			http::request_t<Body, Fields>& req, std::chrono::duration<Rep, Period> timeout)
		{
			error_code ec;
			http::response_t<Body, Fields> rep = execute(
				std::forward<String>(host), std::forward<StrOrInt>(port), req, timeout, ec);
			asio::detail::throw_error(ec);
			return rep;
		}

		template<typename String, typename StrOrInt, class Body = http::string_body, class Fields = http::fields>
		static inline http::response_t<Body, Fields> execute(String&& host, StrOrInt&& port,
			http::request_t<Body, Fields>& req, error_code& ec)
		{
			ec.clear();
			return execute(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				req, std::chrono::milliseconds(http_execute_timeout), ec);
		}

		template<typename String, typename StrOrInt, class Body = http::string_body, class Fields = http::fields>
		static inline http::response_t<Body, Fields> execute(String&& host, StrOrInt&& port,
			http::request_t<Body, Fields>& req)
		{
			error_code ec;
			http::response_t<Body, Fields> rep = execute(
				std::forward<String>(host), std::forward<StrOrInt>(port), req, ec);
			asio::detail::throw_error(ec);
			return rep;
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 * You need to encode the "url"(by url_encode) before calling this function
		 */
		template<class Body = http::string_body, class Fields = http::fields>
		static inline http::response_t<Body, Fields> execute(std::string_view url, error_code& ec)
		{
			return execute(url, std::chrono::milliseconds(http_execute_timeout), ec);
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 * You need to encode the "url"(by url_encode) before calling this function
		 */
		template<class Body = http::string_body, class Fields = http::fields>
		static inline http::response_t<Body, Fields> execute(std::string_view url)
		{
			error_code ec;
			http::response_t<Body, Fields> rep = execute(url, ec);
			asio::detail::throw_error(ec);
			return rep;
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 * You need to encode the "url"(by url_encode) before calling this function
		 */
		template<class Rep, class Period, class Body = http::string_body, class Fields = http::fields>
		static inline http::response_t<Body, Fields> execute(std::string_view url,
			std::chrono::duration<Rep, Period> timeout, error_code& ec)
		{
			ec.clear();
			http::request_t<Body, Fields> req = http::make_request<Body, Fields>(url, ec);
			if (ec) return http::response_t<Body, Fields>{ http::status::unknown, 11};
			std::string_view host = http::url_to_host(url);
			std::string_view port = http::url_to_port(url);
			return execute(
				host, port, req, timeout, ec);
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 * You need to encode the "url"(by url_encode) before calling this function
		 */
		template<class Rep, class Period, class Body = http::string_body, class Fields = http::fields>
		static inline http::response_t<Body, Fields> execute(std::string_view url,
			std::chrono::duration<Rep, Period> timeout)
		{
			error_code ec;
			http::response_t<Body, Fields> rep = execute(url, timeout, ec);
			asio::detail::throw_error(ec);
			return rep;
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 * You need to encode the "target"(by url_encode) before calling this function
		 */
		template<typename String, typename StrOrInt, class Body = http::string_body, class Fields = http::fields>
		static inline http::response_t<Body, Fields> execute(String&& host, StrOrInt&& port,
			std::string_view target, error_code& ec)
		{
			return execute(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				target, std::chrono::milliseconds(http_execute_timeout), ec);
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 * You need to encode the "target"(by url_encode) before calling this function
		 */
		template<typename String, typename StrOrInt, class Body = http::string_body, class Fields = http::fields>
		static inline http::response_t<Body, Fields> execute(String&& host, StrOrInt&& port,
			std::string_view target)
		{
			error_code ec;
			http::response_t<Body, Fields> rep = execute(
				std::forward<String>(host), std::forward<StrOrInt>(port), target, ec);
			asio::detail::throw_error(ec);
			return rep;
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 * You need to encode the "target"(by url_encode) before calling this function
		 */
		template<typename String, typename StrOrInt, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		static inline http::response_t<Body, Fields> execute(String&& host, StrOrInt&& port,
			std::string_view target, std::chrono::duration<Rep, Period> timeout, error_code& ec)
		{
			using host_type = std::remove_cv_t<std::remove_reference_t<String>>;
			using port_type = std::remove_cv_t<std::remove_reference_t<StrOrInt>>;
			ec.clear();
			http::request_t<Body, Fields> req = http::make_request<
				const host_type&, const port_type&, Body, Fields>(
					const_cast<const host_type&>(host), const_cast<const port_type&>(port),
					target);
			return execute(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				req, timeout, ec);
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 * You need to encode the "target"(by url_encode) before calling this function
		 */
		template<typename String, typename StrOrInt, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		static inline http::response_t<Body, Fields> execute(String&& host, StrOrInt&& port,
			std::string_view target, std::chrono::duration<Rep, Period> timeout)
		{
			error_code ec;
			http::response_t<Body, Fields> rep = execute(
				std::forward<String>(host), std::forward<StrOrInt>(port), target, timeout, ec);
			asio::detail::throw_error(ec);
			return rep;
		}

	public:
		/**
		 * @function : bind recv listener
		 * @param    : fun - a user defined callback function
		 * Function signature : void(http::response_t<http::string_body>& rep)
		 */
		template<class F, class ...C>
		inline derived_t & bind_recv(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::recv,
				observer_t<http::request&, http::response&>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	protected:
		template<class Data, class Callback>
		inline bool _do_send(Data& data, Callback&& callback)
		{
			return this->derived()._http_send(data, std::forward<Callback>(callback));
		}

		template<class Data>
		inline send_data_t _rdc_convert_to_send_data(Data& data)
		{
			return data;
		}

		template<class Invoker>
		inline void _rdc_invoke_with_none(const error_code& ec, Invoker& invoker)
		{
			invoker(ec, this->req_, this->rep_);
		}

		template<class Invoker>
		inline void _rdc_invoke_with_recv(const error_code& ec, Invoker& invoker, recv_data_t data)
		{
			invoker(ec, this->req_, this->rep_);
		}

		template<class Invoker, class FnData>
		inline void _rdc_invoke_with_send(const error_code& ec, Invoker& invoker, FnData& fn_data)
		{
			invoker(ec, fn_data(), this->rep_);
		}

	protected:
		template<typename MatchCondition>
		inline void _post_recv(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived()._http_post_recv(std::move(this_ptr), std::move(condition));
		}

		template<typename MatchCondition>
		inline void _handle_recv(const error_code & ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived()._http_handle_recv(ec, bytes_recvd, std::move(this_ptr), std::move(condition));
		}

		template<typename MatchCondition>
		inline void _fire_recv(std::shared_ptr<derived_t>& this_ptr, condition_wrap<MatchCondition>& condition)
		{
			this->listener_.notify(event_type::recv, this->req_, this->rep_);

			this->derived()._rdc_handle_recv(this_ptr, this->rep_, condition);
		}

	protected:
		http::request             req_;

		http::response            rep_;
	};
}

namespace asio2
{
	class http_client : public detail::http_client_impl_t<http_client, detail::template_args_http_client>
	{
	public:
		using http_client_impl_t<http_client, detail::template_args_http_client>::http_client_impl_t;
	};
}

#endif // !__ASIO2_HTTP_CLIENT_HPP__
