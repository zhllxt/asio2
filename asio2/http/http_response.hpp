/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_HTTP_RESPONSE_HPP__
#define __ASIO2_HTTP_RESPONSE_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <unordered_map>

#include <asio2/http/http_parser.h>
#include <asio2/http/mime_types.hpp>


namespace asio2
{

	class http_response
	{
	public:

		/**
		* @construct
		*/
		explicit http_response()
		{
			m_headers.reserve(30);
		}

		/**
		* @destruct
		*/
		virtual ~http_response()
		{
		}
		/// get functions
	public:
		inline unsigned short http_major()
		{
			return m_http_major;
		}
		inline unsigned short http_minor()
		{
			return m_http_minor;
		}
		inline unsigned int status_code()
		{
			return m_status_code; /* responses only */
		}
		/**
		 * the handler like this : 
		 * void fun(std::string field, std::string value);
		 * or 
		 * void fun(std::string & field, std::string & value);
		 */
		template<typename _handler>
		void for_each_header(_handler handler)
		{
			for (auto & pair : m_headers)
			{
				handler(pair.first, pair.second);
			}
		}
		inline std::string get_header_value(const std::string & field)
		{
			auto iter = m_headers.find(field);
			return ((iter == m_headers.end()) ? "" : iter->second);
		}

	public:
		inline bool is_keepalive()
		{
			auto iter = m_headers.find("connection");
			if (iter != m_headers.end())
			{
				if (
#if defined(__unix__) || defined(__linux__)
					strcasecmp
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
					_stricmp
#endif
					(iter->second.data(), "keep-alive") == 0)
					return true;
			}
			return false;
		}

		inline std::string host()
		{
			auto iter = m_headers.find("host");
			if (iter != m_headers.end())
			{
				auto pos = iter->second.find_first_of(':');
				if (pos != std::string::npos)
					return iter->second.substr(0, pos);
				else
					return iter->second;
			}
			return "";
		}

		inline std::string port()
		{
			auto iter = m_headers.find("host");
			if (iter != m_headers.end())
			{
				auto pos = iter->second.find_first_of(':');
				if (pos != std::string::npos)
					return iter->second.substr(pos + 1);
			}
			return "";
		}

		/// set functions
	public:
		inline http_response & http_major(unsigned short major)
		{
			m_http_major = major;
			return (*this);
		}
		inline http_response & http_minor(unsigned short minor)
		{
			m_http_minor = minor;
			return (*this);
		}
		inline http_response & status_code(unsigned int status_code)
		{
			m_status_code = status_code; /* responses only */
			return (*this);
		}
		inline http_response & http_version(unsigned short major, unsigned short minor)
		{
			m_http_major = major;
			m_http_minor = minor;
			return (*this);
		}
		inline http_response & add_header(std::string & field, std::string & value)
		{
			m_headers.emplace(field, value);
			return (*this);
		}
		inline http_response & add_header(const char * field, const char * value)
		{
			m_headers.emplace(field, value);
			return (*this);
		}
		
	public:
		bool url_decode(std::string & url)
		{
			std::size_t index = 0, size = url.size();
			for (std::size_t i = 0; i < size; ++i)
			{
				if (url[i] == '%')
				{
					if (i + 3 <= size)
					{
						int value = 0;
						std::istringstream is(url.substr(i + 1, 2));
						if (is >> std::hex >> value)
						{
							url[index++] = static_cast<char>(value);
							i += 2;
						}
						else
							return false;
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
		//HTTP/1.1 302 Found
		//Server: openresty
		//Date: Fri, 10 Nov 2017 03:11:50 GMT
		//Content-Type: text/html; charset=utf-8
		//Transfer-Encoding: chunked
		//Connection: keep-alive
		//Keep-Alive: timeout=20
		//Location: http://bbs.csdn.net/home
		//Cache-Control: no-cache
		//
		//5a
		//<html><body>You are being <a href="http://bbs.csdn.net/home">redirected</a>.</body></html>
		//0
		std::string data()
		{
			std::ostringstream oss;
			oss << "HTTP/" << m_http_major << "." << m_http_minor << " " << m_status_code << " " << http::http_status_str(m_status_code) << "\r\n";
			for (auto & pair : m_headers)
			{
				oss << pair.first << ": " << pair.second << "\r\n";
			}
			oss << "\r\n";

			return oss.str();
		}

		std::size_t  size()
		{
			return 0;
		}


	protected:
		unsigned short m_http_major   = 1;
		unsigned short m_http_minor   = 1;
		unsigned int   m_status_code  = http::HTTP_STATUS_OK; /* responses only */

		std::unordered_map<std::string, std::string> m_headers;

	};

	using response_ptr = std::shared_ptr<http_response>;

}

#endif // !__ASIO2_HTTP_RESPONSE_HPP__
