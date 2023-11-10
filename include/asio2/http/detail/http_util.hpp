/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_HTTP_UTIL_HPP__
#define __ASIO2_HTTP_UTIL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cctype>
#include <sstream>

#include <memory>

#include <asio2/external/asio.hpp>
#include <asio2/external/beast.hpp>

#include <asio2/base/error.hpp>

#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/buffer_wrap.hpp>
#include <asio2/http/detail/http_parser.h>
#include <asio2/http/detail/mime_types.hpp>
#include <asio2/http/multipart.hpp>

#include <asio2/util/string.hpp>

#ifdef ASIO2_HEADER_ONLY
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
			  0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, // 2
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

			if (url.empty())
				return r;

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
		std::basic_string<CharT, Traits, Allocator> url_encode(std::string_view url, std::size_t offset = 0)
		{
			using size_type   [[maybe_unused]] = typename std::string_view::size_type;
			using value_type  [[maybe_unused]] = typename std::string_view::value_type;
			using rvalue_type [[maybe_unused]] = typename std::basic_string<CharT, Traits, Allocator>::value_type;

			std::basic_string<CharT, Traits, Allocator> r;
			r.reserve(url.size() * 2);

			if (url.empty())
				return r;

			size_type i = 0;

			if (offset == 0)
			{
				http::parses::http_parser_url u;
				if (0 == http::parses::http_parser_parse_url(url.data(), url.size(), 0, std::addressof(u)))
				{
					if /**/ (u.field_set & (1 << (int)http::parses::url_fields::UF_PATH))
					{
						i = u.field_data[(int)http::parses::url_fields::UF_PATH].off + 1;
					}
					else if (u.field_set & (1 << (int)http::parses::url_fields::UF_PORT))
					{
						i = u.field_data[(int)http::parses::url_fields::UF_PORT].off +
							u.field_data[(int)http::parses::url_fields::UF_PORT].len;
					}
					else if (u.field_set & (1 << (int)http::parses::url_fields::UF_HOST))
					{
						i = u.field_data[(int)http::parses::url_fields::UF_HOST].off +
							u.field_data[(int)http::parses::url_fields::UF_HOST].len;
					}

					if constexpr (std::is_same_v<CharT, char>)
					{
						r += std::string_view{ url.data(), i };
					}
					else
					{
						for (size_type n = 0; n < i; ++n)
						{
							r += static_cast<rvalue_type>(url[n]);
						}
					}
				}
			}
			else
			{
				r += url.substr(0, offset);
				i = offset;
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
		bool has_unencode_char(std::string_view url, std::size_t offset = 0) noexcept
		{
			using size_type   = typename std::string_view::size_type;
			using value_type  = typename std::string_view::value_type;

			if (url.empty())
				return false;

			size_type i = 0;

			if (offset == 0)
			{
				http::parses::http_parser_url u;
				if (0 == http::parses::http_parser_parse_url(url.data(), url.size(), 0, std::addressof(u)))
				{
					if /**/ (u.field_set & (1 << (int)http::parses::url_fields::UF_PATH))
					{
						i = u.field_data[(int)http::parses::url_fields::UF_PATH].off + 1;
					}
					else if (u.field_set & (1 << (int)http::parses::url_fields::UF_PORT))
					{
						i = u.field_data[(int)http::parses::url_fields::UF_PORT].off +
							u.field_data[(int)http::parses::url_fields::UF_PORT].len;
					}
					else if (u.field_set & (1 << (int)http::parses::url_fields::UF_HOST))
					{
						i = u.field_data[(int)http::parses::url_fields::UF_HOST].off +
							u.field_data[(int)http::parses::url_fields::UF_HOST].len;
					}
				}
			}
			else
			{
				i = offset;
			}

			for (; i < url.size(); ++i)
			{
				unsigned char c = static_cast<unsigned char>(url[i]);

				if (c == static_cast<unsigned char>('%'))
				{
					if (i + 3 <= url.size())
					{
						value_type h = url[i + 1];
						value_type l = url[i + 2];

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
		bool has_undecode_char(std::string_view url, std::size_t offset = 0) noexcept
		{
			using size_type   = typename std::string_view::size_type;
			using value_type  = typename std::string_view::value_type;

			if (url.empty())
				return false;

			for (size_type i = offset; i < url.size(); ++i)
			{
				value_type c = url[i];

				if (c == '%')
				{
					if (i + 3 <= url.size())
					{
						value_type h = url[i + 1];
						value_type l = url[i + 2];

						bool f1 = false, f2 = false;

						if /**/ (h >= '0' && h <= '9') { f1 = true; }
						else if (h >= 'a' && h <= 'f') { f1 = true; }
						else if (h >= 'A' && h <= 'F') { f1 = true; }

						if /**/ (l >= '0' && l <= '9') { f2 = true; }
						else if (l >= 'a' && l <= 'f') { f2 = true; }
						else if (l >= 'A' && l <= 'F') { f2 = true; }

						if (f1 && f2)
						{
							return true;
						}
					}
				}
				else if (c == '+')
				{
					return true;
				}
			}

			return false;
		}

		template<typename = void>
		std::string_view url_to_host(std::string_view url)
		{
			if (url.empty())
				return std::string_view{};

			http::parses::http_parser_url u;
			if (0 != http::parses::http_parser_parse_url(url.data(), url.size(), 0, std::addressof(u)))
				return std::string_view{};

			if (!(u.field_set & (1 << (int)http::parses::url_fields::UF_HOST)))
				return std::string_view{};

			return std::string_view{ &url[
				u.field_data[(int)http::parses::url_fields::UF_HOST].off],
				u.field_data[(int)http::parses::url_fields::UF_HOST].len };
		}

		template<typename = void>
		std::string_view url_to_port(std::string_view url)
		{
			if (url.empty())
				return std::string_view{};

			http::parses::http_parser_url u;
			if (0 != http::parses::http_parser_parse_url(url.data(), url.size(), 0, std::addressof(u)))
				return std::string_view{};

			if (u.field_set & (1 << (int)http::parses::url_fields::UF_PORT))
				return std::string_view{ &url[
					u.field_data[(int)http::parses::url_fields::UF_PORT].off],
					u.field_data[(int)http::parses::url_fields::UF_PORT].len };

			if (u.field_set & (1 << (int)http::parses::url_fields::UF_SCHEMA))
			{
				std::string_view schema(&url[
					u.field_data[(int)http::parses::url_fields::UF_SCHEMA].off],
					u.field_data[(int)http::parses::url_fields::UF_SCHEMA].len);
				if (asio2::iequals(schema, "http"))
					return std::string_view{ "80" };
				if (asio2::iequals(schema, "https"))
					return std::string_view{ "443" };
			}

			return std::string_view{ "80" };
		}

		template<typename = void>
		std::string_view url_to_path(std::string_view url)
		{
			if (url.empty())
				return std::string_view{};

			http::parses::http_parser_url u;
			if (0 != http::parses::http_parser_parse_url(url.data(), url.size(), 0, std::addressof(u)))
				return std::string_view{};

			if (!(u.field_set & (1 << (int)http::parses::url_fields::UF_PATH)))
				return std::string_view{ "/" };

			return std::string_view{ &url[
				u.field_data[(int)http::parses::url_fields::UF_PATH].off],
				u.field_data[(int)http::parses::url_fields::UF_PATH].len };
		}

		template<typename = void>
		std::string_view url_to_query(std::string_view url)
		{
			if (url.empty())
				return std::string_view{};

			http::parses::http_parser_url u;
			if (0 != http::parses::http_parser_parse_url(url.data(), url.size(), 0, std::addressof(u)))
				return std::string_view{};

			if (!(u.field_set & (1 << (int)http::parses::url_fields::UF_QUERY)))
				return std::string_view{};

			return std::string_view{ &url[
				u.field_data[(int)http::parses::url_fields::UF_QUERY].off],
				u.field_data[(int)http::parses::url_fields::UF_QUERY].len };
		}

		template<typename = void>
		inline bool url_match(std::string_view pattern, std::string_view url)
		{
			if (pattern == "*" || pattern == "/*")
				return true;

			if (url.empty())
				return false;

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

		template<class StringT = std::string_view>
		inline std::string make_error_page(http::status result, StringT&& desc = std::string_view{})
		{
			std::string_view reason = http::obsolete_reason(result);
			std::string_view descrb = asio2::detail::to_string_view(desc);
			std::string content;
			if (descrb.empty())
				content.reserve(reason.size() * 2 + 67);
			else
				content.reserve(reason.size() * 2 + 67 + descrb.size() + 21);
			content += "<html><head><title>";
			content += reason;
			content += "</title></head><body><h1>";
			content += std::to_string(asio2::detail::to_underlying(result));
			content += " ";
			content += reason;
			content += "</h1>";
			if (!descrb.empty())
			{
				content += "<p>Description : ";
				content += descrb;
				content += "</p>";
			}
			content += "</body></html>";
			return content;
		}

		template<class StringT = std::string_view>
		inline std::string error_page(http::status result, StringT&& desc = std::string_view{})
		{
			return make_error_page(result, std::forward<StringT>(desc));
		}

		/**
		 * @brief Returns `true` if the HTTP message's Content-Type is "multipart/form-data";
		 */
		template<class HttpMessage>
		inline bool has_multipart(const HttpMessage& msg) noexcept
		{
			return (asio2::ifind(msg[http::field::content_type], "multipart/form-data") != std::string_view::npos);
		}

		/**
		 * @brief Get the "multipart/form-data" body content.
		 */
		template<class HttpMessage, class String = std::string>
		inline basic_multipart_fields<String> get_multipart(const HttpMessage& msg)
		{
			return multipart_parser_execute(msg);
		}

		/**
		 * @brief Get the "multipart/form-data" body content. same as get_multipart
		 */
		template<class HttpMessage, class String = std::string>
		inline basic_multipart_fields<String> multipart(const HttpMessage& msg)
		{
			return get_multipart(msg);
		}
	}

	// /boost_1_80_0/libs/beast/example/doc/http_examples.hpp
	template<bool isRequest, class SyncReadStream, class DynamicBuffer, class HeaderCallback, class BodyCallback>
	void read_large_body(SyncReadStream& stream, DynamicBuffer& buffer, HeaderCallback&& cbh, BodyCallback&& cbb)
	{
		error_code& ec = asio2::get_last_error();

		// Declare the parser with an empty body since
		// we plan on capturing the chunks ourselves.
		http::parser<isRequest, http::string_body> hp;

		hp.body_limit((std::numeric_limits<std::uint64_t>::max)());

		// First read the complete header
		http::read_header(stream, buffer, hp, ec);
		if (ec)
			return;

		cbh(hp.get());

		// should we check the http reponse status?
		// should we handle the http range? 301 302 ?
		//if (hp.get().result() != http::status::ok)
		//{
		//	ec = http::error::bad_status;
		//	return;
		//}

		// if the http response has no body, returned with error.
		if (hp.is_done())
		{
			ec = http::error::end_of_stream;
			return;
		}

		http::parser<isRequest, http::buffer_body> p(std::move(hp));

		if (p.get().chunked())
		{
			// This container will hold the extensions for each chunk
			http::chunk_extensions ce;

			// This string will hold the body of each chunk
			//std::string chunk;

			// Declare our chunk header callback  This is invoked
			// after each chunk header and also after the last chunk.
			auto header_cb = [&](
				std::uint64_t size,          // Size of the chunk, or zero for the last chunk
				std::string_view extensions, // The raw chunk-extensions string. Already validated.
				error_code& ev)              // We can set this to indicate an error
			{
				// Parse the chunk extensions so we can access them easily
				ce.parse(extensions, ev);
				if (ev)
					return;

				// See if the chunk is too big
				if (size > (std::numeric_limits<std::size_t>::max)())
				{
					ev = http::error::body_limit;
					return;
				}

				// Make sure we have enough storage, and
				// reset the container for the upcoming chunk
				//chunk.reserve(static_cast<std::size_t>(size));
				//chunk.clear();
			};

			// Set the callback. The function requires a non-const reference so we
			// use a local variable, since temporaries can only bind to const refs.
			p.on_chunk_header(header_cb);

			bool continued = true;

			// Declare the chunk body callback. This is called one or
			// more times for each piece of a chunk body.
			auto body_cb = [&](
				std::uint64_t remain,   // The number of bytes left in this chunk
				std::string_view body,  // A buffer holding chunk body data
				error_code& ev)         // We can set this to indicate an error
			{
				// If this is the last piece of the chunk body,
				// set the error so that the call to `read` returns
				// and we can process the chunk.
				if (remain == body.size())
					ev = http::error::end_of_chunk;

				// Append this piece to our container
				//chunk.append(body.data(), body.size());
				continued = cbb(body);

				// The return value informs the parser of how much of the body we
				// consumed. We will indicate that we consumed everything passed in.
				return body.size();
			};
			p.on_chunk_body(body_cb);

			while (continued && !p.is_done())
			{
				// Read as much as we can. When we reach the end of the chunk, the chunk
				// body callback will make the read return with the end_of_chunk error.
				http::read(stream, buffer, p, ec);
				if (!ec)
					continue;
				else if (ec != http::error::end_of_chunk)
					return;
				else
					ec = {};
			}

			if (!continued && !p.is_done())
			{
				ec = asio::error::operation_aborted;
			}
		}
		else
		{
			std::array<char, 512> buf;

			while (!p.is_done())
			{
				p.get().body().data = buf.data();
				p.get().body().size = buf.size();

				std::size_t bytes_read = http::read(stream, buffer, p, ec);

				if (ec == http::error::need_buffer)
					ec = {};
				if (ec)
					return;

				ASIO2_ASSERT(bytes_read == buf.size() - p.get().body().size);

				if (!cbb(std::string_view(buf.data(), bytes_read)))
				{
					ec = asio::error::operation_aborted;
					break;
				}
			}
		}
	}

	template<bool isRequest, class Body, class Fields>
	inline void try_prepare_payload(http::message<isRequest, Body, Fields>& msg)
	{
		try
		{
			msg.prepare_payload();
		}
		catch (const std::exception&)
		{
			asio2::set_last_error(asio::error::invalid_argument);
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

#ifdef ASIO2_HEADER_ONLY
namespace bho::beast::websocket
#else
namespace boost::beast::websocket
#endif
{
	enum class frame : std::uint8_t
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
