/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_HTTP_REQUEST_HPP__
#define __ASIO2_HTTP_REQUEST_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <string.h>

#include <memory>
#include <unordered_map>

#include <asio2/http/http_parser.h>
#include <asio2/http/mime_types.hpp>


namespace asio2
{

	class http_request
	{
		friend class http_request_parser;

	public:

		/**
		 * @construct
		 */
		explicit http_request()
		{
			m_uri.reserve(512);
			m_headers.reserve(30);
		}

		/**
		 * @destruct
		 */
		virtual ~http_request()
		{
		}

		/// get functions
	public:
		inline unsigned int   method()
		{
			return m_method;
		}
		inline std::string    uri()
		{
			return m_uri;
		}
		inline unsigned short http_major()
		{
			return m_http_major;
		}
		inline unsigned short http_minor()
		{
			return m_http_minor;
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
		inline http_request & method(unsigned int method)
		{
			m_method = method;
			return (*this);
		}
		inline http_request & uri(std::string uri)
		{
			m_uri.append(uri);
			return (*this);
		}
		inline http_request & uri(const char *at, size_t length)
		{
			m_uri.append(at, length);
			return (*this);
		}
		inline http_request & http_major(unsigned short major)
		{
			m_http_major = major;
			return (*this);
		}
		inline http_request & http_minor(unsigned short minor)
		{
			m_http_minor = minor;
			return (*this);
		}
		inline http_request & http_version(unsigned short major, unsigned short minor)
		{
			m_http_major = major;
			m_http_minor = minor;
			return (*this);
		}
		inline http_request & add_header(std::string & field, std::string & value)
		{
			m_headers.emplace(field, value);
			return (*this);
		}
		inline http_request & add_header(const char * field, const char * value)
		{
			m_headers.emplace(field, value);
			return (*this);
		}
		
	public:
		std::string data()
		{
			std::size_t size = 0;
			std::ostringstream oss;
			oss << http::method_strings[m_method] << " " << m_uri << " " << "HTTP/" << m_http_major << "." << m_http_minor << "\r\n";
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

		unsigned int   m_method     = http::HTTP_GET;       /* requests only */
		std::string    m_uri;
		unsigned short m_http_major = 1;
		unsigned short m_http_minor = 1;

		std::unordered_map<std::string, std::string> m_headers;

	};

	using request_ptr = std::shared_ptr<http_request>;

}

#endif // !__ASIO2_HTTP_REQUEST_HPP__
