/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_HTTP_URL_HPP__
#define __ASIO2_HTTP_URL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/http/detail/http_util.hpp>

#ifdef ASIO2_HEADER_ONLY
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
		 * @brief constructor
		 * if the 'str' has unencoded char, it will be encoded automatically, otherwise
		 * the url parsing will be failed.
		 */
		url(std::string str) : string_(std::move(str))
		{
			std::memset((void*)(std::addressof(parser_)), 0, sizeof(http::parses::http_parser_url));

			if (!string_.empty())
			{
				bool has_unencode_ch = http::has_unencode_char(string_);

				if (has_unencode_ch)
				{
					string_ = http::url_encode(string_);
				}

				if (0 != http::parses::http_parser_parse_url(string_.data(), string_.size(), 0, std::addressof(parser_)))
				{
					if (has_unencode_ch)
					{
						string_ = http::url_decode(string_);
					}

					asio2::set_last_error(asio::error::invalid_argument);
				}
			}
		}

		url(url&&) noexcept = default;
		url(url const&) = default;
		url& operator=(url&&) noexcept = default;
		url& operator=(url const&) = default;

		/**
		 * @brief Gets the content of the "schema" section, maybe empty
		 * The return value is usually http or https
		 */
		inline std::string_view get_schema() noexcept
		{
			return this->field(http::parses::url_fields::UF_SCHEMA);
		}

		/**
		 * @brief Gets the content of the "schema" section, maybe empty
		 * The return value is usually http or https
		 * same as get_schema
		 */
		inline std::string_view schema() noexcept
		{
			return this->get_schema();
		}

		/**
		 * @brief Gets the content of the "host" section, maybe empty
		 */
		inline std::string_view get_host() noexcept
		{
			return this->field(http::parses::url_fields::UF_HOST);
		}

		/**
		 * @brief Gets the content of the "host" section, maybe empty
		 * same as get_host
		 */
		inline std::string_view host() noexcept
		{
			return this->get_host();
		}

		inline std::string_view get_default_port() noexcept
		{
			std::string_view schema = this->schema();
			if (asio2::iequals(schema, "http"))
				return std::string_view{ "80" };
			if (asio2::iequals(schema, "https"))
				return std::string_view{ "443" };
			return std::string_view{ "80" };
		}

		inline std::string_view default_port() noexcept
		{
			return this->get_default_port();
		}

		/**
		 * @brief Gets the content of the "port" section
		 */
		inline std::string_view get_port() noexcept
		{
			std::string_view p = this->field(http::parses::url_fields::UF_PORT);
			if (p.empty())
				return this->default_port();
			return p;
		}

		/**
		 * @brief Gets the content of the "port" section
		 * same as get_port
		 */
		inline std::string_view port() noexcept
		{
			return this->get_port();
		}

		/**
		 * @brief Gets the content of the "path" section
		 * the return value maybe has undecoded char, you can use http::url_decode(...) to decoded it.
		 */
		inline std::string_view get_path() noexcept
		{
			std::string_view p = this->field(http::parses::url_fields::UF_PATH);
			if (p.empty())
				return std::string_view{ "/" };
			return p;
		}

		/**
		 * @brief Gets the content of the "path" section
		 * the return value maybe has undecoded char, you can use http::url_decode(...) to decoded it.
		 * same as get_path
		 */
		inline std::string_view path() noexcept
		{
			return this->get_path();
		}

		/**
		 * @brief Gets the content of the "query" section, maybe empty
		 * the return value maybe has undecoded char, you can use http::url_decode(...) to decoded it.
		 */
		inline std::string_view get_query() noexcept
		{
			return this->field(http::parses::url_fields::UF_QUERY);
		}

		/**
		 * @brief Gets the content of the "query" section, maybe empty
		 * the return value maybe has undecoded char, you can use http::url_decode(...) to decoded it.
		 * same as get_query
		 */
		inline std::string_view query() noexcept
		{
			return this->get_query();
		}

		/**
		 * @brief Gets the "target", which composed by path and query
		 * the return value maybe has undecoded char, you can use http::url_decode(...) to decoded it.
		 */
		inline std::string_view get_target() noexcept
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
		 * @brief Gets the "target", which composed by path and query
		 * the return value maybe has undecoded char, you can use http::url_decode(...) to decoded it.
		 * same as get_target
		 */
		inline std::string_view target() noexcept
		{
			return this->get_target();
		}

		/**
		 * @brief Gets the content of the specific section, maybe empty
		 */
		inline std::string_view get_field(http::parses::url_fields f) noexcept
		{
			if (!(parser_.field_set & (1 << int(f))))
				return std::string_view{};

			return std::string_view{ &string_[parser_.field_data[int(f)].off], parser_.field_data[int(f)].len };
		}

		/**
		 * @brief Gets the content of the specific section, maybe empty
		 * same as get_field
		 */
		inline std::string_view field(http::parses::url_fields f) noexcept
		{
			return this->get_field(std::move(f));
		}

		inline http::parses::http_parser_url &     parser() noexcept { return this->parser_; }
		inline std::string                   &     string() noexcept { return this->string_; }
		inline http::parses::http_parser_url & get_parser() noexcept { return this->parser_; }
		inline std::string                   & get_string() noexcept { return this->string_; }

	protected:
		http::parses::http_parser_url         parser_;
		std::string                           string_;
	};
}

#endif // !__ASIO2_HTTP_URL_HPP__
