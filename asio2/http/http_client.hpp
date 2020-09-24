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
	template<class derived_t, class socket_t, class body_t, class buffer_t>
	class http_client_impl_t
		: public tcp_client_impl_t<derived_t, socket_t, buffer_t>
		, public http_send_cp<derived_t, body_t, buffer_t, false>
		, public http_send_op<derived_t, body_t, buffer_t, false>
		, public http_recv_op<derived_t, body_t, buffer_t, false>
	{
		template <class, bool>                friend class user_timer_cp;
		template <class>                      friend class post_cp;
		template <class, bool>                friend class reconnect_timer_cp;
		template <class, bool>                friend class connect_timeout_cp;
		template <class, class, bool>         friend class connect_cp;
		template <class, class, bool>         friend class disconnect_cp;
		template <class>                      friend class data_persistence_cp;
		template <class>                      friend class event_queue_cp;
		template <class, bool>                friend class send_cp;
		template <class, bool>                friend class tcp_send_op;
		template <class, bool>                friend class tcp_recv_op;
		template <class, class, class, bool>  friend class http_send_cp;
		template <class, class, class, bool>  friend class http_send_op;
		template <class, class, class, bool>  friend class http_recv_op;
		template <class, class, class>        friend class client_impl_t;
		template <class, class, class>        friend class tcp_client_impl_t;

	public:
		using self = http_client_impl_t<derived_t, socket_t, body_t, buffer_t>;
		using super = tcp_client_impl_t<derived_t, socket_t, buffer_t>;
		using body_type = body_t;
		using buffer_type = buffer_t;
		using super::send;
		using http_send_cp<derived_t, body_t, buffer_t, false>::send;
		using data_persistence_cp<derived_t>::_data_persistence;

	public:
		/**
		 * @constructor
		 */
		explicit http_client_impl_t(
			std::size_t init_buffer_size = tcp_frame_size,
			std::size_t max_buffer_size = (std::numeric_limits<std::size_t>::max)()
		)
			: super(init_buffer_size, max_buffer_size)
			, http_send_cp<derived_t, body_t, buffer_t, false>(this->io_)
			, http_send_op<derived_t, body_t, buffer_t, false>()
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
		template<class Rep, class Period, class Body = http::string_body, class Fields = http::fields,
			class Buffer = beast::flat_buffer>
		static inline http::response_t<Body, Fields> execute(std::string_view host, std::string_view port,
			http::request_t<Body, Fields>& req, std::chrono::duration<Rep, Period> timeout, error_code& ec)
		{
			http::response_t<Body, Fields> rep;
			try
			{
				// set default result to unknown
				rep.result(http::status::unknown);

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
				resolver.async_resolve(host, port, [&]
				(const error_code& ec1, const asio::ip::tcp::resolver::results_type& endpoints)
				{
					if (ec1) { ec = ec1; return; }

					// Make the connection on the IP address we get from a lookup
					asio::async_connect(socket, endpoints,
						[&](const error_code& ec2, const asio::ip::tcp::endpoint&)
					{
						if (ec2) { ec = ec2; return; }

						http::async_write(socket, req, [&](const error_code & ec3, std::size_t)
						{
							if (ec3) { ec = ec3; return; }

							// Then start asynchronous reading
							http::async_read(socket, buffer, rep,
								[&](const error_code& ec4, std::size_t)
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
			return rep;
		}

		template<class Body = http::string_body, class Fields = http::fields, class Buffer = beast::flat_buffer>
		static inline http::response_t<Body, Fields> execute(std::string_view host, std::string_view port,
			http::request_t<Body, Fields>& req, error_code& ec)
		{
			using Rep = std::chrono::milliseconds::rep;
			using Period = std::chrono::milliseconds::period;
			ec.clear();
			return execute<Rep, Period, Body, Fields, Buffer>(host, port, req,
				std::chrono::milliseconds(http_execute_timeout), ec);
		}

		template<class Body = http::string_body, class Fields = http::fields, class Buffer = beast::flat_buffer>
		static inline http::response_t<Body, Fields> execute(std::string_view host, std::string_view port,
			http::request_t<Body, Fields>& req)
		{
			error_code ec;
			http::response_t<body_t> rep = execute(host, port, req, ec);
			asio::detail::throw_error(ec);
			return rep;
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 * You need to encode the "url"(by url_encode) before calling this function
		 */
		static inline http::response_t<body_t> execute(std::string_view url, error_code& ec)
		{
			using Rep = std::chrono::milliseconds::rep;
			using Period = std::chrono::milliseconds::period;
			ec.clear();
			http::request_t<body_t> req = http::make_request<body_t>(url, ec);
			if (ec) return http::response_t<body_t>{ http::status::unknown, 11};
			std::string_view host = http::url_to_host(url);
			std::string_view port = http::url_to_port(url);
			return execute<Rep, Period, body_t>(host, port, req,
				std::chrono::milliseconds(http_execute_timeout), ec);
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 * You need to encode the "url"(by url_encode) before calling this function
		 */
		static inline http::response_t<body_t> execute(std::string_view url)
		{
			error_code ec;
			http::response_t<body_t> rep = execute(url, ec);
			asio::detail::throw_error(ec);
			return rep;
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 * You need to encode the "target"(by url_encode) before calling this function
		 */
		static inline http::response_t<body_t> execute(std::string_view host, std::string_view port,
			std::string_view target, error_code& ec)
		{
			using Rep = std::chrono::milliseconds::rep;
			using Period = std::chrono::milliseconds::period;
			ec.clear();
			http::request_t<body_t> req = http::make_request<body_t>(host, port, target);
			return execute<Rep, Period, body_t>(host, port, req,
				std::chrono::milliseconds(http_execute_timeout), ec);
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 * You need to encode the "target"(by url_encode) before calling this function
		 */
		static inline http::response_t<body_t> execute(std::string_view host, std::string_view port,
			std::string_view target)
		{
			error_code ec;
			http::response_t<body_t> rep = execute(host, port, target, ec);
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
			this->listener_.bind(event::recv,
				observer_t<http::request&, http::response&>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	protected:
		template<bool isRequest, class Body, class Fields = http::fields>
		inline auto _data_persistence(http::message<isRequest, Body, Fields>& msg)
		{
			return this->derived()._data_persistence(
				const_cast<const http::message<isRequest, Body, Fields>&>(msg));
		}

		template<bool isRequest, class Body, class Fields = http::fields>
		inline auto _data_persistence(const http::message<isRequest, Body, Fields>& msg)
		{
			return copyable_wrapper(std::move(msg));
		}

		template<bool isRequest, class Body, class Fields = http::fields>
		inline auto _data_persistence(http::message<isRequest, Body, Fields>&& msg)
		{
			// why use copyable_wrapper? beacuse http::message<isRequest, http::file_body>
			// is moveable-only, but std::function is copyable-only
			return copyable_wrapper(std::move(msg));
		}

		template<class Data, class Callback>
		inline bool _do_send(Data& data, Callback&& callback)
		{
			return this->derived()._http_send(data, std::forward<Callback>(callback));
		}

	protected:
		template<typename MatchCondition>
		inline void _post_recv(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived()._http_post_recv(std::move(this_ptr), condition);
		}

		template<typename MatchCondition>
		inline void _handle_recv(const error_code & ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived()._http_handle_recv(ec, bytes_recvd, std::move(this_ptr), condition);
		}

		template<typename MatchCondition>
		inline void _fire_recv(std::shared_ptr<derived_t>& this_ptr, condition_wrap<MatchCondition>& condition)
		{
			detail::ignore::unused(this_ptr, condition);

			this->listener_.notify(event::recv, this->req_, this->rep_);
		}

	protected:
		http::request             req_;

		http::response            rep_;
	};
}

namespace asio2
{
	class http_client : public detail::http_client_impl_t<http_client,
		asio::ip::tcp::socket, http::string_body, beast::flat_buffer>
	{
	public:
		using http_client_impl_t<http_client, asio::ip::tcp::socket,
			http::string_body, beast::flat_buffer>::http_client_impl_t;
	};
}

#endif // !__ASIO2_HTTP_CLIENT_HPP__
