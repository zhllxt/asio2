/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 */

#ifndef ASIO_STANDALONE

#ifndef __ASIO2_HTTP_UTIL_HPP__
#define __ASIO2_HTTP_UTIL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cctype>
#include <sstream>

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

		#if defined(__GNUC__) || defined(__GNUG__)
			__attribute__((unused))
		#endif
		bool url_decode(std::string & url)
		{
			std::size_t index = 0, size = url.size();
			for (std::size_t i = 0; i < size; ++i)
			{
				if (url[i] == '%')
				{
					if (i + 3 <= size)
					{
						using type = std::string::value_type;

						type h = url[i + 1];
						type l = url[i + 2];

						if /**/ (h >= '0' && h <= '9') h = type(h - '0'     );
						else if (h >= 'a' && h <= 'f') h = type(h - 'a' + 10);
						else if (h >= 'A' && h <= 'F') h = type(h - 'A' + 10);
						else return false;

						if /**/ (l >= '0' && l <= '9') l = type(l - '0'     );
						else if (l >= 'a' && l <= 'f') l = type(l - 'a' + 10);
						else if (l >= 'A' && l <= 'F') l = type(l - 'A' + 10);
						else return false;

						url[index++] = static_cast<std::string::value_type>(h * 16 + l);
						i += 2;
					}
					else
						return false;
				}
				else if (url[i] == '+')
					url[index++] = ' ';
				else
					url[index++] = url[i];
			}
			url.resize(index);
			return true;
		}

		#if defined(__GNUC__) || defined(__GNUG__)
			__attribute__((unused))
		#endif
		std::string url_encode(std::string_view url)
		{
			std::string s;
			std::size_t size = url.size();
			for (std::size_t i = 0; i < size; i++)
			{
				using type = std::string_view::value_type;
				type c = url[i];
				if (std::isalnum(static_cast<unsigned char>(c)) ||
					(c == '-') || (c == '_') || (c == '.') || (c == '~'))
					s += c;
				else if (c == ' ')
					s += '+';
				else
				{
					s += '%';
					type h = type(static_cast<unsigned char>(c) >> 4);
					s += h > 9 ? type(h + 55) : type(h + 48);
					type l = type(static_cast<unsigned char>(c) % 16);
					s += l > 9 ? type(l + 55) : type(l + 48);
				}
			}
			return s;
		}
		
		bool url_to_hostport(std::string_view url, std::string_view& host, std::string_view& port, error_code& ec)
		{
			try
			{
				ec.clear();

				http::cparser::http_parser_url u;
				if (0 != http::cparser::http_parser_parse_url(url.data(), url.size(), 0, &u))
					asio::detail::throw_error(asio::error::invalid_argument);

				std::string_view schema{ "http" };

				if (u.field_set & (1 << http::cparser::UF_SCHEMA))
					schema = { &url[u.field_data[http::cparser::UF_SCHEMA].off],u.field_data[http::cparser::UF_SCHEMA].len };
				if (u.field_set & (1 << http::cparser::UF_HOST))
					host = { &url[u.field_data[http::cparser::UF_HOST].off],u.field_data[http::cparser::UF_HOST].len };
				if (u.field_set & (1 << http::cparser::UF_PORT))
					port = { &url[u.field_data[http::cparser::UF_PORT].off],u.field_data[http::cparser::UF_PORT].len };

				if (port.empty())
				{
					std::string h(schema);
					std::transform(h.begin(), h.end(), h.begin(), [](std::string::value_type c) { return std::tolower(c); });
					port = h == "https" ? "443" : "80";
				}
				return true;
			}
			catch (system_error & e)
			{
				ec = e.code();
			}
			return false;
		}

		bool url_to_hostport(std::string_view url, std::string_view& host, std::string_view& port)
		{
			error_code ec;
			bool ret = url_to_hostport(url, host, port, ec);
			asio::detail::throw_error(ec);
			return ret;
		}

		template<class Body = string_body, class Fields = fields>
		request<Body, Fields> make_request(std::string_view uri, error_code& ec)
		{
			request<Body, Fields> req;
			try
			{
				ec.clear();

				// If a space or \r\n character is found, it is not a URL
				if (uri.find(' ') != std::string_view::npos || uri.find("\r\n") != std::string_view::npos)
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
					cparser::http_parser_url u;
					if (0 != cparser::http_parser_parse_url(uri.data(), uri.size(), 0, &u))
						asio::detail::throw_error(asio::error::invalid_argument);

					/* <scheme>://<user>:<password>@<host>:<port>/<path>;<params>?<query>#<fragment> */

					std::string_view /*schema{ "http" }, */host, port, target{ "/" }/*, userinfo, path, query, fragment*/;

					//if (u.field_set & (1 << cparser::UF_SCHEMA))
					//	schema = { &uri[u.field_data[cparser::UF_SCHEMA].off],u.field_data[cparser::UF_SCHEMA].len };
					if (u.field_set & (1 << cparser::UF_HOST))
						host = { &uri[u.field_data[cparser::UF_HOST].off],u.field_data[cparser::UF_HOST].len };
					if (u.field_set & (1 << cparser::UF_PORT))
						port = { &uri[u.field_data[cparser::UF_PORT].off],u.field_data[cparser::UF_PORT].len };
					if (u.field_set & (1 << cparser::UF_PATH))
						target = { &uri[u.field_data[cparser::UF_PATH].off],uri.size() - u.field_data[cparser::UF_PATH].off };
					//	path = target = { &uri[u.field_data[cparser::UF_PATH].off],u.field_data[cparser::UF_PATH].len };
					//if (u.field_set & (1 << cparser::UF_USERINFO))
					//	userinfo = { &uri[u.field_data[cparser::UF_USERINFO].off],u.field_data[cparser::UF_USERINFO].len };
					//if (u.field_set & (1 << cparser::UF_QUERY))
					//	query = { &uri[u.field_data[cparser::UF_QUERY].off],u.field_data[cparser::UF_QUERY].len };
					//if (u.field_set & (1 << cparser::UF_FRAGMENT))
					//	fragment = { &uri[u.field_data[cparser::UF_FRAGMENT].off],u.field_data[cparser::UF_FRAGMENT].len };

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

		template<class Body = string_body, class Fields = fields>
		inline request<Body, Fields> make_request(std::string_view uri)
		{
			error_code ec;
			request<Body, Fields> req = make_request<Body, Fields>(uri, ec);
			asio::detail::throw_error(ec);
			return req;
		}

		template<class Body = string_body, class Fields = fields>
		inline request<Body, Fields> make_request(std::string_view host, std::string_view port,
			std::string_view target, verb method = verb::get, unsigned version = 11)
		{
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
			make_response(status code, std::string_view body)
		{
			response<Body, Fields> rep;
			rep.version(11);
			rep.set(field::server, BOOST_BEAST_VERSION_STRING);
			rep.result(code);
			rep.body() = body;
			rep.prepare_payload();
			return rep;
		}

		template<class Body = string_body, class Fields = fields>
		inline typename std::enable_if_t<std::is_same_v<Body, string_body>, response<Body, Fields>>
			make_response(unsigned code, std::string_view body)
		{
			return make_response(http::int_to_status(code), body);
		}
	}
}

#endif // !__ASIO2_HTTP_UTIL_HPP__

#endif
