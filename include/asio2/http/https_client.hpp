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
		template<typename String, typename StrOrInt, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields, class Buffer = beast::flat_buffer>
		static inline http::response<Body, Fields> execute(
			const asio::ssl::context& ctx, String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req, std::chrono::duration<Rep, Period> timeout, error_code& ec)
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
				asio::ssl::stream<asio::ip::tcp::socket&> stream(socket, const_cast<asio::ssl::context&>(ctx));

				// This buffer is used for reading and must be persisted
				Buffer buffer;

				// Look up the domain name
				resolver.async_resolve(std::forward<String>(host), to_string(std::forward<StrOrInt>(port)),
				[&](const error_code& ec1, const asio::ip::tcp::resolver::results_type& endpoints) mutable
				{
					if (ec1) { ec = ec1; return; }

					// Make the connection on the IP address we get from a lookup
					asio::async_connect(socket, endpoints,
					[&](const error_code & ec2, const asio::ip::tcp::endpoint&) mutable
					{
						if (ec2) { ec = ec2; return; }

						stream.async_handshake(asio::ssl::stream_base::client,
						[&](const error_code& ec3) mutable
						{
							if (ec3) { ec = ec3; return; }

							http::async_write(stream, req, [&](const error_code& ec4, std::size_t) mutable
							{
								// can't use stream.shutdown(),in some case the shutdowm will blocking forever.
								if (ec4) { ec = ec4; stream.async_shutdown([](const error_code&) {}); return; }

								// Then start asynchronous reading
								http::async_read(stream, buffer, parser,
								[&](const error_code& ec5, std::size_t) mutable
								{
									// Reading completed, assign the read the result to ec
									// If the code does not execute into here, the ec value
									// is the default value timed_out.
									ec = ec5;

									stream.async_shutdown([](const error_code&) mutable {});
								});
							});
						});
					});
				});

				// timedout run
				ioc.run_for(timeout);

				error_code ec_ignore{};

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

		// ----------------------------------------------------------------------------------------

		template<typename String, typename StrOrInt, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(
			const asio::ssl::context& ctx, String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req, std::chrono::duration<Rep, Period> timeout)
		{
			error_code ec;
			http::response<Body, Fields> rep = execute(ctx,
				std::forward<String>(host), std::forward<StrOrInt>(port), req, timeout, ec);
			asio::detail::throw_error(ec);
			return rep;
		}

		template<typename String, typename StrOrInt,
			class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(
			const asio::ssl::context& ctx, String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req, error_code& ec)
		{
			return execute(ctx, std::forward<String>(host), std::forward<StrOrInt>(port),
				req, std::chrono::milliseconds(http_execute_timeout), ec);
		}

		// ----------------------------------------------------------------------------------------

		template<typename String, typename StrOrInt, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req, std::chrono::duration<Rep, Period> timeout, error_code& ec)
		{
			return execute(asio::ssl::context{ asio::ssl::context::sslv23 },
				std::forward<String>(host), std::forward<StrOrInt>(port), req, timeout, ec);
		}

		template<typename String, typename StrOrInt, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req, std::chrono::duration<Rep, Period> timeout)
		{
			error_code ec;
			http::response<Body, Fields> rep = execute(
				std::forward<String>(host), std::forward<StrOrInt>(port), req, timeout, ec);
			asio::detail::throw_error(ec);
			return rep;
		}

		template<typename String, typename StrOrInt,
			class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req, error_code& ec)
		{
			return execute(std::forward<String>(host), std::forward<StrOrInt>(port),
				req, std::chrono::milliseconds(http_execute_timeout), ec);
		}

		template<typename String, typename StrOrInt,
			class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req)
		{
			error_code ec;
			http::response<Body, Fields> rep = execute(
				std::forward<String>(host), std::forward<StrOrInt>(port), req, ec);
			asio::detail::throw_error(ec);
			return rep;
		}

		// ----------------------------------------------------------------------------------------

		template<class Rep, class Period, class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(
			const asio::ssl::context& ctx, http::web_request& req, std::chrono::duration<Rep, Period> timeout,
			error_code& ec)
		{
			return execute(ctx, req.url().host(), req.url().port(), req.base(), timeout, ec);
		}

		template<class Rep, class Period, class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(
			const asio::ssl::context& ctx, http::web_request& req, std::chrono::duration<Rep, Period> timeout)
		{
			error_code ec;
			http::response<Body, Fields> rep = execute(ctx, req, timeout, ec);
			asio::detail::throw_error(ec);
			return rep;
		}

		template<class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(
			const asio::ssl::context& ctx, http::web_request& req, error_code& ec)
		{
			return execute(ctx, req, std::chrono::milliseconds(http_execute_timeout), ec);
		}

		template<class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(
			const asio::ssl::context& ctx, http::web_request& req)
		{
			error_code ec;
			http::response<Body, Fields> rep = execute(ctx, req, ec);
			asio::detail::throw_error(ec);
			return rep;
		}

		template<class Rep, class Period, class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(
			http::web_request& req, std::chrono::duration<Rep, Period> timeout, error_code& ec)
		{
			return execute(asio::ssl::context{ asio::ssl::context::sslv23 }, req, timeout, ec);
		}

		template<class Rep, class Period, class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(
			http::web_request& req, std::chrono::duration<Rep, Period> timeout)
		{
			error_code ec;
			http::response<Body, Fields> rep = execute(req, timeout, ec);
			asio::detail::throw_error(ec);
			return rep;
		}

		template<class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(http::web_request& req, error_code& ec)
		{
			return execute(req, std::chrono::milliseconds(http_execute_timeout), ec);
		}

		template<class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(http::web_request& req)
		{
			error_code ec;
			http::response<Body, Fields> rep = execute(req, ec);
			asio::detail::throw_error(ec);
			return rep;
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<class Rep, class Period, class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(const asio::ssl::context& ctx, std::string_view url,
			std::chrono::duration<Rep, Period> timeout, error_code& ec)
		{
			http::web_request req = http::make_request(url, ec);
			if (ec)
			{
				return http::response<Body, Fields>{ http::status::unknown, 11};
			}
			return execute(ctx, req.host(), req.port(), req.base(), timeout, ec);
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<class Rep, class Period, class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(const asio::ssl::context& ctx, std::string_view url,
			std::chrono::duration<Rep, Period> timeout)
		{
			error_code ec;
			http::response<Body, Fields> rep = execute(ctx, url, timeout, ec);
			asio::detail::throw_error(ec);
			return rep;
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(const asio::ssl::context& ctx, std::string_view url,
			error_code& ec)
		{
			return execute(ctx, url, std::chrono::milliseconds(http_execute_timeout), ec);
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(const asio::ssl::context& ctx, std::string_view url)
		{
			error_code ec;
			http::response<Body, Fields> rep = execute(ctx, url, ec);
			asio::detail::throw_error(ec);
			return rep;
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<class Rep, class Period, class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(std::string_view url,
			std::chrono::duration<Rep, Period> timeout, error_code& ec)
		{
			return execute(asio::ssl::context{ asio::ssl::context::sslv23 }, url, timeout, ec);
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<class Rep, class Period, class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(std::string_view url,
			std::chrono::duration<Rep, Period> timeout)
		{
			error_code ec;
			http::response<Body, Fields> rep = execute(url, timeout, ec);
			asio::detail::throw_error(ec);
			return rep;
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(std::string_view url, error_code& ec)
		{
			return execute(url, std::chrono::milliseconds(http_execute_timeout), ec);
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(std::string_view url)
		{
			error_code ec;
			http::response<Body, Fields> rep = execute(url, ec);
			asio::detail::throw_error(ec);
			return rep;
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<typename String, typename StrOrInt, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(String&& host, StrOrInt&& port,
			std::string_view target, std::chrono::duration<Rep, Period> timeout, error_code& ec)
		{
			http::web_request req = http::make_request(host, port, target);
			return execute(asio::ssl::context{ asio::ssl::context::sslv23 },
				std::forward<String>(host), std::forward<StrOrInt>(port),
				req.base(), timeout, ec);
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<typename String, typename StrOrInt, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(String&& host, StrOrInt&& port,
			std::string_view target, std::chrono::duration<Rep, Period> timeout)
		{
			error_code ec;
			http::response<Body, Fields> rep = execute(
				std::forward<String>(host), std::forward<StrOrInt>(port), target, timeout, ec);
			asio::detail::throw_error(ec);
			return rep;
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<typename String, typename StrOrInt,
			class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(String&& host, StrOrInt&& port,
			std::string_view target, error_code& ec)
		{
			return execute(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				target, std::chrono::milliseconds(http_execute_timeout), ec);
		}

		/**
		 * @function : blocking execute the http request until it is returned on success or failure
		 */
		template<typename String, typename StrOrInt,
			class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(String&& host, StrOrInt&& port,
			std::string_view target)
		{
			error_code ec;
			http::response<Body, Fields> rep = execute(
				std::forward<String>(host), std::forward<StrOrInt>(port), target, ec);
			asio::detail::throw_error(ec);
			return rep;
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

		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			this->derived()._rdc_stop();

			this->derived()._ssl_stop(this_ptr, [this, ec, this_ptr]() mutable
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
			{
				return this->derived()._done_connect(ec, std::move(this_ptr), std::move(condition));
			}

			this->derived()._ssl_start(this_ptr, condition, this->socket_, *this);

			this->derived()._post_handshake(std::move(this_ptr), std::move(condition));
		}

		inline void _fire_handshake(std::shared_ptr<derived_t>& this_ptr)
		{
			// the _fire_handshake must be executed in the thread 0.
			ASIO2_ASSERT(this->derived().io().strand().running_in_this_thread());

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
