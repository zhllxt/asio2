/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_HTTP_RESPONSE_PARSER_HPP__
#define __ASIO2_HTTP_RESPONSE_PARSER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <unordered_map>

#include <asio2/base/def.hpp>

#include <asio2/http/http_parser.h>
#include <asio2/http/mime_types.hpp>

#include <asio2/util/buffer.hpp>

#include <asio2/http/http_request.hpp>
#include <asio2/http/http_response.hpp>

namespace asio2
{

	class http_response_parser
	{
		friend class http_connection_impl;

	protected:

		typedef enum { NOTHING, FIELD, VALUE } last_on_header_t;

		enum class status { idle, indeterminate, success, fail };

	public:

		/**
		 * @construct
		 */
		explicit http_response_parser()
		{
			m_parser.data = this;

			header_value.reserve(128);
		}

		/**
		 * @destruct
		 */
		virtual ~http_response_parser()
		{
		}

		void reset()
		{
			http::http_parser_init(&m_parser, http::HTTP_RESPONSE);
			m_status = status::idle;
			header_field = "";
			header_value = "";
			last_on_header = NOTHING;
		}

		inline int get_http_errno()
		{
			return (((int)m_parser.http_errno) | ASIO2_HTTP_ERROR_CODE_MASK);
		}

		inline status get_status()
		{
			return m_status;
		}

		status parse(std::shared_ptr<buffer<uint8_t>> buf_ptr, std::shared_ptr<http_response> response_ptr)
		{
			if (buf_ptr->size() == 0)
			{
				m_status = status::idle;
				return m_status;
			}

			size_t parsed = http::http_parser_execute(&m_parser, &get_http_parser_settings(), (const char *)buf_ptr->read_begin(), buf_ptr->size(), response_ptr.get());

			/* Stop parsing if we parsed nothing, as that indicates something header! */
			if (parsed == 0 || m_parser.http_errno != http::HPE_OK)
			{
				m_status = status::fail;
				return m_status;
			}

			if (m_parser.state == http::state::s_start_req)
			{
				m_status = status::success;
				return m_status;
			}

			/* Stop parsing if we're a connection upgrade (e.g. WebSockets) */
			//if (m_parser.upgrade) {
			//	return;
			//}

			m_status = status::indeterminate;
			return m_status;
		}

	protected:

		http::http_parser_settings & get_http_parser_settings()
		{
			static http::http_parser_settings settings =
			{
				[](http::http_parser * parser, void * user_data) -> int
				{
					return static_cast<http_response_parser *>(parser->data)->on_message_begin(user_data);
				},
				[](http::http_parser * parser, const char *at, size_t length, void * user_data) -> int
				{
					return static_cast<http_response_parser *>(parser->data)->on_url(at, length, user_data);
				},
				[](http::http_parser * parser, const char *at, size_t length, void * user_data) -> int
				{
					return static_cast<http_response_parser *>(parser->data)->on_status(at, length, user_data);
				},
				[](http::http_parser * parser, const char *at, size_t length, void * user_data) -> int
				{
					return static_cast<http_response_parser *>(parser->data)->on_header_field(at, length, user_data);
				},
				[](http::http_parser * parser, const char *at, size_t length, void * user_data) -> int
				{
					return static_cast<http_response_parser *>(parser->data)->on_header_value(at, length, user_data);
				},
				[](http::http_parser * parser, void * user_data) -> int
				{
					return static_cast<http_response_parser *>(parser->data)->on_headers_complete(user_data);
				},
				[](http::http_parser * parser, const char *at, size_t length, void * user_data) -> int
				{
					return static_cast<http_response_parser *>(parser->data)->on_body(at, length, user_data);
				},
				[](http::http_parser * parser, void * user_data) -> int
				{
					return static_cast<http_response_parser *>(parser->data)->on_message_complete(user_data);
				},
				[](http::http_parser * parser, void * user_data) -> int
				{
					return static_cast<http_response_parser *>(parser->data)->on_chunk_header(user_data);
				},
				[](http::http_parser * parser, void * user_data) -> int
				{
					return static_cast<http_response_parser *>(parser->data)->on_chunk_complete(user_data);
				}
			};
			return settings;
		}

	protected:
		int on_message_begin(void * user_data)
		{
			return 0;
		}
		int on_url(const char *at, size_t length, void * user_data)
		{
			return 0;
		}
		int on_status(const char *at, size_t length, void * user_data)
		{
			static_cast<http_response*>(user_data)->status_code(m_parser.status_code);
			return 0;
		}
		int on_header_field(const char *at, size_t length, void * user_data)
		{
			switch (last_on_header) {
			case NOTHING:
				// Allocate new buffer and copy callback data into it
				header_field.assign(at, length);
				break;
			case VALUE:
				// New header started.
				// Copy current name,value buffers to headers
				// list and allocate new buffer for new name
				std::transform(header_field.begin(), header_field.end(), header_field.begin(), ::tolower);
				static_cast<http_response*>(user_data)->add_header(header_field, header_value);
				header_field.assign(at, length);
				break;
			case FIELD:
				// Previous name continues. Reallocate name
				// buffer and append callback data to it
				header_field.append(at, length);
				break;
			}
			last_on_header = FIELD;
			return 0;
		}
		int on_header_value(const char *at, size_t length, void * user_data)
		{
			switch (last_on_header) {
			case FIELD:
				//Value for current header started. Allocate
				//new buffer and copy callback data to it
				header_value.assign(at, length);
				break;
			case VALUE:
				//Value continues. Reallocate value buffer
				//and append callback data to it
				header_value.append(at, length);
				break;
			case NOTHING:
				// this shouldn't happen
				//DEBUG(10)("Internal error in http-parser");
				break;
			}
			last_on_header = VALUE;
			return 0;
		}
		int on_headers_complete(void * user_data)
		{
			static_cast<http_response*>(user_data)->http_version(m_parser.http_major, m_parser.http_minor);

			/* Add the most recently read header to the map, if any */
			if (last_on_header == VALUE)
			{
				std::transform(header_field.begin(), header_field.end(), header_field.begin(), ::tolower);
				static_cast<http_response*>(user_data)->add_header(header_field, header_value);
				header_field = "";
			}

			/* See if we can guess a file extension */
			std::string extension = get_extension_for_mime_type(static_cast<http_response*>(user_data)->get_header_value("content-type"));

			/* Choose an output function based on the content encoding */
			std::string content_encoding(static_cast<http_response*>(user_data)->get_header_value("content-encoding"));

			if ((content_encoding == "gzip" || content_encoding == "deflate")) {
				//#ifdef HAVE_LIBZ
				//				DEBUG(10) ("%s: detected zlib content, decompressing", output_path.data());
				//				unzip = true;
				//#else
				//				/* We can't decompress, so just give it a .gz */
				//				output_path.append(".gz");
				//				DEBUG(5) ("%s: refusing to decompress since zlib is unavailable", output_path.data());
				//#endif
			}


			/* We can do something smart with the headers here.
			*
			* For example, we could:
			*  - Record all headers into the report.xml
			*  - Pick the intended filename if we see Content-Disposition: attachment; name="..."
			*  - Record headers into filesystem extended attributes on the body file
			*/

			return 0;
		}
		int on_body(const char *at, size_t length, void * user_data)
		{
			return 0;
		}
		int on_message_complete(void * user_data)
		{
			reset();

			return 0;
		}
		int on_chunk_header(void * user_data)
		{
			return 0;
		}
		int on_chunk_complete(void * user_data)
		{
			return 0;
		}

	protected:
		http::http_parser m_parser;

		status m_status = status::idle;

		/* placeholders for possibly-incomplete header data */
		last_on_header_t last_on_header = NOTHING;
		std::string      header_value, header_field;
	};

}

#endif // !__ASIO2_HTTP_RESPONSE_PARSER_HPP__
