/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */


#ifndef __ASIO2_URL_PARSER_HPP__
#define __ASIO2_URL_PARSER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if !defined(NDEBUG) && !defined(DEBUG) && !defined(_DEBUG)
#	define NDEBUG
#endif

#include <cctype>
#include <cassert>
#include <cstring>

#include <stdexcept>

#include <string>
#include <memory>
#include <algorithm>
#include <unordered_map>

#include <asio2/util/helper.hpp>

namespace asio2 
{

	/**
	 * more details see server.hpp
	 */
	class url_parser
	{
	public:
		url_parser(std::string url) : m_url(url)
		{
			if (!_parse_url())
				_clear();
		}
		virtual ~url_parser()
		{
		}

		std::string get_url()        { return m_url; }
		std::string get_protocol()   { return m_protocol; }
		std::string get_ip()         { return m_ip; }
		std::string get_port()       { return m_port; }
		std::string get_model()      { return m_model; }

		std::string get_param_value(std::string name)
		{
			auto iterator = m_params.find(name);
			if (iterator != m_params.end())
				return iterator->second;
			return std::string();
		}

		template<typename _handler>
		void for_each_param(_handler handler)
		{
			std::for_each(m_params.begin(), m_params.end(), handler);
		}

	protected:

		bool _parse_url()
		{
			if (m_url.empty())
			{
				assert(false);
				return false;
			}

			// erase the invalid character : space \t \r \n and so on
			trim(m_url);

			// tolower
			std::transform(m_url.begin(), m_url.end(), m_url.begin(), ::tolower);

			// parse the protocol
			std::size_t pos_protocol_end = m_url.find_first_of("://", 0);
			if (pos_protocol_end == 0 || pos_protocol_end == std::string::npos)
			{
				assert(false);
				return false;
			}
			m_protocol = m_url.substr(0, pos_protocol_end - 0);

			// parse the ip
			std::size_t pos_ip_begin = pos_protocol_end + std::strlen("://");
			std::size_t pos_ip_end   = m_url.find_first_of(":", pos_ip_begin);
			if (pos_ip_end == std::string::npos)
			{
				assert(false);
				return false;
			}
			m_ip = m_url.substr(pos_ip_begin, pos_ip_end - pos_ip_begin);
			if (m_ip.empty() || m_ip == "*")
				m_ip = "0.0.0.0";
			else if (m_ip == "localhost")
				m_ip = "127.0.0.1";

			// parse the port
			std::size_t pos_port_begin = pos_ip_end + std::strlen(":");
			std::size_t pos_port_end = m_url.find_first_of("/", pos_port_begin);
			m_port = m_url.substr(pos_port_begin, (pos_port_end == std::string::npos) ? std::string::npos : pos_port_end - pos_port_begin);
			if (m_port.empty())
			{
				assert(false);
				return false;
			}

			if (pos_port_end != std::string::npos)
			{
				// parse the model
				std::size_t pos_model_begin = pos_port_end + std::strlen("/");
				std::size_t pos_model_end = m_url.find_first_of("?", pos_model_begin);
				if (pos_model_end != std::string::npos)
					m_model = m_url.substr(pos_model_begin, pos_model_end - pos_model_begin);
				else
				{
					if (m_url.find_first_of("=", pos_model_begin) == std::string::npos)
						m_model = m_url.substr(pos_model_begin);
					else
						pos_model_end = pos_model_begin - 1;
				}

				if (pos_model_end != std::string::npos)
				{
					// parse the params
					std::size_t pos_params_begin = pos_model_end + std::strlen("?");
					if (m_url.length() > pos_params_begin)
					{
						std::string params = m_url.substr(pos_params_begin);
						if (params[params.length() - 1] != '&')
							params += '&';

						std::size_t pos_sep = 0, pos_head = 0;
						while ((pos_sep = params.find_first_of('&', pos_sep)) != std::string::npos)
						{
							if (pos_sep > pos_head)
							{
								std::string param = params.substr(pos_head, pos_sep - pos_head);

								std::size_t pos_equal = param.find_first_of('=', 0);
								if (pos_equal > 0 && pos_equal + 1 < param.length())
								{
									std::string name = param.substr(0, pos_equal);
									pos_equal++;
									std::string value = param.substr(pos_equal);
									m_params.emplace(name, value);
								}
							}

							pos_sep++;
							pos_head = pos_sep;
						}
					}
				}
			}

			return true;
		}

		void _clear()
		{
			m_url.clear();

			m_protocol.clear();
			m_ip.clear();
			m_port.clear();
			m_model.clear();

			m_params.clear();
		}

	protected:

		std::string m_url;

		std::string m_protocol;
		std::string m_ip;
		std::string m_port;
		std::string m_model;

		std::unordered_map<std::string, std::string> m_params;

	};

}

#endif // !__ASIO2_URL_PARSER_HPP__
