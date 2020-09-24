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

#ifndef __ASIO2_HTTPS_CLIENT_HPP__
#define __ASIO2_HTTPS_CLIENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/http/http_client.hpp>
#include <asio2/tcp/component/ssl_stream_cp.hpp>
#include <asio2/tcp/component/ssl_context_cp.hpp>

namespace asio2::detail
{
	template<class derived_t, class socket_t, class body_t, class buffer_t>
	class https_client_impl_t
		: public ssl_context_cp<derived_t, false>
		, public http_client_impl_t<derived_t, socket_t, body_t, buffer_t>
		, public ssl_stream_cp<derived_t, socket_t, false>
	{
		template <class, bool>                       friend class user_timer_cp;
		template <class>                             friend class post_cp;
		template <class, bool>                       friend class reconnect_timer_cp;
		template <class, bool>                       friend class connect_timeout_cp;
		template <class, class, bool>                friend class connect_cp;
		template <class, class, bool>                friend class disconnect_cp;
		template <class>                             friend class data_persistence_cp;
		template <class>                             friend class event_queue_cp;
		template <class, bool>                       friend class send_cp;
		template <class, bool>                       friend class tcp_send_op;
		template <class, bool>                       friend class tcp_recv_op;
		template <class, class, class, bool>         friend class http_send_cp;
		template <class, class, class, bool>         friend class http_send_op;
		template <class, class, class, bool>         friend class http_recv_op;
		template <class, bool>						 friend class ssl_context_cp;
		template <class, class, bool>                friend class ssl_stream_cp;
		template <class, class, class>               friend class client_impl_t;
		template <class, class, class>               friend class tcp_client_impl_t;
		template <class, class, class, class>        friend class http_client_impl_t;

	public:
		using self = https_client_impl_t<derived_t, socket_t, body_t, buffer_t>;
		using super = http_client_impl_t<derived_t, socket_t, body_t, buffer_t>;
		using body_type = body_t;
		using buffer_type = buffer_t;
		using ssl_context_comp = ssl_context_cp<derived_t, false>;
		using ssl_stream_comp = ssl_stream_cp<derived_t, socket_t, false>;
		using super::send;
		using super::_data_persistence;

	public:
		/**
		 * @constructor
		 */
		explicit https_client_impl_t(
			asio::ssl::context::method method = asio::ssl::context::sslv23,
			std::size_t init_buffer_size = tcp_frame_size,
			std::size_t max_buffer_size = (std::numeric_limits<std::size_t>::max)()
		)
			: ssl_context_comp(method)
			, super(init_buffer_size, max_buffer_size)
			, ssl_stream_comp(this->io_, *this, asio::ssl::stream_base::client)
		{
		}

		/**
		 * @destructor
		 */
		~https_client_impl_t()
		{
			this->stop();
		}

		/**
		 * @function : get the stream object refrence
		 */
		inline typename ssl_stream_comp::stream_type & stream()
		{
			ASIO2_ASSERT(bool(this->ssl_stream_));
			return (*(this->ssl_stream_));
		}

	public:
		template<class Rep, class Period, class Body = http::string_body, class Fields = http::fields,
			class Buffer = beast::flat_buffer>
		static inline http::response_t<Body, Fields> execute(asio::ssl::context ctx, std::string_view host,
			std::string_view port, http::request_t<Body, Fields>& req, std::chrono::duration<Rep, Period> timeout,
			error_code& ec)
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
				asio::ssl::stream<asio::ip::tcp::socket&> stream(socket, ctx);

				// This buffer is used for reading and must be persisted
				Buffer buffer;

				// Look up the domain name
				resolver.async_resolve(host, port, [&](const error_code& ec1,
					const asio::ip::tcp::resolver::results_type& endpoints)
				{
					if (ec1) { ec = ec1; return; }

					// Make the connection on the IP address we get from a lookup
					asio::async_connect(socket, endpoints, [&](const error_code & ec2,
						const asio::ip::tcp::endpoint&)
					{
						if (ec2) { ec = ec2; return; }

						stream.async_handshake(asio::ssl::stream_base::client,
							[&](const error_code& ec3)
						{
							if (ec3) { ec = ec3; return; }

							http::async_write(stream, req,
								[&](const error_code& ec4, std::size_t)
							{
								if (ec4) { ec = ec4; return; }

								// Then start asynchronous reading
								http::async_read(stream, buffer, rep,
									[&](const error_code& ec5, std::size_t)
								{
									// Reading completed, assign the read the result to ec
									// If the code does not execute into here, the ec value
									// is the default value timed_out.
									ec = ec5;
								});
							});
						});
					});
				});

				// timedout run
				ioc.run_for(timeout);

				// close ssl stream
				stream.shutdown(ec_ignore);

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
			return execute<Rep, Period, Body, Fields, Buffer>(asio::ssl::context{ asio::ssl::context::sslv23 },
				host, port, req, std::chrono::milliseconds(http_execute_timeout), ec);
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
			return execute<Rep, Period, body_t>(asio::ssl::context{ asio::ssl::context::sslv23 }, host, port, req,
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
			return execute<Rep, Period, body_t>(asio::ssl::context{ asio::ssl::context::sslv23 }, host, port, req,
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
		 * @function : bind ssl handshake listener
		 * @param    : fun - a user defined callback function
		 * Function signature : void(asio2::error_code ec)
		 */
		template<class F, class ...C>
		inline derived_t & bind_handshake(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::handshake,
				observer_t<error_code>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	protected:
		template<typename MatchCondition>
		inline void _do_init(condition_wrap<MatchCondition> condition)
		{
			super::_do_init(condition);

			this->derived()._ssl_init(condition, this->socket_, *this);
		}

		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			this->derived()._ssl_stop(this_ptr, [this, ec, this_ptr]()
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

			this->derived()._post_handshake(std::move(this_ptr), std::move(condition));
		}

		inline void _fire_handshake(detail::ignore, error_code ec)
		{
			this->listener_.notify(event::handshake, ec);
		}

	protected:
	};
}

namespace asio2
{
	class https_client : public detail::https_client_impl_t<https_client,
		asio::ip::tcp::socket, http::string_body, beast::flat_buffer>
	{
	public:
		using https_client_impl_t<https_client, asio::ip::tcp::socket,
			http::string_body, beast::flat_buffer>::https_client_impl_t;
	};
}

#endif // !__ASIO2_HTTPS_CLIENT_HPP__

#endif
