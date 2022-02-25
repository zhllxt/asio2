/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_HTTP_MAKE_HPP__
#define __ASIO2_HTTP_MAKE_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/http/request.hpp>
#include <asio2/http/response.hpp>

#ifdef BEAST_HEADER_ONLY
namespace bho::beast::http
#else
namespace boost::beast::http
#endif
{
	/**
	 * @function : make A typical HTTP request struct from the uri
	 * if the string is a url, it must start with http or https,
	 * eg : the url must be "https://www.github.com", the url can't be "www.github.com"
	 */
	template<class String>
	http::web_request make_request(String&& uri, error_code& ec)
	{
		http::web_request req;
		try
		{
			ec.clear();

			req.url() = http::url{ asio2::detail::to_string(std::forward<String>(uri)), ec };

			if (req.url().string().empty())
				::asio::detail::throw_error(::asio::error::invalid_argument);

			// If a \r\n string is found, it is not a URL
			if (ec && req.url().string().find("\r\n") != std::string::npos)
			{
				http::request_parser<http::string_body> parser;
				parser.eager(true);
				parser.put(::asio::buffer(req.url().string()), ec);
				req = parser.get();
				::asio::detail::throw_error(ec);
			}
			// It is a URL
			else
			{
				if (ec)
					::asio::detail::throw_error(::asio::error::invalid_argument);

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
					::asio::detail::throw_error(::asio::error::invalid_argument);

				if (!port.empty() && port != "443" && port != "80")
					req.set(http::field::host, std::string(host) + ":" + std::string(port));
				else
					req.set(http::field::host, host);
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
	template<class String>
	inline http::web_request make_request(String&& uri)
	{
		error_code ec;
		http::web_request req = make_request(std::forward<String>(uri), ec);
		::asio::detail::throw_error(ec);
		return req;
	}

	/**
	 * @function : make A typical HTTP request struct from the uri
	 */
	template<typename String, typename StrOrInt>
	inline http::web_request make_request(String&& host, StrOrInt&& port,
		std::string_view target, http::verb method = http::verb::get, unsigned version = 11)
	{
		http::web_request req;

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
		//rep.set(http::field::server, BEAST_VERSION_STRING);
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
}

#endif // !__ASIO2_HTTP_MAKE_HPP__
