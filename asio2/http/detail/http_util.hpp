/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

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
#include <asio2/http/multipart.hpp>

#include <asio2/util/string.hpp>

#ifdef BEAST_HEADER_ONLY
namespace beast::http
#else
namespace boost::beast::http
#endif
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
				else if (url[i] == static_cast<value_type>('+'))
					r += static_cast<rvalue_type>(' ');
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

			http::http_parser_ns::http_parser_url u;
			if (0 == http::http_parser_ns::http_parser_parse_url(url.data(), url.size(), 0, &u))
			{
				if (u.field_set & (1 << (int)http::http_parser_ns::url_fields::UF_PATH))
				{
					i = u.field_data[(int)http::http_parser_ns::url_fields::UF_PATH].off;
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
				if (std::isalnum(static_cast<unsigned char>(c)) || reserve.find(c) != std::string_view::npos)
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

				if (c != '%' && !std::isalnum(static_cast<unsigned char>(c)) &&
					reserve.find(c) == std::string_view::npos)
					return true;
			}
			return false;
		}

		template<typename = void>
		std::string_view url_to_host(std::string_view url)
		{
			http::http_parser_ns::http_parser_url u;
			if (0 != http::http_parser_ns::http_parser_parse_url(url.data(), url.size(), 0, &u))
				return std::string_view{};

			if (!(u.field_set & (1 << (int)http::http_parser_ns::url_fields::UF_HOST)))
				return std::string_view{};

			return std::string_view{ &url[u.field_data[(int)http::http_parser_ns::url_fields::UF_HOST].off],
				u.field_data[(int)http::http_parser_ns::url_fields::UF_HOST].len };
		}

		template<typename = void>
		std::string_view url_to_port(std::string_view url)
		{
			http::http_parser_ns::http_parser_url u;
			if (0 != http::http_parser_ns::http_parser_parse_url(url.data(), url.size(), 0, &u))
				return std::string_view{};

			if (u.field_set & (1 << (int)http::http_parser_ns::url_fields::UF_PORT))
				return std::string_view{ &url[u.field_data[(int)http::http_parser_ns::url_fields::UF_PORT].off],
				u.field_data[(int)http::http_parser_ns::url_fields::UF_PORT].len };

			if (u.field_set & (1 << (int)http::http_parser_ns::url_fields::UF_SCHEMA))
			{
				std::string schema(&url[u.field_data[(int)http::http_parser_ns::url_fields::UF_SCHEMA].off],
					u.field_data[(int)http::http_parser_ns::url_fields::UF_SCHEMA].len);
				std::transform(schema.begin(), schema.end(), schema.begin(), [](std::string::value_type c)
				{
					return std::tolower(c);
				});
				return (schema == "https" ? std::string_view{ "443" } : std::string_view{ "80" });
			}

			return std::string_view{ "80" };
		}

		template<typename = void>
		std::string_view url_to_path(std::string_view url)
		{
			http::http_parser_ns::http_parser_url u;
			if (0 != http::http_parser_ns::http_parser_parse_url(url.data(), url.size(), 0, &u))
				return std::string_view{};

			if (!(u.field_set & (1 << (int)http::http_parser_ns::url_fields::UF_PATH)))
				return std::string_view{};

			return std::string_view{ &url[u.field_data[(int)http::http_parser_ns::url_fields::UF_PATH].off],
				u.field_data[(int)http::http_parser_ns::url_fields::UF_PATH].len };
		}

		template<typename = void>
		std::string_view url_to_query(std::string_view url)
		{
			http::http_parser_ns::http_parser_url u;
			if (0 != http::http_parser_ns::http_parser_parse_url(url.data(), url.size(), 0, &u))
				return std::string_view{};

			if (!(u.field_set & (1 << (int)http::http_parser_ns::url_fields::UF_QUERY)))
				return std::string_view{};

			return std::string_view{ &url[u.field_data[(int)http::http_parser_ns::url_fields::UF_QUERY].off],
				u.field_data[(int)http::http_parser_ns::url_fields::UF_QUERY].len };
		}

		/**
		 * @function : make A typical HTTP request struct from the uri
		 * You need to encode the "uri"(by url_encode) before calling this function
		 */
		template<class Body = http::string_body, class Fields = http::fields>
		http::request_t<Body, Fields> make_request(std::string_view uri, error_code& ec)
		{
			http::request_t<Body, Fields> req;
			try
			{
				ec.clear();

				http_parser_ns::http_parser_url u;
				int state = http_parser_ns::http_parser_parse_url(uri.data(), uri.size(), 0, &u);

				// If a \r\n string is found, it is not a URL
				if (uri.find("\r\n") != std::string_view::npos && 0 != state)
				{
					http::request_parser<Body> parser;
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

					//if (u.field_set & (1 << (int)http_parser_ns::url_fields::UF_SCHEMA))
					//	schema = { &uri[u.field_data[(int)http_parser_ns::url_fields::UF_SCHEMA].off],
					//	u.field_data[(int)http_parser_ns::url_fields::UF_SCHEMA].len };
					if (u.field_set & (1 << (int)http_parser_ns::url_fields::UF_HOST))
						host = { &uri[u.field_data[(int)http_parser_ns::url_fields::UF_HOST].off],
						u.field_data[(int)http_parser_ns::url_fields::UF_HOST].len };
					if (u.field_set & (1 << (int)http_parser_ns::url_fields::UF_PORT))
						port = { &uri[u.field_data[(int)http_parser_ns::url_fields::UF_PORT].off],
						u.field_data[(int)http_parser_ns::url_fields::UF_PORT].len };
					if (u.field_set & (1 << (int)http_parser_ns::url_fields::UF_PATH))
						target = { &uri[u.field_data[(int)http_parser_ns::url_fields::UF_PATH].off],
						uri.size() - u.field_data[(int)http_parser_ns::url_fields::UF_PATH].off };
					//	path = target = { &uri[u.field_data[(int)http_parser_ns::url_fields::UF_PATH].off],
					//	u.field_data[(int)http_parser_ns::url_fields::UF_PATH].len };
					//if (u.field_set & (1 << (int)http_parser_ns::url_fields::UF_USERINFO))
					//	userinfo = { &uri[u.field_data[(int)http_parser_ns::url_fields::UF_USERINFO].off],
					//	u.field_data[(int)http_parser_ns::url_fields::UF_USERINFO].len };
					//if (u.field_set & (1 << (int)http_parser_ns::url_fields::UF_QUERY))
					//	query = { &uri[u.field_data[(int)http_parser_ns::url_fields::UF_QUERY].off],
					//	u.field_data[(int)http_parser_ns::url_fields::UF_QUERY].len };
					//if (u.field_set & (1 << (int)http_parser_ns::url_fields::UF_FRAGMENT))
					//	fragment = { &uri[u.field_data[(int)http_parser_ns::url_fields::UF_FRAGMENT].off],
					//	u.field_data[(int)http_parser_ns::url_fields::UF_FRAGMENT].len };

					// Set up an HTTP GET request message
					req.method(http::verb::get);
					req.version(11);
					req.target(beast::string_view(target.data(), target.size()));
					req.set(http::field::server, BEAST_VERSION_STRING);

					if (port.empty())
						req.set(http::field::host, host);
					else
						req.set(http::field::host, std::string(host) + ":" + std::string(port));

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
		template<class Body = http::string_body, class Fields = http::fields>
		inline http::request_t<Body, Fields> make_request(std::string_view uri)
		{
			error_code ec;
			http::request_t<Body, Fields> req = make_request<Body, Fields>(uri, ec);
			asio::detail::throw_error(ec);
			return req;
		}

		/**
		 * @function : make A typical HTTP request struct from the uri
		 * You need to encode the "target"(by url_encode) before calling this function
		 */
		template<typename String, typename StrOrInt, class Body = http::string_body, class Fields = http::fields>
		inline http::request_t<Body, Fields> make_request(String&& host, StrOrInt&& port,
			std::string_view target, http::verb method = http::verb::get, unsigned version = 11)
		{
			ASIO2_ASSERT(!has_unencode_char(target));
			http::request_t<Body, Fields> req;
			// Set up an HTTP GET request message
			req.method(method);
			req.version(version);
			req.target(target.empty() ? beast::string_view{ "/" } : beast::string_view(target.data(), target.size()));
			std::string sport = asio2::detail::to_string(std::forward<StrOrInt>(port));
			asio2::trim_both(sport);
			if (!sport.empty() && sport != "443" && sport != "80")
			{
				req.set(http::field::host, std::string(std::forward<String>(host)) + ":" + sport);
			}
			else
			{
				req.set(http::field::host, std::forward<String>(host));
			}
			req.set(http::field::server, BEAST_VERSION_STRING);
			return req;
		}

		template<class Body = http::string_body, class Fields = http::fields>
		inline http::response_t<Body, Fields> make_response(std::string_view uri, error_code& ec)
		{
			ec.clear();
			http::response_parser<Body> parser;
			parser.eager(true);
			parser.put(asio::buffer(uri), ec);
			http::response_t<Body, Fields> rep = parser.get();
			return rep;
		}

		template<class Body = http::string_body, class Fields = http::fields>
		inline http::response_t<Body, Fields> make_response(std::string_view uri)
		{
			error_code ec;
			http::response_t<Body, Fields> rep = make_response<Body, Fields>(uri, ec);
			asio::detail::throw_error(ec);
			return rep;
		}

		template<class Body = http::string_body, class Fields = http::fields>
		inline typename std::enable_if_t<std::is_same_v<Body, http::string_body>, http::response_t<Body, Fields>>
			make_response(http::status code, std::string_view body, unsigned version = 11)
		{
			http::response_t<Body, Fields> rep;
			rep.version(version);
			rep.set(http::field::server, BEAST_VERSION_STRING);
			rep.result(code);
			rep.body() = body;
			rep.prepare_payload();
			return rep;
		}

		template<class Body = http::string_body, class Fields = http::fields>
		inline typename std::enable_if_t<std::is_same_v<Body, http::string_body>, http::response_t<Body, Fields>>
			make_response(unsigned code, std::string_view body, unsigned version = 11)
		{
			return make_response(http::int_to_status(code), body, version);
		}

		template<typename = void>
		inline bool url_match(std::string_view pattern, std::string_view url)
		{
			if (pattern == "/*")
				return true;
			std::vector<std::string_view> fragments = asio2::split(pattern, "*");
			std::size_t index = 0;
			while (!url.empty())
			{
				if (index == fragments.size())
					return (pattern.back() == '*');
				std::string_view fragment = fragments[index++];
				if (fragment.empty())
					continue;
				while (fragment.size() > static_cast<std::string_view::size_type>(1) && fragment.back() == '/')
				{
					fragment.remove_suffix(1);
				}
				std::size_t pos = url.find(fragment);
				if (pos == std::string_view::npos)
					return false;
				url = url.substr(pos + fragment.size());
			}
			return true;
		}

		template<typename = void>
		inline std::string error_page(http::status result, std::string desc = {})
		{
			std::string_view reason = http::obsolete_reason(result);
			std::string content;
			if (desc.empty())
				content.reserve(reason.size() * 2 + 67);
			else
				content.reserve(reason.size() * 2 + 67 + desc.size() + 21);
			content += "<html><head><title>";
			content += reason;
			content += "</title></head><body><h1>";
			content += std::to_string(asio2::detail::enum_to_int(result));
			content += " ";
			content += reason;
			content += "</h1>";
			if (!desc.empty())
			{
				content += "<p>Description : ";
				content += std::move(desc);
				content += "</p>";
			}
			content += "</body></html>";
			return content;
		}

		/**
		 * @function : Returns `true` if the HTTP message's Content-Type is "multipart/form-data";
		 */
		template<bool isRequest, class Body, class Fields>
		inline bool has_multipart(const http::message<isRequest, Body, Fields>& msg)
		{
			return (asio2::ifind(msg[http::field::content_type], "multipart/form-data") != std::string_view::npos);
		}

		/**
		 * @function : Get the "multipart/form-data" body content.
		 */
		template<bool isRequest, class Body, class Fields, class String = std::string_view>
		inline basic_multipart_fields<String> multipart(const http::message<isRequest, Body, Fields>& msg)
		{
			return multipart_parser_execute(msg);
		}
	}

	template<typename, typename = void>
	struct is_http_message : std::false_type {};

	template<typename T>
	struct is_http_message<T, std::void_t<typename T::header_type, typename T::body_type,
		typename std::enable_if_t<std::is_same_v<T, http::message<
		T::header_type::is_request::value,
		typename T::body_type,
		typename T::header_type::fields_type>>>>> : std::true_type {};

	template<class T>
	inline constexpr bool is_http_message_v = is_http_message<T>::value;
}

#ifdef BEAST_HEADER_ONLY
namespace beast::websocket
#else
namespace boost::beast::websocket
#endif
{
	enum class frame
	{
		/// 
		unknown,

		/// A message frame was received
		message,

		/// A ping frame was received
		ping,

		/// A pong frame was received
		pong,

		/// http is upgrade to websocket
		open,

		/// A close frame was received
		close
	};
}

#endif // !__ASIO2_HTTP_UTIL_HPP__
