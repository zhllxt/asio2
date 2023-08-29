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
		 */
		url()
		{
			this->reset("");
		}

		/**
		 * @brief constructor
		 */
		url(std::string str)
		{
			this->reset(std::move(str));
		}

		url(std::string_view str)
		{
			this->reset(str);
		}

		url(const char* str)
		{
			this->reset(str);
		}

		url(url&&) noexcept = default;
		url(url const&) = default;
		url& operator=(url&&) noexcept = default;
		url& operator=(url const&) = default;

		url& reset(std::string str)
		{
			string_ = std::move(str);

			std::memset((void*)(std::addressof(parser_)), 0, sizeof(http::parses::http_parser_url));

			if (!string_.empty())
			{
				if (0 != http::parses::http_parser_parse_url(
					string_.data(), string_.size(), 0, std::addressof(parser_)))
				{
					asio2::set_last_error(asio::error::invalid_argument);
				}
			}

			return *this;
		}

		url& reset(std::string_view str)
		{
			return reset(std::string(str));
		}

		url& reset(const char* str)
		{
			return reset(std::string(str));
		}

		/**
		 * @brief Gets the content of the "schema" section, maybe empty
		 * The return value is usually http or https
		 */
		inline std::string_view get_schema() const noexcept
		{
			return this->field(http::parses::url_fields::UF_SCHEMA);
		}

		/**
		 * @brief Gets the content of the "schema" section, maybe empty
		 * The return value is usually http or https
		 * same as get_schema
		 */
		inline std::string_view schema() const noexcept
		{
			return this->get_schema();
		}

		/**
		 * @brief Gets the content of the "host" section, maybe empty
		 */
		inline std::string_view get_host() const noexcept
		{
			return this->field(http::parses::url_fields::UF_HOST);
		}

		/**
		 * @brief Gets the content of the "host" section, maybe empty
		 * same as get_host
		 */
		inline std::string_view host() const noexcept
		{
			return this->get_host();
		}

		inline std::string_view get_default_port() const noexcept
		{
			std::string_view schema = this->schema();
			if (asio2::iequals(schema, "http"))
				return std::string_view{ "80" };
			if (asio2::iequals(schema, "https"))
				return std::string_view{ "443" };
			return std::string_view{ "80" };
		}

		inline std::string_view default_port() const noexcept
		{
			return this->get_default_port();
		}

		/**
		 * @brief Gets the content of the "port" section
		 */
		inline std::string_view get_port() const noexcept
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
		inline std::string_view port() const noexcept
		{
			return this->get_port();
		}

		/**
		 * @brief Gets the content of the "path" section
		 * the return value maybe has undecoded char, you can use http::url_decode(...) to decoded it.
		 */
		inline std::string_view get_path() const noexcept
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
		inline std::string_view path() const noexcept
		{
			return this->get_path();
		}

		/**
		 * @brief Gets the content of the "query" section, maybe empty
		 * the return value maybe has undecoded char, you can use http::url_decode(...) to decoded it.
		 */
		inline std::string_view get_query() const noexcept
		{
			return this->field(http::parses::url_fields::UF_QUERY);
		}

		/**
		 * @brief Gets the content of the "query" section, maybe empty
		 * the return value maybe has undecoded char, you can use http::url_decode(...) to decoded it.
		 * same as get_query
		 */
		inline std::string_view query() const noexcept
		{
			return this->get_query();
		}

		/**
		 * @brief Gets the "target", which composed by path and query
		 * the return value maybe has undecoded char, you can use http::url_decode(...) to decoded it.
		 */
		inline std::string_view get_target() const noexcept
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
		inline std::string_view target() const noexcept
		{
			return this->get_target();
		}

		/**
		 * @brief Gets the content of the specific section, maybe empty
		 */
		inline std::string_view get_field(http::parses::url_fields f) const noexcept
		{
			if (!(parser_.field_set & (1 << int(f))))
				return std::string_view{};

			return std::string_view{ &string_[parser_.field_data[int(f)].off], parser_.field_data[int(f)].len };
		}

		/**
		 * @brief Gets the content of the specific section, maybe empty
		 * same as get_field
		 */
		inline std::string_view field(http::parses::url_fields f) const noexcept
		{
			return this->get_field(std::move(f));
		}

		inline http::parses::http_parser_url &     parser() noexcept { return this->parser_; }
		inline std::string                   &     string() noexcept { return this->string_; }
		inline http::parses::http_parser_url & get_parser() noexcept { return this->parser_; }
		inline std::string                   & get_string() noexcept { return this->string_; }

		inline http::parses::http_parser_url const&     parser() const noexcept { return this->parser_; }
		inline std::string                   const&     string() const noexcept { return this->string_; }
		inline http::parses::http_parser_url const& get_parser() const noexcept { return this->parser_; }
		inline std::string                   const& get_string() const noexcept { return this->string_; }

	protected:
		http::parses::http_parser_url         parser_;
		std::string                           string_;
	};

	template<class derived_t>
	std::string make_url_string(derived_t& derive, std::string_view target)
	{
		std::string url;

		if constexpr (std::is_base_of_v<asio2::detail::ssl_stream_tag, derived_t>)
		{
			url += "https://";

			url += derive.get_local_address();

			if (unsigned short n = derive.get_local_port(); n != 443)
			{
				url += ":";
				url += std::to_string(n);
			}
		}
		else
		{
			url = "http://";

			url += derive.get_local_address();

			if (unsigned short n = derive.get_local_port(); n != 80)
			{
				url += ":";
				url += std::to_string(n);
			}
		}

		url += target;

		return url;
	}

	template<class derived_t>
	std::string make_url_string(std::shared_ptr<derived_t>& derive_ptr, std::string_view target)
	{
		return make_url_string(*derive_ptr, target);
	}

	template<class derived_t>
	http::url make_url(derived_t& derive, std::string_view target)
	{
		return http::url(make_url_string(derive, target));
	}

	template<class derived_t>
	http::url make_url(std::shared_ptr<derived_t>& derive_ptr, std::string_view target)
	{
		return http::url(make_url_string(derive_ptr, target));
	}
}

#endif // !__ASIO2_HTTP_URL_HPP__
