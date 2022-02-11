/*
 * COPYRIGHT (C) 2017-2021, zhllxt
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

#include <asio2/3rd/asio.hpp>
#include <asio2/3rd/beast.hpp>
#include <asio2/base/error.hpp>

#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/buffer_wrap.hpp>
#include <asio2/http/detail/http_parser.h>
#include <asio2/http/detail/mime_types.hpp>
#include <asio2/http/multipart.hpp>

#include <asio2/util/string.hpp>

#ifdef BEAST_HEADER_ONLY
namespace bho::beast::http
#else
namespace boost::beast::http
#endif
{
	namespace
	{
		// Percent-encoding : https://en.wikipedia.org/wiki/Percent-encoding

		// ---- RFC 3986 section 2.2 Reserved Characters (January 2005)
		// !#$&'()*+,/:;=?@[]
		// 
		// ---- RFC 3986 section 2.3 Unreserved Characters (January 2005)
		// ABCDEFGHIJKLMNOPQRSTUVWXYZ
		// abcdefghijklmnopqrstuvwxyz
		// 0123456789-_.~
		// 
		// 
		// http://www.baidu.com/query?key=x!#$&'()*+,/:;=?@[ ]-_.~%^{}\"|<>`\\y
		// 
		// C# System.Web.HttpUtility.UrlEncode
		// http%3a%2f%2fwww.baidu.com%2fquery%3fkey%3dx!%23%24%26%27()*%2b%2c%2f%3a%3b%3d%3f%40%5b+%5d-_.%7e%25%5e%7b%7d%22%7c%3c%3e%60%5cy
		// 
		// java.net.URLEncoder.encode
		// http%3A%2F%2Fwww.baidu.com%2Fquery%3Fkey%3Dx%21%23%24%26%27%28%29*%2B%2C%2F%3A%3B%3D%3F%40%5B+%5D-_.%7E%25%5E%7B%7D%5C%22%7C%3C%3E%60%5C%5Cy
		// 
		// postman
		// 
		// 
		// asp
		// http://127.0.0.1/index.asp?id=x!#$&name='()*+,/:;=?@[ ]-_.~%^{}\"|<>`\\y
		// http://127.0.0.1/index.asp?id=x%21%23%24&name=%27%28%29*%2B%2C%2F%3A%3B=?%40%5B%20%5D-_.%7E%25%5E%7B%7D%22%7C%3C%3E%60%5Cy
		// the character    &=?    can't be encoded, otherwise the result of queryString is wrong.
		// <%
		//   id=request.queryString("id")
		//   response.write "id=" & id
		//   response.write "</br>"
		//   name=request.queryString("name")
		//   response.write "name=" & name
		// %>
		// 
		// 
		// http%+y%2f%2fwww.baidu.com%2fquery%3fkey%3dx!%23%24%26%27()*%2b%2c%2f%3a%3b%3d%3f%40%5b+%5d-_.%7e%25%5e%7b%7d%22%7c%3c%3e%60%+5
		// 
		// C# System.Web.HttpUtility.UrlDecode
		// http% y//www.baidu.com/query?key=x!#$&'()*+,/:;=?@[ ]-_.~%^{}"|<>`% 5
		//

		static constexpr char unreserved_char[] = {
			//0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
			  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0
			  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1
			  0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, // 2
			  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1, // 3
			  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 4
			  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, // 5
			  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 6
			  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, // 7
			  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 8
			  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 9
			  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // A
			  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // B
			  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // C
			  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // D
			  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // E
			  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  // F
		};

		template<
			class CharT = char,
			class Traits = std::char_traits<CharT>,
			class Allocator = std::allocator<CharT>
		>
		std::basic_string<CharT, Traits, Allocator> url_decode(std::string_view url)
		{
			using size_type   = typename std::string_view::size_type;
			using value_type  = typename std::string_view::value_type;
			using rvalue_type = typename std::basic_string<CharT, Traits, Allocator>::value_type;

			std::basic_string<CharT, Traits, Allocator> r;
			r.reserve(url.size());

			for (size_type i = 0; i < url.size(); ++i)
			{
				value_type c = url[i];

				if (c == '%')
				{
					if (i + 3 <= url.size())
					{
						value_type h = url[i + 1];
						value_type l = url[i + 2];

						bool f1 = false, f2 = false;

						if /**/ (h >= '0' && h <= '9') { f1 = true; h = value_type(h - '0'     ); }
						else if (h >= 'a' && h <= 'f') { f1 = true; h = value_type(h - 'a' + 10); }
						else if (h >= 'A' && h <= 'F') { f1 = true; h = value_type(h - 'A' + 10); }

						if /**/ (l >= '0' && l <= '9') { f2 = true; l = value_type(l - '0'     ); }
						else if (l >= 'a' && l <= 'f') { f2 = true; l = value_type(l - 'a' + 10); }
						else if (l >= 'A' && l <= 'F') { f2 = true; l = value_type(l - 'A' + 10); }

						if (f1 && f2)
						{
							r += static_cast<rvalue_type>(h * 16 + l);
							i += 2;
						}
						else
						{
							r += static_cast<rvalue_type>(c);
						}
					}
					else
					{
						r += static_cast<rvalue_type>(c);
					}
				}
				else if (c == '+')
				{
					r += static_cast<rvalue_type>(' ');
				}
				else
				{
					r += static_cast<rvalue_type>(c);
				}
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
			using size_type   = typename std::string_view::size_type;
			using value_type  = typename std::string_view::value_type;
			using rvalue_type = typename std::basic_string<CharT, Traits, Allocator>::value_type;

			std::basic_string<CharT, Traits, Allocator> r;
			r.reserve(url.size() * 2);

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

			for (; i < url.size(); ++i)
			{
				unsigned char c = static_cast<unsigned char>(url[i]);

				if (/*std::isalnum(c) || */unreserved_char[c])
				{
					r += static_cast<rvalue_type>(c);
				}
				else
				{
					r += static_cast<rvalue_type>('%');
					rvalue_type h = rvalue_type(c >> 4);
					r += h > rvalue_type(9) ? rvalue_type(h + 55) : rvalue_type(h + 48);
					rvalue_type l = rvalue_type(c % 16);
					r += l > rvalue_type(9) ? rvalue_type(l + 55) : rvalue_type(l + 48);
				}
			}
			return r;
		}

		template<typename = void>
		bool has_unencode_char(std::string_view uri) noexcept
		{
			using size_type   = typename std::string_view::size_type;
			using value_type  = typename std::string_view::value_type;
			using rvalue_type = typename std::string::value_type;

			size_type i = 0;

			http::http_parser_ns::http_parser_url u;
			if (0 == http::http_parser_ns::http_parser_parse_url(uri.data(), uri.size(), 0, &u))
			{
				if (u.field_set & (1 << (int)http::http_parser_ns::url_fields::UF_PATH))
				{
					i = u.field_data[(int)http::http_parser_ns::url_fields::UF_PATH].off;
					if (i < uri.size() && uri[i] == static_cast<value_type>('/'))
					{
						i++;
					}
				}
			}

			for (; i < uri.size(); ++i)
			{
				unsigned char c = static_cast<unsigned char>(uri[i]);

				if (c == static_cast<unsigned char>('%'))
				{
					if (i + 3 <= uri.size())
					{
						value_type h = uri[i + 1];
						value_type l = uri[i + 2];

						if /**/ (h >= '0' && h <= '9') {}
						else if (h >= 'a' && h <= 'f') {}
						else if (h >= 'A' && h <= 'F') {}
						else { return true; }

						if /**/ (l >= '0' && l <= '9') {}
						else if (l >= 'a' && l <= 'f') {}
						else if (l >= 'A' && l <= 'F') {}
						else { return true; }

						i += 2;
					}
					else
					{
						return true;
					}
				}
				else if (!unreserved_char[c])
				{
					return true;
				}
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
					return static_cast<std::string::value_type>(std::tolower(c));
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
		 */
		template<class Body = http::string_body, class Fields = http::fields>
		http::request<Body, Fields> make_request(std::string_view uri, error_code& ec)
		{
			http::request<Body, Fields> req;
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
					parser.put(::asio::buffer(uri), ec);
					req = parser.get();
					::asio::detail::throw_error(ec);
				}
				// It is a URL
				else
				{
					std::string encoded;
					if (has_unencode_char(uri))
					{
						encoded = url_encode(uri);
						uri = encoded;
					}

					if (0 != state)
						::asio::detail::throw_error(::asio::error::invalid_argument);

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
						::asio::detail::throw_error(::asio::error::invalid_argument);
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
		 */
		template<class Body = http::string_body, class Fields = http::fields>
		inline http::request<Body, Fields> make_request(std::string_view uri)
		{
			error_code ec;
			http::request<Body, Fields> req = make_request<Body, Fields>(uri, ec);
			::asio::detail::throw_error(ec);
			return req;
		}

		/**
		 * @function : make A typical HTTP request struct from the uri
		 */
		template<typename String, typename StrOrInt, class Body = http::string_body, class Fields = http::fields>
		inline http::request<Body, Fields> make_request(String&& host, StrOrInt&& port,
			std::string_view target, http::verb method = http::verb::get, unsigned version = 11)
		{
			std::string encoded;
			if (has_unencode_char(target))
			{
				encoded = url_encode(target);
				target = encoded;
			}

			http::request<Body, Fields> req;
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
		inline http::response<Body, Fields> make_response(std::string_view uri, error_code& ec)
		{
			ec.clear();
			http::response_parser<Body> parser;
			parser.eager(true);
			parser.put(::asio::buffer(uri), ec);
			http::response<Body, Fields> rep = parser.get();
			return rep;
		}

		template<class Body = http::string_body, class Fields = http::fields>
		inline http::response<Body, Fields> make_response(std::string_view uri)
		{
			error_code ec;
			http::response<Body, Fields> rep = make_response<Body, Fields>(uri, ec);
			::asio::detail::throw_error(ec);
			return rep;
		}

		template<class Body = http::string_body, class Fields = http::fields>
		inline typename std::enable_if_t<std::is_same_v<Body, http::string_body>, http::response<Body, Fields>>
			make_response(http::status code, std::string_view body, unsigned version = 11)
		{
			http::response<Body, Fields> rep;
			rep.version(version);
			rep.set(http::field::server, BEAST_VERSION_STRING);
			rep.result(code);
			rep.body() = body;
			rep.prepare_payload();
			return rep;
		}

		template<class Body = http::string_body, class Fields = http::fields>
		inline typename std::enable_if_t<std::is_same_v<Body, http::string_body>, http::response<Body, Fields>>
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
			content += std::to_string(asio2::detail::to_underlying(result));
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
		template<class HttpMessage>
		inline bool has_multipart(const HttpMessage& msg) noexcept
		{
			return (asio2::ifind(msg[http::field::content_type], "multipart/form-data") != std::string_view::npos);
		}

		/**
		 * @function : Get the "multipart/form-data" body content.
		 */
		template<class HttpMessage, class String = std::string_view>
		inline basic_multipart_fields<String> multipart(const HttpMessage& msg)
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
namespace bho::beast::websocket
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
