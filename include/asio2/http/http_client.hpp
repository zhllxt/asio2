/*
 * COPYRIGHT (C) 2017-2021, zhllxt
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

#include <asio2/base/detail/push_options.hpp>

#include <asio2/tcp/tcp_client.hpp>

#include <asio2/http/detail/http_util.hpp>
#include <asio2/http/detail/http_make.hpp>

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
		: public tcp_client_impl_t<derived_t, args_t>
		, public http_send_op     <derived_t, args_t>
		, public http_recv_op     <derived_t, args_t>
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
		 * @constructor
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
		template<typename String, typename StrOrInt, typename... Args>
		inline bool start(String&& host, StrOrInt&& port, Args&&... args)
		{
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
			return this->derived().template _do_connect<true>(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				condition_helper::make_condition('0', std::forward<Args>(args)...));
		}

	public:
		/**
		 * @function : get the request object, same as get_request
		 */
		inline const http::web_request & request() noexcept { return this->req_; }

		/**
		 * @function : get the response object, same as get_response
		 */
		inline const http::web_response& response() noexcept { return this->rep_; }

		/**
		 * @function : get the request object
		 */
		inline const http::web_request & get_request() noexcept { return this->req_; }

		/**
		 * @function : get the response object
		 */
		inline const http::web_response& get_response() noexcept { return this->rep_; }

	public:
		template<typename String, typename StrOrInt, class Rep, class Period, class Proxy,
			class Body = http::string_body, class Fields = http::fields, class Buffer = beast::flat_buffer>
		typename std::enable_if_t<!std::is_same_v<detail::remove_cvref_t<Proxy>, error_code>,
			http::response<Body, Fields>>
		static inline execute(String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req, std::chrono::duration<Rep, Period> timeout, Proxy&& proxy)
		{
			http::parser<false, Body, typename Fields::allocator_type> parser;
			try
			{
				// First assign default value timed_out to last error
				set_last_error(asio::error::timed_out);

				// set default result to unknown
				parser.get().result(http::status::unknown);
				parser.eager(true);

				// The io_context is required for all I/O
				asio::io_context ioc;

				// These objects perform our I/O
				asio::ip::tcp::resolver resolver{ ioc };
				asio::ip::tcp::socket socket{ ioc };

				// This buffer is used for reading and must be persisted
				Buffer buffer;

				// if has socks5 proxy
				if constexpr (std::is_base_of_v<asio2::socks5::detail::option_base,
					typename detail::element_type_adapter<detail::remove_cvref_t<Proxy>>::type>)
				{
					auto socks5 = detail::to_shared_ptr(std::forward<Proxy>(proxy));

					std::string_view h{ socks5->host() };
					std::string_view p{ socks5->port() };

					// Look up the domain name
					resolver.async_resolve(h, p, [&, s5 = std::move(socks5)]
					(const error_code& ec1, const asio::ip::tcp::resolver::results_type& endpoints) mutable
					{
						if (ec1) { set_last_error(ec1); return; }

						// Make the connection on the IP address we get from a lookup
						asio::async_connect(socket, endpoints,
						[&, s5 = std::move(s5)](const error_code& ec2, const asio::ip::tcp::endpoint&) mutable
						{
							if (ec2) { set_last_error(ec2); return; }

							detail::socks5_client_connect_op
							{
								ioc,
								to_string(std::forward<String>(host)), to_string(std::forward<StrOrInt>(port)),
								socket,
								std::move(s5),
								[&](error_code ecs5) mutable
								{
									if (ecs5) { set_last_error(ecs5); return; }

									http::async_write(socket, req, [&](const error_code & ec3, std::size_t) mutable
									{
										if (ec3) { set_last_error(ec3); return; }

										// Then start asynchronous reading
										http::async_read(socket, buffer, parser,
										[&](const error_code& ec4, std::size_t) mutable
										{
											// Reading completed, assign the read the result to last error
											// If the code does not execute into here, the last error
											// is the default value timed_out.
											set_last_error(ec4);
										});
									});
								}
							};
						});
					});
				}
				else
				{
					// Look up the domain name
					resolver.async_resolve(std::forward<String>(host), to_string(std::forward<StrOrInt>(port)),
					[&](const error_code& ec1, const asio::ip::tcp::resolver::results_type& endpoints) mutable
					{
						if (ec1) { set_last_error(ec1); return; }

						// Make the connection on the IP address we get from a lookup
						asio::async_connect(socket, endpoints,
						[&](const error_code& ec2, const asio::ip::tcp::endpoint&) mutable
						{
							if (ec2) { set_last_error(ec2); return; }

							http::async_write(socket, req, [&](const error_code & ec3, std::size_t) mutable
							{
								if (ec3) { set_last_error(ec3); return; }

								// Then start asynchronous reading
								http::async_read(socket, buffer, parser,
								[&](const error_code& ec4, std::size_t) mutable
								{
									// Reading completed, assign the read the result to last error
									// If the code does not execute into here, the last error
									// is the default value timed_out.
									set_last_error(ec4);
								});
							});
						});
					});
				}

				// timedout run
				ioc.run_for(timeout);

				error_code ec_ignore{};

				// Gracefully close the socket
				socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec_ignore);
				socket.close(ec_ignore);
			}
			catch (system_error & e)
			{
				set_last_error(e);
			}

			return parser.release();
		}

		// ----------------------------------------------------------------------------------------

		template<typename String, typename StrOrInt, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req, std::chrono::duration<Rep, Period> timeout)
		{
			return derived_t::execute(
				std::forward<String>(host), std::forward<StrOrInt>(port), req, timeout, std::in_place);
		}

		template<typename String, typename StrOrInt, class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req)
		{
			return derived_t::execute(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				req, std::chrono::milliseconds(http_execute_timeout));
		}

		// ----------------------------------------------------------------------------------------

		template<class Rep, class Period, class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(
			http::web_request& req, std::chrono::duration<Rep, Period> timeout)
		{
			return derived_t::execute(req.url().host(), req.url().port(), req.base(), timeout, std::in_place);
		}

		template<class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(http::web_request& req)
		{
			return derived_t::execute(req, std::chrono::milliseconds(http_execute_timeout));
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<class Rep, class Period, class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(std::string_view url,
			std::chrono::duration<Rep, Period> timeout)
		{
			http::web_request req = http::make_request(url);
			if (get_last_error())
			{
				return http::response<Body, Fields>{ http::status::unknown, 11};
			}
			return derived_t::execute(
				req.host(), req.port(), req.base(), timeout, std::in_place);
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(std::string_view url)
		{
			return derived_t::execute(url, std::chrono::milliseconds(http_execute_timeout));
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<typename String, typename StrOrInt, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(String&& host, StrOrInt&& port,
			std::string_view target, std::chrono::duration<Rep, Period> timeout)
		{
			http::web_request req = http::make_request(host, port, target);
			if (get_last_error())
			{
				return http::response<Body, Fields>{ http::status::unknown, 11};
			}
			return derived_t::execute(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				req.base(), timeout, std::in_place);
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<typename String, typename StrOrInt, class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(String&& host, StrOrInt&& port,
			std::string_view target)
		{
			return derived_t::execute(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				target, std::chrono::milliseconds(http_execute_timeout));
		}

		// ----------------------------------------------------------------------------------------

		template<typename String, typename StrOrInt, class Proxy,
			class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req, Proxy&& proxy)
		{
			return derived_t::execute(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				req, std::chrono::milliseconds(http_execute_timeout), std::forward<Proxy>(proxy));
		}

		// ----------------------------------------------------------------------------------------

		template<class Rep, class Period, class Proxy, class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(
			http::web_request& req, std::chrono::duration<Rep, Period> timeout, Proxy&& proxy)
		{
			return derived_t::execute(req.url().host(), req.url().port(), req.base(), timeout,
				std::forward<Proxy>(proxy));
		}

		template<class Proxy, class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(http::web_request& req, Proxy&& proxy)
		{
			return derived_t::execute(req, std::chrono::milliseconds(http_execute_timeout), std::forward<Proxy>(proxy));
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<class Rep, class Period, class Proxy, class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(std::string_view url,
			std::chrono::duration<Rep, Period> timeout, Proxy&& proxy)
		{
			http::web_request req = http::make_request(url);
			if (get_last_error())
			{
				return http::response<Body, Fields>{ http::status::unknown, 11};
			}
			return derived_t::execute(req.host(), req.port(), req.base(), timeout, std::forward<Proxy>(proxy));
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<class Proxy, class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(std::string_view url, Proxy&& proxy)
		{
			return derived_t::execute(url, std::chrono::milliseconds(http_execute_timeout), std::forward<Proxy>(proxy));
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<typename String, typename StrOrInt, class Rep, class Period, class Proxy,
			class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(String&& host, StrOrInt&& port,
			std::string_view target, std::chrono::duration<Rep, Period> timeout, Proxy&& proxy)
		{
			http::web_request req = http::make_request(host, port, target);
			if (get_last_error())
			{
				return http::response<Body, Fields>{ http::status::unknown, 11};
			}
			return derived_t::execute(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				req.base(), timeout, std::forward<Proxy>(proxy));
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<typename String, typename StrOrInt, class Proxy,
			class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(String&& host, StrOrInt&& port,
			std::string_view target, Proxy&& proxy)
		{
			return derived_t::execute(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				target, std::chrono::milliseconds(http_execute_timeout), std::forward<Proxy>(proxy));
		}

	public:
		/**
		 * @function : bind recv listener
		 * @param    : fun - a user defined callback function
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

		template<class Invoker, class FnData>
		inline void _rdc_invoke_with_send(const error_code& ec, Invoker& invoker, FnData& fn_data)
		{
			if (invoker)
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
		http::web_request             req_;

		http::web_response            rep_;
	};
}

namespace asio2
{
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

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_HTTP_CLIENT_HPP__
