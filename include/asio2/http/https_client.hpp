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

#ifndef __ASIO2_HTTPS_CLIENT_HPP__
#define __ASIO2_HTTPS_CLIENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/http/http_client.hpp>
#include <asio2/tcp/component/ssl_stream_cp.hpp>
#include <asio2/tcp/component/ssl_context_cp.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;

	template<class derived_t, class args_t = template_args_http_client>
	class https_client_impl_t
		: public ssl_context_cp    <derived_t, args_t>
		, public http_client_impl_t<derived_t, args_t>
		, public ssl_stream_cp     <derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;

	public:
		using super = http_client_impl_t <derived_t, args_t>;
		using self  = https_client_impl_t<derived_t, args_t>;

		using args_type   = args_t;
		using body_type   = typename args_t::body_t;
		using buffer_type = typename args_t::buffer_t;

		using ssl_context_comp = ssl_context_cp<derived_t, args_t>;
		using ssl_stream_comp  = ssl_stream_cp <derived_t, args_t>;

		using super::send;
		using super::async_send;

	public:
		/**
		 * @constructor
		 */
		template<class... Args>
		explicit https_client_impl_t(
			asio::ssl::context::method method = asio::ssl::context::sslv23,
			Args&&... args
		)
			: ssl_context_comp(method)
			, super(std::forward<Args>(args)...)
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
		inline typename ssl_stream_comp::stream_type & stream() noexcept
		{
			ASIO2_ASSERT(bool(this->ssl_stream_));
			return (*(this->ssl_stream_));
		}

	public:
		template<typename String, typename StrOrInt, class Rep, class Period, class Proxy,
			class Body = http::string_body, class Fields = http::fields, class Buffer = beast::flat_buffer>
		typename std::enable_if_t<!std::is_same_v<detail::remove_cvref_t<Proxy>, error_code>,
			http::response<Body, Fields>>
		static inline execute(const asio::ssl::context& ctx, String&& host, StrOrInt&& port,
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
				asio::ssl::stream<asio::ip::tcp::socket&> stream(socket, const_cast<asio::ssl::context&>(ctx));

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

									stream.async_handshake(asio::ssl::stream_base::client,
									[&](const error_code& ec3) mutable
									{
										if (ec3) { set_last_error(ec3); return; }

										http::async_write(stream, req, [&](const error_code& ec4, std::size_t) mutable
										{
											// can't use stream.shutdown(),in some case the shutdowm will blocking forever.
											if (ec4) { set_last_error(ec4); stream.async_shutdown([](const error_code&) {}); return; }

											// Then start asynchronous reading
											http::async_read(stream, buffer, parser,
											[&](const error_code& ec5, std::size_t) mutable
											{
												// Reading completed, assign the read the result to last error
												// If the code does not execute into here, the last error
												// is the default value timed_out.
												set_last_error(ec5);

												stream.async_shutdown([](const error_code&) mutable {});
											});
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

							stream.async_handshake(asio::ssl::stream_base::client,
							[&](const error_code& ec3) mutable
							{
								if (ec3) { set_last_error(ec3); return; }

								http::async_write(stream, req, [&](const error_code& ec4, std::size_t) mutable
								{
									// can't use stream.shutdown(),in some case the shutdowm will blocking forever.
									if (ec4) { set_last_error(ec4); stream.async_shutdown([](const error_code&) {}); return; }

									// Then start asynchronous reading
									http::async_read(stream, buffer, parser,
									[&](const error_code& ec5, std::size_t) mutable
									{
										// Reading completed, assign the read the result to last error
										// If the code does not execute into here, the last error
										// is the default value timed_out.
										set_last_error(ec5);

										stream.async_shutdown([](const error_code&) mutable {});
									});
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
		static inline http::response<Body, Fields> execute(
			const asio::ssl::context& ctx, String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req, std::chrono::duration<Rep, Period> timeout)
		{
			return derived_t::execute(ctx, std::forward<String>(host), std::forward<StrOrInt>(port),
				req, timeout, std::in_place);
		}

		template<typename String, typename StrOrInt,
			class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(
			const asio::ssl::context& ctx, String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req)
		{
			return derived_t::execute(ctx, std::forward<String>(host), std::forward<StrOrInt>(port),
				req, std::chrono::milliseconds(http_execute_timeout), std::in_place);
		}

		// ----------------------------------------------------------------------------------------

		template<typename String, typename StrOrInt, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req, std::chrono::duration<Rep, Period> timeout)
		{
			return derived_t::execute(asio::ssl::context{ asio::ssl::context::sslv23 },
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
			const asio::ssl::context& ctx, http::web_request& req, std::chrono::duration<Rep, Period> timeout)
		{
			return derived_t::execute(ctx, req.url().host(), req.url().port(), req.base(), timeout, std::in_place);
		}

		template<class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(
			const asio::ssl::context& ctx, http::web_request& req)
		{
			return derived_t::execute(ctx, req, std::chrono::milliseconds(http_execute_timeout));
		}

		template<class Rep, class Period, class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(
			http::web_request& req, std::chrono::duration<Rep, Period> timeout)
		{
			return derived_t::execute(asio::ssl::context{ asio::ssl::context::sslv23 }, req, timeout);
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
		static inline http::response<Body, Fields> execute(const asio::ssl::context& ctx, std::string_view url,
			std::chrono::duration<Rep, Period> timeout)
		{
			http::web_request req = http::make_request(url);
			if (get_last_error())
			{
				return http::response<Body, Fields>{ http::status::unknown, 11};
			}
			return derived_t::execute(ctx, req.host(), req.port(), req.base(), timeout, std::in_place);
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(const asio::ssl::context& ctx, std::string_view url)
		{
			return derived_t::execute(ctx, url, std::chrono::milliseconds(http_execute_timeout));
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<class Rep, class Period, class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(std::string_view url,
			std::chrono::duration<Rep, Period> timeout)
		{
			return derived_t::execute(asio::ssl::context{ asio::ssl::context::sslv23 }, url, timeout);
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
		static inline http::response<Body, Fields> execute(
			const asio::ssl::context& ctx, String&& host, StrOrInt&& port,
			std::string_view target, std::chrono::duration<Rep, Period> timeout)
		{
			http::web_request req = http::make_request(host, port, target);
			if (get_last_error())
			{
				return http::response<Body, Fields>{ http::status::unknown, 11};
			}
			return derived_t::execute(ctx,
				std::forward<String>(host), std::forward<StrOrInt>(port),
				req.base(), timeout, std::in_place);
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<typename String, typename StrOrInt, class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(
			const asio::ssl::context& ctx, String&& host, StrOrInt&& port,
			std::string_view target)
		{
			return derived_t::execute(ctx,
				std::forward<String>(host), std::forward<StrOrInt>(port),
				target, std::chrono::milliseconds(http_execute_timeout));
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<typename String, typename StrOrInt, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		typename std::enable_if_t<detail::can_convert_to_string_v<detail::remove_cvref_t<String>>,
			http::response<Body, Fields>>
		static inline execute(String&& host, StrOrInt&& port, std::string_view target,
			std::chrono::duration<Rep, Period> timeout)
		{
			return derived_t::execute(asio::ssl::context{ asio::ssl::context::sslv23 },
				std::forward<String>(host), std::forward<StrOrInt>(port),
				target, timeout);
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<typename String, typename StrOrInt, class Body = http::string_body, class Fields = http::fields>
		typename std::enable_if_t<detail::can_convert_to_string_v<detail::remove_cvref_t<String>>,
			http::response<Body, Fields>>
		static inline execute(String&& host, StrOrInt&& port, std::string_view target)
		{
			return derived_t::execute(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				target, std::chrono::milliseconds(http_execute_timeout));
		}

		// ----------------------------------------------------------------------------------------

		template<typename String, typename StrOrInt, class Proxy,
			class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(
			const asio::ssl::context& ctx, String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req, Proxy&& proxy)
		{
			return derived_t::execute(ctx, std::forward<String>(host), std::forward<StrOrInt>(port),
				req, std::chrono::milliseconds(http_execute_timeout), std::forward<Proxy>(proxy));
		}

		// ----------------------------------------------------------------------------------------

		template<typename String, typename StrOrInt, class Proxy, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req, std::chrono::duration<Rep, Period> timeout, Proxy&& proxy)
		{
			return derived_t::execute(asio::ssl::context{ asio::ssl::context::sslv23 },
				std::forward<String>(host), std::forward<StrOrInt>(port),
				req, timeout, std::forward<Proxy>(proxy));
		}

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

		template<class Proxy, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(
			const asio::ssl::context& ctx, http::web_request& req,
			std::chrono::duration<Rep, Period> timeout, Proxy&& proxy)
		{
			return derived_t::execute(ctx, req.url().host(), req.url().port(), req.base(), timeout,
				std::forward<Proxy>(proxy));
		}

		template<class Proxy, class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(
			const asio::ssl::context& ctx, http::web_request& req, Proxy&& proxy)
		{
			return derived_t::execute(ctx, req, std::chrono::milliseconds(http_execute_timeout),
				std::forward<Proxy>(proxy));
		}

		template<class Proxy, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(
			http::web_request& req, std::chrono::duration<Rep, Period> timeout, Proxy&& proxy)
		{
			return derived_t::execute(asio::ssl::context{ asio::ssl::context::sslv23 }, req, timeout,
				std::forward<Proxy>(proxy));
		}

		template<class Proxy, class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(http::web_request& req, Proxy&& proxy)
		{
			return derived_t::execute(req, std::chrono::milliseconds(http_execute_timeout),
				std::forward<Proxy>(proxy));
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<class Proxy, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(
			const asio::ssl::context& ctx, std::string_view url,
			std::chrono::duration<Rep, Period> timeout, Proxy&& proxy)
		{
			http::web_request req = http::make_request(url);
			if (get_last_error())
			{
				return http::response<Body, Fields>{ http::status::unknown, 11};
			}
			return derived_t::execute(ctx, req.host(), req.port(), req.base(), timeout, std::forward<Proxy>(proxy));
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<class Proxy, class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(
			const asio::ssl::context& ctx, std::string_view url, Proxy&& proxy)
		{
			return derived_t::execute(ctx, url, std::chrono::milliseconds(http_execute_timeout),
				std::forward<Proxy>(proxy));
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<class Proxy, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(std::string_view url,
			std::chrono::duration<Rep, Period> timeout, Proxy&& proxy)
		{
			return derived_t::execute(asio::ssl::context{ asio::ssl::context::sslv23 }, url, timeout,
				std::forward<Proxy>(proxy));
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<class Proxy, class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(std::string_view url, Proxy&& proxy)
		{
			return derived_t::execute(url, std::chrono::milliseconds(http_execute_timeout),
				std::forward<Proxy>(proxy));
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<typename String, typename StrOrInt, class Proxy, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(
			const asio::ssl::context& ctx, String&& host, StrOrInt&& port,
			std::string_view target, std::chrono::duration<Rep, Period> timeout, Proxy&& proxy)
		{
			http::web_request req = http::make_request(host, port, target);
			if (get_last_error())
			{
				return http::response<Body, Fields>{ http::status::unknown, 11};
			}
			return derived_t::execute(ctx,
				std::forward<String>(host), std::forward<StrOrInt>(port),
				req.base(), timeout, std::forward<Proxy>(proxy));
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<typename String, typename StrOrInt, class Proxy,
			class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(
			const asio::ssl::context& ctx, String&& host, StrOrInt&& port,
			std::string_view target, Proxy&& proxy)
		{
			return derived_t::execute(ctx,
				std::forward<String>(host), std::forward<StrOrInt>(port),
				target, std::chrono::milliseconds(http_execute_timeout), std::forward<Proxy>(proxy));
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<typename String, typename StrOrInt, class Proxy, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		typename std::enable_if_t<detail::can_convert_to_string_v<detail::remove_cvref_t<String>>,
			http::response<Body, Fields>>
		static inline execute(String&& host, StrOrInt&& port, std::string_view target,
			std::chrono::duration<Rep, Period> timeout, Proxy&& proxy)
		{
			return derived_t::execute(asio::ssl::context{ asio::ssl::context::sslv23 },
				std::forward<String>(host), std::forward<StrOrInt>(port),
				target, timeout, std::forward<Proxy>(proxy));
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<typename String, typename StrOrInt, class Proxy,
			class Body = http::string_body, class Fields = http::fields>
		typename std::enable_if_t<detail::can_convert_to_string_v<detail::remove_cvref_t<String>>,
			http::response<Body, Fields>>
		static inline execute(String&& host, StrOrInt&& port, std::string_view target, Proxy&& proxy)
		{
			return derived_t::execute(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				target, std::chrono::milliseconds(http_execute_timeout), std::forward<Proxy>(proxy));
		}

	public:
		/**
		 * @function : bind ssl handshake listener
		 * @param    : fun - a user defined callback function
		 * Function signature : void()
		 */
		template<class F, class ...C>
		inline derived_t & bind_handshake(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::handshake,
				observer_t<>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	protected:
		template<typename MatchCondition>
		inline void _do_init(condition_wrap<MatchCondition> condition)
		{
			super::_do_init(condition);

			this->derived()._ssl_init(condition, this->socket_, *this);
		}

		template<typename DeferEvent>
		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			this->derived()._rdc_stop();

			this->derived()._ssl_stop(this_ptr,
				defer_event
				{
					[this, ec, this_ptr, e = chain.move_event()] (event_queue_guard<derived_t> g) mutable
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
			{
				return this->derived()._done_connect(ec, std::move(this_ptr), std::move(condition), std::move(chain));
			}

			this->derived()._ssl_start(this_ptr, condition, this->socket_, *this);

			this->derived()._post_handshake(std::move(this_ptr), std::move(condition), std::move(chain));
		}

		inline void _fire_handshake(std::shared_ptr<derived_t>& this_ptr)
		{
			// the _fire_handshake must be executed in the thread 0.
			ASIO2_ASSERT(this->derived().io().running_in_this_thread());

			detail::ignore_unused(this_ptr);

			this->listener_.notify(event_type::handshake);
		}

	protected:
	};
}

namespace asio2
{
	template<class derived_t>
	class https_client_t : public detail::https_client_impl_t<derived_t, detail::template_args_http_client>
	{
	public:
		using detail::https_client_impl_t<derived_t, detail::template_args_http_client>::https_client_impl_t;
	};

	class https_client : public https_client_t<https_client>
	{
	public:
		using https_client_t<https_client>::https_client_t;
	};
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_HTTPS_CLIENT_HPP__

#endif
