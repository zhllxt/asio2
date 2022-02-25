/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_HTTP_URL_HPP__
#define __ASIO2_HTTP_URL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/http/detail/http_util.hpp>

#ifdef BEAST_HEADER_ONLY
namespace bho::beast::http
#else
namespace boost::beast::http
#endif
{
	/**
	 * The object wrapped for a url string like "http://www.github.com"
	 * <scheme>://<user>:<password>@<host>:<port>/<path>;<params>?<query>#<fragment>
	 */
	class url
	{
	public:
		/**
		 * @constructor
		 * if the 'str' has unencoded char, it will be encoded automatically, otherwise
		 * the url parsing will be failed.
		 */
		url(std::string str) : string_(std::move(str))
		{
			std::memset((void*)(&parser_), 0, sizeof(http::parses::http_parser_url));

			if (http::has_unencode_char(string_))
			{
				string_ = http::url_encode(string_);
			}

			if (!string_.empty())
			{
				http::parses::http_parser_parse_url(string_.data(), string_.size(), 0, &parser_);
			}
		}

		/**
		 * @constructor
		 * if the 'str' has unencoded char, it will be encoded automatically, otherwise
		 * the url parsing will be failed.
		 */
		url(std::string str, error_code& ec) : string_(std::move(str))
		{
			ec = asio::error::invalid_argument;

			std::memset((void*)(&parser_), 0, sizeof(http::parses::http_parser_url));

			if (http::has_unencode_char(string_))
			{
				string_ = http::url_encode(string_);
			}

			if (!string_.empty())
			{
				if (0 == http::parses::http_parser_parse_url(string_.data(), string_.size(), 0, &parser_))
					ec.clear();
			}
		}

		url(url&&) noexcept = default;
		url(url const&) = default;
		url& operator=(url&&) noexcept = default;
		url& operator=(url const&) = default;

		/**
		 * @function : Gets the content of the "schema" section, maybe empty
		 * The return value is usually http or https
		 */
		inline std::string_view schema() noexcept
		{
			return this->field(http::parses::url_fields::UF_SCHEMA);
		}

		/**
		 * @function : Gets the content of the "host" section, maybe empty
		 */
		inline std::string_view host() noexcept
		{
			return this->field(http::parses::url_fields::UF_HOST);
		}

		inline std::string_view default_port() noexcept
		{
			std::string_view schema = this->schema();
			if (asio2::iequals(schema, "http"))
				return std::string_view{ "80" };
			if (asio2::iequals(schema, "https"))
				return std::string_view{ "443" };
			return std::string_view{ "80" };
		}

		/**
		 * @function : Gets the content of the "port" section
		 */
		inline std::string_view port() noexcept
		{
			std::string_view p = this->field(http::parses::url_fields::UF_PORT);
			if (p.empty())
				return this->default_port();
			return p;
		}

		/**
		 * @function : Gets the content of the "path" section
		 * the return value maybe has undecoded char, you can use http::url_decode(...) to decoded it.
		 */
		inline std::string_view path() noexcept
		{
			std::string_view p = this->field(http::parses::url_fields::UF_PATH);
			if (p.empty())
				return std::string_view{ "/" };
			return p;
		}

		/**
		 * @function : Gets the content of the "query" section, maybe empty
		 * the return value maybe has undecoded char, you can use http::url_decode(...) to decoded it.
		 */
		inline std::string_view query() noexcept
		{
			return this->field(http::parses::url_fields::UF_QUERY);
		}

		/**
		 * @function : Gets the "target", which composed by path and query
		 * the return value maybe has undecoded char, you can use http::url_decode(...) to decoded it.
		 */
		inline std::string_view target() noexcept
		{
			if (parser_.field_set & (1 << (int)http::parses::url_fields::UF_PATH))
			{
				return std::string_view{ &string_[
					parser_.field_data[(int)http::parses::url_fields::UF_PATH].off],
					string_.size() -
					parser_.field_data[(int)http::parses::url_fields::UF_PATH].off };
			}

			return std::string_view{ "/" };
		}

		/**
		 * @function : Gets the content of the specific section, maybe empty
		 */
		inline std::string_view field(http::parses::url_fields f) noexcept
		{
			if (!(parser_.field_set & (1 << int(f))))
				return std::string_view{};

			return std::string_view{ &string_[parser_.field_data[int(f)].off], parser_.field_data[int(f)].len };
		}

		inline http::parses::http_parser_url & parser() noexcept { return this->parser_; }
		inline std::string                   & string() noexcept { return this->string_; }

	protected:
		http::parses::http_parser_url         parser_;
		std::string                           string_;
	};
}

#endif // !__ASIO2_HTTP_URL_HPP__
