/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef ASIO_STANDALONE

#ifndef __ASIO2_HTTP_UTIL_HPP__
#define __ASIO2_HTTP_UTIL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cctype>
#include <sstream>

#include <memory>

#include <asio2/base/selector.hpp>
#include <asio2/base/error.hpp>

#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/buffer_wrap.hpp>
#include <asio2/http/detail/http_parser.h>
#include <asio2/http/detail/mime_types.hpp>

namespace boost::beast::http
{
	namespace
	{
		// Percent-encoding : https://en.wikipedia.org/wiki/Percent-encoding

		template<
			class CharT = char,
			class Traits = std::char_traits<CharT>,
			class Allocator = std::allocator<CharT>
		>
			std::basic_string<CharT, Traits, Allocator> url_decode(std::string_view url)
		{
			std::basic_string<CharT, Traits, Allocator> r;
			r.reserve(url.size());
			using size_type = typename std::string_view::size_type;
			using value_type = typename std::string_view::value_type;
			using rvalue_type = typename std::basic_string<CharT, Traits, Allocator>::value_type;
			for (size_type i = 0; i < url.size(); ++i)
			{
				if (url[i] == static_cast<value_type>('%'))
				{
					if (i + 3 <= url.size())
					{
						value_type h = url[i + 1];
						value_type l = url[i + 2];

						if /**/ (h >= static_cast<value_type>('0') && h <= static_cast<value_type>('9')) h = value_type(h - '0'     );
						else if (h >= static_cast<value_type>('a') && h <= static_cast<value_type>('f')) h = value_type(h - 'a' + 10);
						else if (h >= static_cast<value_type>('A') && h <= static_cast<value_type>('F')) h = value_type(h - 'A' + 10);
						else asio::detail::throw_error(asio::error::invalid_argument);

						if /**/ (l >= static_cast<value_type>('0') && l <= static_cast<value_type>('9')) l = value_type(l - '0'     );
						else if (l >= static_cast<value_type>('a') && l <= static_cast<value_type>('f')) l = value_type(l - 'a' + 10);
						else if (l >= static_cast<value_type>('A') && l <= static_cast<value_type>('F')) l = value_type(l - 'A' + 10);
						else asio::detail::throw_error(asio::error::invalid_argument);

						r += static_cast<rvalue_type>(h * 16 + l);
						i += 2;
					}
					else
						asio::detail::throw_error(asio::error::invalid_argument);
				}
				else
					r += static_cast<rvalue_type>(url[i]);
			}
			return r;
		}

		template<
			class CharT = char,
			class Traits = std::char_traits<CharT>,
			class Allocator = std::allocator<CharT>
		>
			std::basic_string<CharT, Traits, Allocator> url_encode(std::string_view url)
		{
			std::basic_string<CharT, Traits, Allocator> r;
			r.reserve(url.size() * 2);
			using size_type = typename std::string_view::size_type;
			using value_type = typename std::string_view::value_type;
			using rvalue_type = typename std::basic_string<CharT, Traits, Allocator>::value_type;
			size_type i = 0;

			http::cparser::http_parser_url u;
			if (0 == http::cparser::http_parser_parse_url(url.data(), url.size(), 0, &u))
			{
				if (u.field_set & (1 << (int)http::cparser::url_fields::UF_PATH))
				{
					i = u.field_data[(int)http::cparser::url_fields::UF_PATH].off;
					for (size_type n = 0; n < i; ++n)
					{
						r += static_cast<rvalue_type>(url[n]);
					}
					if (i < url.size() && url[i] == static_cast<value_type>('/'))
					{
						r += static_cast<rvalue_type>(url[i++]);
					}
				}
			}

			using namespace std::literals;
			std::string_view reserve = "!*();:@&=$,/?[]-_.~"sv;

			for (; i < url.size(); ++i)
			{
				value_type c = url[i];
				if (std::isalnum(static_cast<char>(c)) || reserve.find(static_cast<char>(c)) != std::string_view::npos)
					r += static_cast<rvalue_type>(c);
				else
				{
					r += static_cast<rvalue_type>('%');
					rvalue_type h = rvalue_type(static_cast<unsigned char>(c) >> 4);
					r += h > rvalue_type(9) ? rvalue_type(h + 55) : rvalue_type(h + 48);
					rvalue_type l = rvalue_type(static_cast<unsigned char>(c) % 16);
					r += l > rvalue_type(9) ? rvalue_type(l + 55) : rvalue_type(l + 48);
				}
			}
			return r;
		}

		template<typename = void>
		bool has_unencode_char(std::string_view s)
		{
			using namespace std::literals;
			std::string_view reserve = "!*();:@&=$,/?[]-_.~"sv;
			std::string_view invalid = " `#{}'\"\\|^+<>"sv;

			for (auto c : s)
			{
				if (invalid.find(c) != std::string_view::npos)
					return true;

				if (c != '%' && !std::isalnum(c) && reserve.find(c) == std::string_view::npos)
					return true;
			}
			return false;
		}

		template<typename = void>
		std::string_view url_to_host(std::string_view url)
		{
			http::cparser::http_parser_url u;
			if (0 != http::cparser::http_parser_parse_url(url.data(), url.size(), 0, &u))
				return std::string_view{};

			if (!(u.field_set & (1 << (int)http::cparser::url_fields::UF_HOST)))
				return std::string_view{};

			return std::string_view{ &url[u.field_data[(int)http::cparser::url_fields::UF_HOST].off],
				u.field_data[(int)http::cparser::url_fields::UF_HOST].len };
		}

		template<typename = void>
		std::string_view url_to_port(std::string_view url)
		{
			http::cparser::http_parser_url u;
			if (0 != http::cparser::http_parser_parse_url(url.data(), url.size(), 0, &u))
				return std::string_view{};

			if (u.field_set & (1 << (int)http::cparser::url_fields::UF_PORT))
				return std::string_view{ &url[u.field_data[(int)http::cparser::url_fields::UF_PORT].off],
				u.field_data[(int)http::cparser::url_fields::UF_PORT].len };

			if (u.field_set & (1 << (int)http::cparser::url_fields::UF_SCHEMA))
			{
				std::string schema(&url[u.field_data[(int)http::cparser::url_fields::UF_SCHEMA].off],
					u.field_data[(int)http::cparser::url_fields::UF_SCHEMA].len);
				std::transform(schema.begin(), schema.end(), schema.begin(), [](std::string::value_type c) { return std::tolower(c); });
				return (schema == "https" ? std::string_view{ "443" } : std::string_view{ "80" });
			}

			return std::string_view{ "80" };
		}

		template<typename = void>
		std::string_view url_to_path(std::string_view url)
		{
			http::cparser::http_parser_url u;
			if (0 != http::cparser::http_parser_parse_url(url.data(), url.size(), 0, &u))
				return std::string_view{};

			if (!(u.field_set & (1 << (int)http::cparser::url_fields::UF_PATH)))
				return std::string_view{};

			return std::string_view{ &url[u.field_data[(int)http::cparser::url_fields::UF_PATH].off],
				u.field_data[(int)http::cparser::url_fields::UF_PATH].len };
		}

		template<typename = void>
		std::string_view url_to_query(std::string_view url)
		{
			http::cparser::http_parser_url u;
			if (0 != http::cparser::http_parser_parse_url(url.data(), url.size(), 0, &u))
				return std::string_view{};

			if (!(u.field_set & (1 << (int)http::cparser::url_fields::UF_QUERY)))
				return std::string_view{};

			return std::string_view{ &url[u.field_data[(int)http::cparser::url_fields::UF_QUERY].off],
				u.field_data[(int)http::cparser::url_fields::UF_QUERY].len };
		}

		/**
		 * @function : make A typical HTTP request struct from the uri
		 * You need to encode the "uri"(by url_encode) before calling this function
		 */
		template<class Body = string_body, class Fields = fields>
		request<Body, Fields> make_request(std::string_view uri, error_code& ec)
		{
			request<Body, Fields> req;
			try
			{
				ec.clear();

				cparser::http_parser_url u;
				int state = cparser::http_parser_parse_url(uri.data(), uri.size(), 0, &u);

				// If a \r\n string is found, it is not a URL
				if (uri.find("\r\n") != std::string_view::npos && 0 != state)
				{
					request_parser<Body> parser;
					parser.eager(true);
					parser.put(asio::buffer(uri), ec);
					req = parser.get();
					asio::detail::throw_error(ec);
				}
				// It is a URL
				else
				{
					ASIO2_ASSERT(!has_unencode_char(uri));

					if (0 != state)
						asio::detail::throw_error(asio::error::invalid_argument);

					/* <scheme>://<user>:<password>@<host>:<port>/<path>;<params>?<query>#<fragment> */

					std::string_view /*schema{ "http" }, */host, port, target{ "/" }/*, userinfo, path, query, fragment*/;

					//if (u.field_set & (1 << (int)cparser::url_fields::UF_SCHEMA))
					//	schema = { &uri[u.field_data[(int)cparser::url_fields::UF_SCHEMA].off],
					//	u.field_data[(int)cparser::url_fields::UF_SCHEMA].len };
					if (u.field_set & (1 << (int)cparser::url_fields::UF_HOST))
						host = { &uri[u.field_data[(int)cparser::url_fields::UF_HOST].off],
						u.field_data[(int)cparser::url_fields::UF_HOST].len };
					if (u.field_set & (1 << (int)cparser::url_fields::UF_PORT))
						port = { &uri[u.field_data[(int)cparser::url_fields::UF_PORT].off],
						u.field_data[(int)cparser::url_fields::UF_PORT].len };
					if (u.field_set & (1 << (int)cparser::url_fields::UF_PATH))
						target = { &uri[u.field_data[(int)cparser::url_fields::UF_PATH].off],
						uri.size() - u.field_data[(int)cparser::url_fields::UF_PATH].off };
					//	path = target = { &uri[u.field_data[(int)cparser::url_fields::UF_PATH].off],
					//	u.field_data[(int)cparser::url_fields::UF_PATH].len };
					//if (u.field_set & (1 << (int)cparser::url_fields::UF_USERINFO))
					//	userinfo = { &uri[u.field_data[(int)cparser::url_fields::UF_USERINFO].off],
					//	u.field_data[(int)cparser::url_fields::UF_USERINFO].len };
					//if (u.field_set & (1 << (int)cparser::url_fields::UF_QUERY))
					//	query = { &uri[u.field_data[(int)cparser::url_fields::UF_QUERY].off],
					//	u.field_data[(int)cparser::url_fields::UF_QUERY].len };
					//if (u.field_set & (1 << (int)cparser::url_fields::UF_FRAGMENT))
					//	fragment = { &uri[u.field_data[(int)cparser::url_fields::UF_FRAGMENT].off],
					//	u.field_data[(int)cparser::url_fields::UF_FRAGMENT].len };

					// Set up an HTTP GET request message
					req.method(verb::get);
					req.version(11);
					req.target(beast::string_view(target.data(), target.size()));
					req.set(field::server, BOOST_BEAST_VERSION_STRING);

					if (port.empty())
						req.set(field::host, host);
					else
						req.set(field::host, std::string(host) + ":" + std::string(port));

					if (host.empty())
						asio::detail::throw_error(asio::error::invalid_argument);
				}
			}
			catch (system_error & e)
			{
				ec = e.code();
			}
			return req;
		}

		/**
		 * @function : make A typical HTTP request struct from the uri
		 * You need to encode the "uri"(by url_encode) before calling this function
		 */
		template<class Body = string_body, class Fields = fields>
		inline request<Body, Fields> make_request(std::string_view uri)
		{
			error_code ec;
			request<Body, Fields> req = make_request<Body, Fields>(uri, ec);
			asio::detail::throw_error(ec);
			return req;
		}

		/**
		 * @function : make A typical HTTP request struct from the uri
		 * You need to encode the "target"(by url_encode) before calling this function
		 */
		template<class Body = string_body, class Fields = fields>
		inline request<Body, Fields> make_request(std::string_view host, std::string_view port,
			std::string_view target, verb method = verb::get, unsigned version = 11)
		{
			ASIO2_ASSERT(!has_unencode_char(target));
			request<Body, Fields> req;
			// Set up an HTTP GET request message
			req.method(method);
			req.version(version);
			req.target(target.empty() ? beast::string_view{ "/" } : beast::string_view(target.data(), target.size()));
			if (!port.empty() && port != "443" && port != "80")
				req.set(http::field::host, std::string(host) + ":" + std::string(port));
			else
				req.set(http::field::host, host);
			req.set(http::field::server, BOOST_BEAST_VERSION_STRING);
			return req;
		}

		template<class Body = string_body, class Fields = fields>
		inline response<Body, Fields> make_response(std::string_view uri, error_code& ec)
		{
			ec.clear();
			response_parser<Body> parser;
			parser.eager(true);
			parser.put(asio::buffer(uri), ec);
			response<Body, Fields> rep = parser.get();
			return rep;
		}

		template<class Body = string_body, class Fields = fields>
		inline response<Body, Fields> make_response(std::string_view uri)
		{
			error_code ec;
			response<Body, Fields> rep = make_response<Body, Fields>(uri, ec);
			asio::detail::throw_error(ec);
			return rep;
		}

		template<class Body = string_body, class Fields = fields>
		inline typename std::enable_if_t<std::is_same_v<Body, string_body>, response<Body, Fields>>
			make_response(status code, std::string_view body, unsigned version = 11)
		{
			response<Body, Fields> rep;
			rep.version(version);
			rep.set(field::server, BOOST_BEAST_VERSION_STRING);
			rep.result(code);
			rep.body() = body;
			rep.prepare_payload();
			return rep;
		}

		template<class Body = string_body, class Fields = fields>
		inline typename std::enable_if_t<std::is_same_v<Body, string_body>, response<Body, Fields>>
			make_response(unsigned code, std::string_view body, unsigned version = 11)
		{
			return make_response(http::int_to_status(code), body, version);
		}
	}

	template<typename, typename = void>
	struct is_http_message : std::false_type {};

	template<typename T>
	struct is_http_message<T, std::void_t<typename T::header_type, typename T::body_type,
		typename std::enable_if_t<std::is_same_v<T, message<
		T::header_type::is_request::value,
		typename T::body_type,
		typename T::header_type::fields_type>>>>> : std::true_type {};

	template<class T>
	inline constexpr bool is_http_message_v = is_http_message<T>::value;
}

#endif // !__ASIO2_HTTP_UTIL_HPP__

#endif
