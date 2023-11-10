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

#ifndef __ASIO2_HTTPS_DOWNLOAD_HPP__
#define __ASIO2_HTTPS_DOWNLOAD_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <fstream>

#include <asio2/base/detail/function_traits.hpp>

#include <asio2/http/detail/http_util.hpp>
#include <asio2/http/detail/http_make.hpp>
#include <asio2/http/detail/http_traits.hpp>

#include <asio2/component/socks/socks5_client_cp.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t, bool Enable = is_https_execute_download_enabled<args_t>::value()>
	struct https_download_impl_bridge;

	template<class derived_t, class args_t>
	struct https_download_impl_bridge<derived_t, args_t, false>
	{
	};

	template<class derived_t, class args_t>
	struct https_download_impl_bridge<derived_t, args_t, true>
	{
	protected:
		template<typename String, typename StrOrInt, class HeaderCallback, class BodyCallback, class Proxy,
			class Body, class Fields, class Buffer>
		static void _download_with_socks5(
			asio::io_context& ioc, asio::ip::tcp::resolver& resolver, asio::ip::tcp::socket& socket,
			asio::ssl::stream<asio::ip::tcp::socket&>& stream,
			Buffer& buffer,
			String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req, HeaderCallback&& cbh, BodyCallback&& cbb, Proxy&& proxy)
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

									http::read_large_body<false>(stream, buffer,
										std::forward<HeaderCallback>(cbh), std::forward<BodyCallback>(cbb));

									stream.async_shutdown([](const error_code&) mutable {});
								});
							});
						}
					);
				});
			});
		}

		template<typename String, typename StrOrInt, class HeaderCallback, class BodyCallback,
			class Body, class Fields, class Buffer>
		static void _download_trivially(
			asio::io_context& ioc, asio::ip::tcp::resolver& resolver, asio::ip::tcp::socket& socket,
			asio::ssl::stream<asio::ip::tcp::socket&>& stream,
			Buffer& buffer,
			String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req, HeaderCallback&& cbh, BodyCallback&& cbb)
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

							http::read_large_body<false>(stream, buffer,
								std::forward<HeaderCallback>(cbh), std::forward<BodyCallback>(cbb));

							stream.async_shutdown([](const error_code&) mutable {});
						});
					});
				});
			});
		}

		template<typename String, typename StrOrInt, class HeaderCallback, class BodyCallback, class Proxy,
			class Body, class Fields, class Buffer>
		static void _download_impl(
			asio::io_context& ioc, asio::ip::tcp::resolver& resolver, asio::ip::tcp::socket& socket,
			asio::ssl::stream<asio::ip::tcp::socket&>& stream,
			Buffer& buffer,
			String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req, HeaderCallback&& cbh, BodyCallback&& cbb, Proxy&& proxy)
		{
			// if has socks5 proxy
			if constexpr (std::is_base_of_v<asio2::socks5::option_base,
				typename detail::element_type_adapter<detail::remove_cvref_t<Proxy>>::type>)
			{
				derived_t::_download_with_socks5(ioc, resolver, socket, stream, buffer
					, std::forward<String>(host), std::forward<StrOrInt>(port)
					, req, std::forward<HeaderCallback>(cbh), std::forward<BodyCallback>(cbb)
					, std::forward<Proxy>(proxy)
				);
			}
			else
			{
				detail::ignore_unused(proxy);

				derived_t::_download_trivially(ioc, resolver, socket, stream, buffer
					, std::forward<String>(host), std::forward<StrOrInt>(port)
					, req, std::forward<HeaderCallback>(cbh), std::forward<BodyCallback>(cbb)
				);
			}
		}

	public:
		/**
		 * @brief blocking download the http file until it is returned on success or failure
		 * @param ctx - asio ssl context.
		 * @param host - The ip of the server.
		 * @param port - The port of the server.
		 * @param req - The http request object to send to the https server.
		 * @param cbh - A function that recv the http response header message. void(auto& message)
		 * @param cbb - A function that circularly receives the contents of the file in chunks. void(std::string_view data)
		 * @param proxy - socks5 proxy, if you do not need a proxy, pass "nullptr" is ok.
		 */
		template<typename String, typename StrOrInt, class HeaderCallback, class BodyCallback, class Proxy,
			class Body = http::string_body, class Fields = http::fields, class Buffer = beast::flat_buffer>
		typename std::enable_if_t<detail::is_character_string_v<detail::remove_cvref_t<String>>
			&& detail::http_proxy_checker_v<Proxy>, bool>
		static inline download(const asio::ssl::context& ctx, String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req, HeaderCallback&& cbh, BodyCallback&& cbb, Proxy&& proxy)
		{
			// First assign default value 0 to last error
			clear_last_error();

			// The io_context is required for all I/O
			asio::io_context ioc;

			// These objects perform our I/O
			asio::ip::tcp::resolver resolver{ ioc };
			asio::ip::tcp::socket socket{ ioc };
			asio::ssl::stream<asio::ip::tcp::socket&> stream(socket, const_cast<asio::ssl::context&>(ctx));

			// This buffer is used for reading and must be persisted
			Buffer buffer;

			// Some sites must set the http::field::user_agent
			if (req.find(http::field::user_agent) == req.end())
				req.set(http::field::user_agent,
					"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/105.0.0.0 Safari/537.36");

			// do work
			derived_t::_download_impl(ioc, resolver, socket, stream, buffer
				, std::forward<String>(host), std::forward<StrOrInt>(port)
				, req, std::forward<HeaderCallback>(cbh), std::forward<BodyCallback>(cbb)
				, std::forward<Proxy>(proxy)
			);

			// timedout run
			ioc.run();

			error_code ec_ignore{};

			// Gracefully close the socket
			socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec_ignore);
			socket.cancel(ec_ignore);
			socket.close(ec_ignore);

			return bool(!get_last_error());
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @brief blocking download the http file until it is returned on success or failure
		 * @param req - The web_request which can be create by http::make_request(...)
		 * @param filepath - The file path to saved the received file content.
		 */
		template<class String2>
		typename std::enable_if_t<detail::can_convert_to_string_v<detail::remove_cvref_t<String2>>, bool>
		static inline download(const asio::ssl::context& ctx, http::web_request& req, String2&& filepath)
		{
			std::filesystem::path path(std::forward<String2>(filepath));

			std::filesystem::create_directories(path.parent_path(), get_last_error());

			std::fstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
			if (!file)
			{
				set_last_error(asio::error::access_denied); // Permission denied
				return false;
			}

			auto cbh = [](const auto&) {};
			auto cbb = [&file](std::string_view chunk) mutable
			{
				file.write(chunk.data(), chunk.size());
				return true;
			};

			return derived_t::download(ctx, req.host(), req.port(), req.base(), cbh, cbb, std::in_place);
		}

		/**
		 * @brief blocking download the http file until it is returned on success or failure
		 * @param req - The web_request which can be create by http::make_request(...)
		 * @param filepath - The file path to saved the received file content.
		 */
		template<class String2>
		typename std::enable_if_t<detail::can_convert_to_string_v<detail::remove_cvref_t<String2>>, bool>
		static inline download(http::web_request& req, String2&& filepath)
		{
			return derived_t::download(asio::ssl::context{ ASIO2_DEFAULT_SSL_METHOD },
				req, std::forward<String2>(filepath));
		}

		/**
		 * @brief blocking download the http file until it is returned on success or failure
		 * @param url - The url of the file to download.
		 * @param filepath - The file path to saved the received file content.
		 */
		template<class String1, class String2>
		typename std::enable_if_t<
			detail::can_convert_to_string_v<detail::remove_cvref_t<String1>> &&
			detail::can_convert_to_string_v<detail::remove_cvref_t<String2>>, bool>
		static inline download(const asio::ssl::context& ctx, String1&& url, String2&& filepath)
		{
			http::web_request req = http::make_request(std::forward<String1>(url));
			if (get_last_error())
				return false;

			return derived_t::download(ctx, req, std::forward<String2>(filepath));
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @brief blocking download the http file until it is returned on success or failure
		 * @param url - The url of the file to download.
		 * @param filepath - The file path to saved the received file content.
		 */
		template<class String1, class String2>
		typename std::enable_if_t<
			detail::can_convert_to_string_v<detail::remove_cvref_t<String1>> &&
			detail::can_convert_to_string_v<detail::remove_cvref_t<String2>>, bool>
		static inline download(String1&& url, String2&& filepath)
		{
			return derived_t::download(asio::ssl::context{ ASIO2_DEFAULT_SSL_METHOD },
				std::forward<String1>(url), std::forward<String2>(filepath));
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @brief blocking download the http file until it is returned on success or failure
		 * @param url - The url of the file to download.
		 * @param cbb - A function that circularly receives the contents of the file in chunks. void(std::string_view data)
		 */
		template<class String1, class BodyCallback>
		typename std::enable_if_t<
			detail::can_convert_to_string_v<detail::remove_cvref_t<String1>> &&
			detail::is_callable_v<BodyCallback>, bool>
		static inline download(const asio::ssl::context& ctx, String1&& url, BodyCallback&& cbb)
		{
			http::web_request req = http::make_request(std::forward<String1>(url));
			if (get_last_error())
				return false;

			auto cbh = [](const auto&) {};

			return derived_t::download(ctx, req.host(), req.port(), req.base(), cbh, cbb, std::in_place);
		}

		/**
		 * @brief blocking download the http file until it is returned on success or failure
		 * @param req - The web_request which can be create by http::make_request(...)
		 * @param cbb - A function that circularly receives the contents of the file in chunks. void(std::string_view data)
		 */
		template<class BodyCallback>
		typename std::enable_if_t<detail::is_callable_v<BodyCallback>, bool>
		static inline download(const asio::ssl::context& ctx, http::web_request& req, BodyCallback&& cbb)
		{
			auto cbh = [](const auto&) {};

			return derived_t::download(ctx, req.host(), req.port(), req.base(), cbh, cbb, std::in_place);
		}

		/**
		 * @brief blocking download the http file until it is returned on success or failure
		 * @param req - The web_request which can be create by http::make_request(...)
		 * @param cbb - A function that circularly receives the contents of the file in chunks. void(std::string_view data)
		 */
		template<class BodyCallback>
		typename std::enable_if_t<detail::is_callable_v<BodyCallback>, bool>
		static inline download(http::web_request& req, BodyCallback&& cbb)
		{
			return derived_t::download(asio::ssl::context{ ASIO2_DEFAULT_SSL_METHOD }, req, cbb);
		}

		/**
		 * @brief blocking download the http file until it is returned on success or failure
		 * @param req - The web_request which can be create by http::make_request(...)
		 * @param cbh - A function that recv the http response header message. void(auto& message)
		 * @param cbb - A function that circularly receives the contents of the file in chunks. void(std::string_view data)
		 */
		template<class HeaderCallback, class BodyCallback>
		typename std::enable_if_t<detail::is_callable_v<BodyCallback>, bool>
		static inline download(const asio::ssl::context& ctx, http::web_request& req, HeaderCallback&& cbh, BodyCallback&& cbb)
		{
			return derived_t::download(ctx, req.host(), req.port(), req.base(), cbh, cbb, std::in_place);
		}

		/**
		 * @brief blocking download the http file until it is returned on success or failure
		 * @param req - The web_request which can be create by http::make_request(...)
		 * @param cbh - A function that recv the http response header message. void(auto& message)
		 * @param cbb - A function that circularly receives the contents of the file in chunks. void(std::string_view data)
		 */
		template<class HeaderCallback, class BodyCallback>
		typename std::enable_if_t<detail::is_callable_v<BodyCallback>, bool>
		static inline download(http::web_request& req, HeaderCallback&& cbh, BodyCallback&& cbb)
		{
			return derived_t::download(asio::ssl::context{ ASIO2_DEFAULT_SSL_METHOD }, req, cbh, cbb);
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @brief blocking download the http file until it is returned on success or failure
		 * @param url - The url of the file to download.
		 * @param cbb - A function that circularly receives the contents of the file in chunks. void(std::string_view data)
		 */
		template<class String1, class BodyCallback>
		typename std::enable_if_t<
			detail::can_convert_to_string_v<detail::remove_cvref_t<String1>> &&
			detail::is_callable_v<BodyCallback>, bool>
		static inline download(String1&& url, BodyCallback&& cbb)
		{
			return derived_t::download(asio::ssl::context{ ASIO2_DEFAULT_SSL_METHOD },
				std::forward<String1>(url), std::forward<BodyCallback>(cbb));
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @brief blocking download the http file until it is returned on success or failure
		 * @param url - The url of the file to download.
		 * @param cbh - A function that recv the http response header message. void(auto& message)
		 * @param cbb - A function that circularly receives the contents of the file in chunks. void(std::string_view data)
		 */
		template<class String1, class HeaderCallback, class BodyCallback>
		typename std::enable_if_t<
			detail::can_convert_to_string_v<detail::remove_cvref_t<String1>> &&
			detail::is_callable_v<BodyCallback>, bool>
		static inline download(const asio::ssl::context& ctx, String1&& url, HeaderCallback&& cbh, BodyCallback&& cbb)
		{
			http::web_request req = http::make_request(std::forward<String1>(url));
			if (get_last_error())
				return false;

			return derived_t::download(ctx, req.host(), req.port(), req.base(), cbh, cbb, std::in_place);
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @brief blocking download the http file until it is returned on success or failure
		 * @param url - The url of the file to download.
		 * @param cbh - A function that recv the http response header message. void(auto& message)
		 * @param cbb - A function that circularly receives the contents of the file in chunks. void(std::string_view data)
		 */
		template<class String1, class HeaderCallback, class BodyCallback>
		typename std::enable_if_t<
			detail::can_convert_to_string_v<detail::remove_cvref_t<String1>> &&
			detail::is_callable_v<BodyCallback>, bool>
		static inline download(String1&& url, HeaderCallback&& cbh, BodyCallback&& cbb)
		{
			return derived_t::download(asio::ssl::context{ ASIO2_DEFAULT_SSL_METHOD },
				std::forward<String1>(url), std::forward<HeaderCallback>(cbh), std::forward<BodyCallback>(cbb));
		}
	};

	template<class derived_t, class args_t = void>
	struct https_download_impl : public https_download_impl_bridge<derived_t, args_t> {};
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_HTTPS_DOWNLOAD_HPP__

#endif
