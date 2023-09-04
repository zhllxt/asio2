/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#if defined(ASIO2_ENABLE_SSL) || defined(ASIO2_USE_SSL)

#ifndef __ASIO2_HTTPS_EXECUTE_HPP__
#define __ASIO2_HTTPS_EXECUTE_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/base/detail/function_traits.hpp>

#include <asio2/http/detail/http_util.hpp>
#include <asio2/http/detail/http_make.hpp>
#include <asio2/http/detail/http_traits.hpp>

#include <asio2/component/socks/socks5_client_cp.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t, bool Enable = is_https_execute_download_enabled<args_t>::value()>
	struct https_execute_impl_bridge;

	template<class derived_t, class args_t>
	struct https_execute_impl_bridge<derived_t, args_t, false>
	{
	};

	template<class derived_t, class args_t>
	struct https_execute_impl_bridge<derived_t, args_t, true>
	{
	protected:
		template<typename String, typename StrOrInt, class Proxy, class Body, class Fields, class Buffer>
		static void _execute_with_socks5(
			asio::io_context& ioc, asio::ip::tcp::resolver& resolver, asio::ip::tcp::socket& socket,
			asio::ssl::stream<asio::ip::tcp::socket&>& stream,
			http::parser<false, Body, typename Fields::allocator_type>& parser,
			Buffer& buffer,
			String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req, Proxy&& proxy)
		{
			auto sk5 = detail::to_shared_ptr(std::forward<Proxy>(proxy));

			std::string_view h{ sk5->host() };
			std::string_view p{ sk5->port() };
			
			if (static_cast<int>(sk5->command()) == 0)
			{
				sk5->command(socks5::command::connect);
			}

			// Look up the domain name
			resolver.async_resolve(h, p, [&, s5 = std::move(sk5)]
			(const error_code& ec1, const asio::ip::tcp::resolver::results_type& endpoints) mutable
			{
				if (ec1) { set_last_error(ec1); return; }

				// Make the connection on the IP address we get from a lookup
				asio::async_connect(socket, endpoints,
				[&, s5 = std::move(s5)](const error_code& ec2, const asio::ip::tcp::endpoint&) mutable
				{
					if (ec2) { set_last_error(ec2); return; }

					socks5_async_handshake
					(
						detail::to_string(std::forward<String  >(host)),
						detail::to_string(std::forward<StrOrInt>(port)),
						socket,
						std::move(s5),
						[&](error_code ecs5, std::string, std::string) mutable
						{
							if (ecs5) { set_last_error(ecs5); return; }

							// https://github.com/djarek/certify
							if (auto it = req.find(http::field::host); it != req.end())
							{
								std::string hostname(it->value());
								SSL_set_tlsext_host_name(stream.native_handle(), hostname.data());
							}

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
					);
				});
			});
		}

		template<typename String, typename StrOrInt, class Body, class Fields, class Buffer>
		static void _execute_trivially(
			asio::io_context& ioc, asio::ip::tcp::resolver& resolver, asio::ip::tcp::socket& socket,
			asio::ssl::stream<asio::ip::tcp::socket&>& stream,
			http::parser<false, Body, typename Fields::allocator_type>& parser,
			Buffer& buffer,
			String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req)
		{
			detail::ignore_unused(ioc);

			// Look up the domain name
			resolver.async_resolve(std::forward<String>(host), detail::to_string(std::forward<StrOrInt>(port)),
			[&](const error_code& ec1, const asio::ip::tcp::resolver::results_type& endpoints) mutable
			{
				if (ec1) { set_last_error(ec1); return; }

				// Make the connection on the IP address we get from a lookup
				asio::async_connect(socket, endpoints,
				[&](const error_code& ec2, const asio::ip::tcp::endpoint&) mutable
				{
					if (ec2) { set_last_error(ec2); return; }

					// https://github.com/djarek/certify
					if (auto it = req.find(http::field::host); it != req.end())
					{
						std::string hostname(it->value());
						SSL_set_tlsext_host_name(stream.native_handle(), hostname.data());
					}

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

		template<typename String, typename StrOrInt, class Proxy, class Body, class Fields, class Buffer>
		static void _execute_impl(
			asio::io_context& ioc, asio::ip::tcp::resolver& resolver, asio::ip::tcp::socket& socket,
			asio::ssl::stream<asio::ip::tcp::socket&>& stream,
			http::parser<false, Body, typename Fields::allocator_type>& parser,
			Buffer& buffer,
			String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req, Proxy&& proxy)
		{
			// if has socks5 proxy
			if constexpr (std::is_base_of_v<asio2::socks5::option_base,
				typename detail::element_type_adapter<detail::remove_cvref_t<Proxy>>::type>)
			{
				derived_t::_execute_with_socks5(ioc, resolver, socket, stream, parser, buffer
					, std::forward<String>(host), std::forward<StrOrInt>(port)
					, req
					, std::forward<Proxy>(proxy)
				);
			}
			else
			{
				detail::ignore_unused(proxy);

				derived_t::_execute_trivially(ioc, resolver, socket, stream, parser, buffer
					, std::forward<String>(host), std::forward<StrOrInt>(port)
					, req
				);
			}
		}

	public:
		template<typename String, typename StrOrInt, class Rep, class Period, class Proxy,
			class Body = http::string_body, class Fields = http::fields, class Buffer = beast::flat_buffer>
		typename std::enable_if_t<detail::is_character_string_v<detail::remove_cvref_t<String>>
			&& detail::http_proxy_checker_v<Proxy>, http::response<Body, Fields>>
		static inline execute(const asio::ssl::context& ctx, String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req, std::chrono::duration<Rep, Period> timeout, Proxy&& proxy)
		{
			http::parser<false, Body, typename Fields::allocator_type> parser;

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

			// do work
			derived_t::_execute_impl(ioc, resolver, socket, stream, parser, buffer
				, std::forward<String>(host), std::forward<StrOrInt>(port)
				, req
				, std::forward<Proxy>(proxy)
			);

			// timedout run
			ioc.run_for(timeout);

			error_code ec_ignore{};

			// Gracefully close the socket
			socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec_ignore);
			socket.cancel(ec_ignore);
			socket.close(ec_ignore);

			return parser.release();
		}

		// ----------------------------------------------------------------------------------------

		template<typename String, typename StrOrInt, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		typename std::enable_if_t<detail::is_character_string_v<detail::remove_cvref_t<String>>
			, http::response<Body, Fields>>
		static inline execute(
			const asio::ssl::context& ctx, String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req, std::chrono::duration<Rep, Period> timeout)
		{
			return derived_t::execute(ctx, std::forward<String>(host), std::forward<StrOrInt>(port),
				req, timeout, std::in_place);
		}

		template<typename String, typename StrOrInt,
			class Body = http::string_body, class Fields = http::fields>
		typename std::enable_if_t<detail::is_character_string_v<detail::remove_cvref_t<String>>
			, http::response<Body, Fields>>
		static inline execute(
			const asio::ssl::context& ctx, String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req)
		{
			return derived_t::execute(ctx, std::forward<String>(host), std::forward<StrOrInt>(port),
				req, std::chrono::milliseconds(http_execute_timeout), std::in_place);
		}

		// ----------------------------------------------------------------------------------------

		template<typename String, typename StrOrInt, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		typename std::enable_if_t<detail::is_character_string_v<detail::remove_cvref_t<String>>
			, http::response<Body, Fields>>
		static inline execute(String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req, std::chrono::duration<Rep, Period> timeout)
		{
			return derived_t::execute(asio::ssl::context{ ASIO2_DEFAULT_SSL_METHOD },
				std::forward<String>(host), std::forward<StrOrInt>(port), req, timeout, std::in_place);
		}

		template<typename String, typename StrOrInt, class Body = http::string_body, class Fields = http::fields>
		typename std::enable_if_t<detail::is_character_string_v<detail::remove_cvref_t<String>>
			, http::response<Body, Fields>>
		static inline execute(String&& host, StrOrInt&& port, http::request<Body, Fields>& req)
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
			return derived_t::execute(asio::ssl::context{ ASIO2_DEFAULT_SSL_METHOD }, req, timeout);
		}

		template<class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(http::web_request& req)
		{
			return derived_t::execute(req, std::chrono::milliseconds(http_execute_timeout));
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @brief blocking execute the http request until it is returned on success or failure
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
		 * @brief blocking execute the http request until it is returned on success or failure
		 */
		template<class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(const asio::ssl::context& ctx, std::string_view url)
		{
			return derived_t::execute(ctx, url, std::chrono::milliseconds(http_execute_timeout));
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @brief blocking execute the http request until it is returned on success or failure
		 */
		template<class Rep, class Period, class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(std::string_view url,
			std::chrono::duration<Rep, Period> timeout)
		{
			return derived_t::execute(asio::ssl::context{ ASIO2_DEFAULT_SSL_METHOD }, url, timeout);
		}

		/**
		 * @brief blocking execute the http request until it is returned on success or failure
		 */
		template<class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(std::string_view url)
		{
			return derived_t::execute(url, std::chrono::milliseconds(http_execute_timeout));
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @brief blocking execute the http request until it is returned on success or failure
		 */
		template<typename String, typename StrOrInt, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		typename std::enable_if_t<detail::is_character_string_v<detail::remove_cvref_t<String>>
			, http::response<Body, Fields>>
		static inline execute(
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
		 * @brief blocking execute the http request until it is returned on success or failure
		 */
		template<typename String, typename StrOrInt, class Body = http::string_body, class Fields = http::fields>
		typename std::enable_if_t<detail::is_character_string_v<detail::remove_cvref_t<String>>
			, http::response<Body, Fields>>
		static inline execute(
			const asio::ssl::context& ctx, String&& host, StrOrInt&& port,
			std::string_view target)
		{
			return derived_t::execute(ctx,
				std::forward<String>(host), std::forward<StrOrInt>(port),
				target, std::chrono::milliseconds(http_execute_timeout));
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @brief blocking execute the http request until it is returned on success or failure
		 */
		template<typename String, typename StrOrInt, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		typename std::enable_if_t<detail::can_convert_to_string_v<detail::remove_cvref_t<String>>,
			http::response<Body, Fields>>
		static inline execute(String&& host, StrOrInt&& port, std::string_view target,
			std::chrono::duration<Rep, Period> timeout)
		{
			return derived_t::execute(asio::ssl::context{ ASIO2_DEFAULT_SSL_METHOD },
				std::forward<String>(host), std::forward<StrOrInt>(port),
				target, timeout);
		}

		/**
		 * @brief blocking execute the http request until it is returned on success or failure
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
		typename std::enable_if_t<detail::is_character_string_v<detail::remove_cvref_t<String>>
			&& detail::http_proxy_checker_v<Proxy>, http::response<Body, Fields>>
		static inline execute(
			const asio::ssl::context& ctx, String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req, Proxy&& proxy)
		{
			return derived_t::execute(ctx, std::forward<String>(host), std::forward<StrOrInt>(port),
				req, std::chrono::milliseconds(http_execute_timeout), std::forward<Proxy>(proxy));
		}

		// ----------------------------------------------------------------------------------------

		template<typename String, typename StrOrInt, class Proxy, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		typename std::enable_if_t<detail::is_character_string_v<detail::remove_cvref_t<String>>
			&& detail::http_proxy_checker_v<Proxy>, http::response<Body, Fields>>
		static inline execute(String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req, std::chrono::duration<Rep, Period> timeout, Proxy&& proxy)
		{
			return derived_t::execute(asio::ssl::context{ ASIO2_DEFAULT_SSL_METHOD },
				std::forward<String>(host), std::forward<StrOrInt>(port),
				req, timeout, std::forward<Proxy>(proxy));
		}

		template<typename String, typename StrOrInt, class Proxy,
			class Body = http::string_body, class Fields = http::fields>
		typename std::enable_if_t<detail::is_character_string_v<detail::remove_cvref_t<String>>
			&& detail::http_proxy_checker_v<Proxy>, http::response<Body, Fields>>
		static inline execute(String&& host, StrOrInt&& port, http::request<Body, Fields>& req, Proxy&& proxy)
		{
			return derived_t::execute(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				req, std::chrono::milliseconds(http_execute_timeout), std::forward<Proxy>(proxy));
		}

		// ----------------------------------------------------------------------------------------

		template<class Proxy, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		typename std::enable_if_t<detail::http_proxy_checker_v<Proxy>, http::response<Body, Fields>>
		static inline execute(
			const asio::ssl::context& ctx, http::web_request& req,
			std::chrono::duration<Rep, Period> timeout, Proxy&& proxy)
		{
			return derived_t::execute(ctx, req.url().host(), req.url().port(), req.base(), timeout,
				std::forward<Proxy>(proxy));
		}

		template<class Proxy, class Body = http::string_body, class Fields = http::fields>
		typename std::enable_if_t<detail::http_proxy_checker_v<Proxy>, http::response<Body, Fields>>
		static inline execute(const asio::ssl::context& ctx, http::web_request& req, Proxy&& proxy)
		{
			return derived_t::execute(ctx, req, std::chrono::milliseconds(http_execute_timeout),
				std::forward<Proxy>(proxy));
		}

		template<class Proxy, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		typename std::enable_if_t<detail::http_proxy_checker_v<Proxy>, http::response<Body, Fields>>
		static inline execute(
			http::web_request& req, std::chrono::duration<Rep, Period> timeout, Proxy&& proxy)
		{
			return derived_t::execute(asio::ssl::context{ ASIO2_DEFAULT_SSL_METHOD }, req, timeout,
				std::forward<Proxy>(proxy));
		}

		template<class Proxy, class Body = http::string_body, class Fields = http::fields>
		typename std::enable_if_t<detail::http_proxy_checker_v<Proxy>, http::response<Body, Fields>>
		static inline execute(http::web_request& req, Proxy&& proxy)
		{
			return derived_t::execute(req, std::chrono::milliseconds(http_execute_timeout),
				std::forward<Proxy>(proxy));
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @brief blocking execute the http request until it is returned on success or failure
		 */
		template<class Proxy, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		typename std::enable_if_t<detail::http_proxy_checker_v<Proxy>, http::response<Body, Fields>>
		static inline execute(
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
		 * @brief blocking execute the http request until it is returned on success or failure
		 */
		template<class Proxy, class Body = http::string_body, class Fields = http::fields>
		typename std::enable_if_t<detail::http_proxy_checker_v<Proxy>, http::response<Body, Fields>>
		static inline execute(const asio::ssl::context& ctx, std::string_view url, Proxy&& proxy)
		{
			return derived_t::execute(ctx, url, std::chrono::milliseconds(http_execute_timeout),
				std::forward<Proxy>(proxy));
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @brief blocking execute the http request until it is returned on success or failure
		 */
		template<class Proxy, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		typename std::enable_if_t<detail::http_proxy_checker_v<Proxy>, http::response<Body, Fields>>
		static inline execute(std::string_view url, std::chrono::duration<Rep, Period> timeout, Proxy&& proxy)
		{
			return derived_t::execute(asio::ssl::context{ ASIO2_DEFAULT_SSL_METHOD }, url, timeout,
				std::forward<Proxy>(proxy));
		}

		/**
		 * @brief blocking execute the http request until it is returned on success or failure
		 */
		template<class Proxy, class Body = http::string_body, class Fields = http::fields>
		typename std::enable_if_t<detail::http_proxy_checker_v<Proxy>, http::response<Body, Fields>>
		static inline execute(std::string_view url, Proxy&& proxy)
		{
			return derived_t::execute(url, std::chrono::milliseconds(http_execute_timeout),
				std::forward<Proxy>(proxy));
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @brief blocking execute the http request until it is returned on success or failure
		 */
		template<typename String, typename StrOrInt, class Proxy, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		typename std::enable_if_t<detail::is_character_string_v<detail::remove_cvref_t<String>>
			&& detail::http_proxy_checker_v<Proxy>, http::response<Body, Fields>>
		static inline execute(
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
		 * @brief blocking execute the http request until it is returned on success or failure
		 */
		template<typename String, typename StrOrInt, class Proxy,
			class Body = http::string_body, class Fields = http::fields>
		typename std::enable_if_t<detail::is_character_string_v<detail::remove_cvref_t<String>>
			&& detail::http_proxy_checker_v<Proxy>, http::response<Body, Fields>>
		static inline execute(
			const asio::ssl::context& ctx, String&& host, StrOrInt&& port,
			std::string_view target, Proxy&& proxy)
		{
			return derived_t::execute(ctx,
				std::forward<String>(host), std::forward<StrOrInt>(port),
				target, std::chrono::milliseconds(http_execute_timeout), std::forward<Proxy>(proxy));
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @brief blocking execute the http request until it is returned on success or failure
		 */
		template<typename String, typename StrOrInt, class Proxy, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		typename std::enable_if_t<detail::is_character_string_v<detail::remove_cvref_t<String>>
			&& detail::http_proxy_checker_v<Proxy>, http::response<Body, Fields>>
		static inline execute(String&& host, StrOrInt&& port, std::string_view target,
			std::chrono::duration<Rep, Period> timeout, Proxy&& proxy)
		{
			return derived_t::execute(asio::ssl::context{ ASIO2_DEFAULT_SSL_METHOD },
				std::forward<String>(host), std::forward<StrOrInt>(port),
				target, timeout, std::forward<Proxy>(proxy));
		}

		/**
		 * @brief blocking execute the http request until it is returned on success or failure
		 */
		template<typename String, typename StrOrInt, class Proxy,
			class Body = http::string_body, class Fields = http::fields>
		typename std::enable_if_t<detail::is_character_string_v<detail::remove_cvref_t<String>>
			&& detail::http_proxy_checker_v<Proxy>, http::response<Body, Fields>>
		static inline execute(String&& host, StrOrInt&& port, std::string_view target, Proxy&& proxy)
		{
			return derived_t::execute(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				target, std::chrono::milliseconds(http_execute_timeout), std::forward<Proxy>(proxy));
		}
	};

	template<class derived_t, class args_t = void>
	struct https_execute_impl : public https_execute_impl_bridge<derived_t, args_t> {};
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_HTTPS_EXECUTE_HPP__

#endif
