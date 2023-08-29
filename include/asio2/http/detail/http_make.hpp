/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_HTTP_MAKE_HPP__
#define __ASIO2_HTTP_MAKE_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/http/request.hpp>
#include <asio2/http/response.hpp>

#ifdef ASIO2_HEADER_ONLY
namespace bho::beast::http
#else
namespace boost::beast::http
#endif
{
	/**
	 * @brief make A typical HTTP request struct from the uri
	 * if the string is a url, it must start with http or https,
	 * eg : the url must be "https://www.github.com", the url can't be "www.github.com"
	 */
	template<class String>
	http::web_request make_request(String&& uri)
	{
		http::web_request req;

		std::string url = asio2::detail::to_string(std::forward<String>(uri));

		// if uri is empty, break
		if (url.empty())
		{
			asio2::set_last_error(::asio::error::invalid_argument);
			return req;
		}

		asio2::clear_last_error();

		bool has_crlf = (url.find("\r\n") != std::string::npos);

		http::parses::http_parser_url& url_parser = req.url().parser();

		int f = http::parses::http_parser_parse_url(url.data(), url.size(), 0, std::addressof(url_parser));

		// If \r\n string is found, and parse the uri failed, it means that the uri is not a URL.
		if (has_crlf && f != 0)
		{
			http::request_parser<http::string_body> req_parser;
			req_parser.eager(true);
			req_parser.put(::asio::buffer(url), asio2::get_last_error());
			req = req_parser.get();
			req.url().reset(std::string(req.target()));
		}
		// It is a URL
		else
		{
			if (f != 0)
				return req;

			if (http::has_unencode_char(url))
			{
				req.url().reset(http::url_encode(url));
			}
			else
			{
				req.url().string() = std::move(url);
			}

			if (asio2::get_last_error())
				return req;

			/* <scheme>://<user>:<password>@<host>:<port>/<path>;<params>?<query>#<fragment> */

			std::string_view host   = req.url().host();
			std::string_view port   = req.url().port();
			std::string_view target = req.url().target();

			// Set up an HTTP GET request message
			req.method(http::verb::get);
			req.version(11);
			req.target(beast::string_view(target.data(), target.size()));
			//req.set(http::field::server, BEAST_VERSION_STRING);

			if (host.empty())
			{
				asio2::set_last_error(::asio::error::invalid_argument);
				return req;
			}

			if (!port.empty() && port != "443" && port != "80")
				req.set(http::field::host, std::string(host) + ":" + std::string(port));
			else
				req.set(http::field::host, host);
		}

		return req;
	}

	/**
	 * @brief make A typical HTTP request struct from the uri
	 */
	template<typename String, typename StrOrInt>
	inline http::web_request make_request(String&& host, StrOrInt&& port,
		std::string_view target, http::verb method = http::verb::get, unsigned version = 11)
	{
		http::web_request req;

		asio2::clear_last_error();

		std::string h = asio2::detail::to_string(std::forward<String>(host));
		asio2::trim_both(h);

		std::string p = asio2::detail::to_string(std::forward<StrOrInt>(port));
		asio2::trim_both(p);

		std::string_view schema{ h.data(), (std::min<std::string_view::size_type>)(4, h.size()) };

		std::string url;
		if (!asio2::iequals(schema, "http"))
		{
			if /**/ (p == "80")
				url += "http://";
			else if (p == "443")
				url += "https://";
			else
				url += "http://";
		}
		url += h;
		if (!p.empty() && p != "443" && p != "80")
		{
			url += ":";
			url += p;
		}
		url += target;

		req.url() = http::url{ std::move(url) };

		// if url is invalid, break
		if (asio2::get_last_error())
			return req;

		// Set up an HTTP GET request message
		req.method(method);
		req.version(version);
		if (target.empty())
		{
			req.target(beast::string_view{ "/" });
		}
		else
		{
			if (http::has_unencode_char(target, 1))
			{
				std::string encoded = http::url_encode(target);
				req.target(beast::string_view(encoded.data(), encoded.size()));
			}
			else
			{
				req.target(beast::string_view(target.data(), target.size()));
			}
		}
		if (!p.empty() && p != "443" && p != "80")
		{
			req.set(http::field::host, h + ":" + p);
		}
		else
		{
			req.set(http::field::host, std::move(h));
		}
		//req.set(http::field::server, BEAST_VERSION_STRING);

		return req;
	}

	template<class Body = http::string_body, class Fields = http::fields>
	inline http::response<Body, Fields> make_response(std::string_view uri)
	{
		asio2::clear_last_error();
		http::response_parser<Body> rep_parser;
		rep_parser.eager(true);
		rep_parser.put(::asio::buffer(uri), asio2::get_last_error());
		http::response<Body, Fields> rep = rep_parser.get();
		return rep;
	}

	template<class Body = http::string_body, class Fields = http::fields>
	inline typename std::enable_if_t<std::is_same_v<Body, http::string_body>, http::response<Body, Fields>>
		make_response(http::status code, std::string_view body, unsigned version = 11)
	{
		http::response<Body, Fields> rep;
		rep.version(version);
		//rep.set(http::field::server, BEAST_VERSION_STRING);
		rep.result(code);
		rep.body() = body;
		http::try_prepare_payload(rep);
		return rep;
	}

	template<class Body = http::string_body, class Fields = http::fields>
	inline typename std::enable_if_t<std::is_same_v<Body, http::string_body>, http::response<Body, Fields>>
		make_response(unsigned code, std::string_view body, unsigned version = 11)
	{
		return make_response(http::int_to_status(code), body, version);
	}
}

#endif // !__ASIO2_HTTP_MAKE_HPP__
