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

#include <cassert>
#include <cctype>
#include <cstring>

#include <stdexcept>

#include <string>
#include <memory>
#include <algorithm>
#include <unordered_map>

#include <asio2/base/def.hpp>
#include <asio2/util/helper.hpp>
#include <asio2/base/error.hpp>

#ifdef _MSC_VER
#	pragma warning(push)
#	pragma warning(disable:4996)
#endif

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
			if (!_parse())
			{
				set_last_error((int)errcode::url_string_invalid);

				_reset();
			}
			else
			{
				_set_so_sndbuf_size_from_url();
				_set_so_rcvbuf_size_from_url();
				_set_send_buffer_size_from_url();
				_set_recv_buffer_size_from_url();
				_set_silence_timeout_from_url();
				_set_max_packet_size();
				_set_packet_header_flag();
				_set_auto_reconnect();
			}
		}
		virtual ~url_parser()
		{
		}

		std::string get_url()        { return m_url     ; }
		std::string get_protocol()   { return m_protocol; }
		std::string get_ip()         { return m_ip      ; }
		std::string get_port()       { return m_port    ; }
		std::string get_model()      { return m_model   ; }

		std::string get_param_value(std::string name)
		{
			auto iter = m_params.find(name);
			return ((iter != m_params.end()) ? iter->second : std::string());
		}

		template<typename _handler>
		void for_each_param(_handler handler)
		{
			for (auto & pair : m_params)
			{
				handler(pair.first, pair.second);
			}
		}

		inline int         get_so_sndbuf_size()     { return m_so_sndbuf_size;     }
		inline int         get_so_rcvbuf_size()     { return m_so_rcvbuf_size;     }
		inline std::size_t get_send_buffer_size()   { return m_send_buffer_size;   }
		inline std::size_t get_recv_buffer_size()   { return m_recv_buffer_size;   }
		inline long        get_silence_timeout()    { return m_silence_timeout;    }
		inline uint8_t     get_packet_header_flag() { return m_packet_header_flag; }
		inline uint32_t    get_max_packet_size()    { return m_max_packet_size;    }
		inline long        get_auto_reconnect()     { return m_auto_reconnect;     }

	protected:
		/**
		 * @function : setsockopt SO_SNDBUF
		 */
		void _set_so_sndbuf_size_from_url()
		{
			auto val = get_param_value("so_sndbuf");
			if (!val.empty())
			{
				int size = std::atoi(val.data());
				if (val.find_last_of('k') != std::string::npos)
					size *= 1024;
				else if (val.find_last_of('m') != std::string::npos)
					size *= 1024 * 1024;
				this->m_so_sndbuf_size = size;
			}
		}

		/**
		 * @function : setsockopt SO_RCVBUF
		 */
		void _set_so_rcvbuf_size_from_url()
		{
			auto val = get_param_value("so_rcvbuf");
			if (!val.empty())
			{
				int size = std::atoi(val.data());
				if (val.find_last_of('k') != std::string::npos)
					size *= 1024;
				else if (val.find_last_of('m') != std::string::npos)
					size *= 1024 * 1024;
				this->m_so_rcvbuf_size = size;
			}
		}

		/**
		 * @function : send_buffer_size for the socket::send function
		 */
		void _set_send_buffer_size_from_url()
		{
			auto val = get_param_value("send_buffer_size");
			if (!val.empty())
			{
				std::size_t size = static_cast<std::size_t>(std::strtoull(val.data(), nullptr, 10));
				if (val.find_last_of('k') != std::string::npos)
					size *= 1024;
				else if (val.find_last_of('m') != std::string::npos)
					size *= 1024 * 1024;
				if (size < 64)
					size = 1024;
				this->m_send_buffer_size = size;
			}
		}

		/**
		 * @function : recv_buffer_size for the socket::recv function
		 */
		void _set_recv_buffer_size_from_url()
		{
			auto val = get_param_value("recv_buffer_size");
			if (!val.empty())
			{
				std::size_t size = static_cast<std::size_t>(std::strtoull(val.data(), nullptr, 10));
				if (val.find_last_of('k') != std::string::npos)
					size *= 1024;
				else if (val.find_last_of('m') != std::string::npos)
					size *= 1024 * 1024;
				if (size < 64)
					size = 1024;
				this->m_recv_buffer_size = size;
			}
		}

		void _set_silence_timeout_from_url()
		{
			auto val = get_param_value("silence_timeout");
			if (!val.empty())
			{
				long size = static_cast<long>(std::atoi(val.data()));
				if (val.find_last_of('m') != std::string::npos)
					size *= 60;
				else if (val.find_last_of('h') != std::string::npos)
					size *= 60 * 60;
				this->m_silence_timeout = size;
			}
			else
			{
				if /**/ (m_protocol == "tcp")
					this->m_silence_timeout = ASIO2_DEFAULT_TCP_SILENCE_TIMEOUT;
				else if (m_protocol == "tcps")
					this->m_silence_timeout = ASIO2_DEFAULT_TCP_SILENCE_TIMEOUT;
				else if (m_protocol == "udp")
					this->m_silence_timeout = ASIO2_DEFAULT_UDP_SILENCE_TIMEOUT;
			}
		}

		void _set_max_packet_size()
		{
			// set max_packet_size from the url
			auto val = get_param_value("max_packet_size");
			if (!val.empty())
				m_max_packet_size = static_cast<uint32_t>(std::atoi(val.data()));
			if (m_max_packet_size < 8 || m_max_packet_size > ASIO2_MAX_PACKET_SIZE)
				m_max_packet_size = ASIO2_MAX_PACKET_SIZE;
		}

		void _set_packet_header_flag()
		{
			// set packet_header_flag from the url
			auto val = get_param_value("packet_header_flag");
			if (!val.empty())
				m_packet_header_flag = static_cast<uint8_t>(std::atoi(val.data()));
			if (m_packet_header_flag == 0 || m_packet_header_flag > ASIO2_MAX_HEADER_FLAG)
				m_packet_header_flag = ASIO2_DEFAULT_HEADER_FLAG;
		}

		void _set_auto_reconnect()
		{
			// set packet_header_flag from the url
			auto val = get_param_value("auto_reconnect");
			if (!val.empty())
			{
				if /**/ (val == "true")
					m_auto_reconnect = 0;
				else if (val == "false")
					m_auto_reconnect = -1;
				else
				{
					auto interval = std::strtol(val.data(), nullptr, 10);
					if /**/ (val.find("s") != std::string::npos)
						interval *= 1000;
					else if (val.find("m") != std::string::npos)
						interval *= 1000 * 60;
					else if (val.find("h") != std::string::npos)
						interval *= 1000 * 60 * 60;
					m_auto_reconnect = interval;
				}
			}
		}

	protected:
		bool _parse()
		{
			if (m_url.empty())
			{
				assert(false);
				return false;
			}

			// erase the invalid character : space \t \r \n and so on
			trim_all(m_url);

			// tolower
			std::transform(m_url.begin(), m_url.end(), m_url.begin(), ::tolower);

			// parse the protocol
			std::size_t pos_protocol_end = m_url.find("://", 0);
			if (pos_protocol_end == 0 || pos_protocol_end == std::string::npos)
			{
				assert(false);
				return false;
			}
			m_protocol = m_url.substr(0, pos_protocol_end - 0);

			// parse the ip
			std::size_t pos_ip_begin = pos_protocol_end + std::strlen("://");
			std::size_t pos_ip_end   = m_url.find(':', pos_ip_begin);
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
			std::size_t pos_port_end = m_url.find('/', pos_port_begin);
			m_port = m_url.substr(pos_port_begin, (pos_port_end == std::string::npos) ? std::string::npos : pos_port_end - pos_port_begin);
			if (m_port.empty())
				m_port = "0";

			if (pos_port_end != std::string::npos)
			{
				// parse the model
				std::size_t pos_model_begin = pos_port_end + std::strlen("/");
				std::size_t pos_model_end = m_url.find('?', pos_model_begin);
				if (pos_model_end != std::string::npos)
					m_model = m_url.substr(pos_model_begin, pos_model_end - pos_model_begin);
				else
				{
					if (m_url.find('=', pos_model_begin) == std::string::npos)
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
						while ((pos_sep = params.find('&', pos_sep)) != std::string::npos)
						{
							if (pos_sep > pos_head)
							{
								std::string param = params.substr(pos_head, pos_sep - pos_head);

								std::size_t pos_equal = param.find('=', 0);
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

		void _reset()
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

		int         m_so_sndbuf_size     = 0;
		int         m_so_rcvbuf_size     = 0;
		std::size_t m_send_buffer_size   = ASIO2_DEFAULT_SEND_BUFFER_SIZE;
		std::size_t m_recv_buffer_size   = ASIO2_DEFAULT_RECV_BUFFER_SIZE;
		long        m_silence_timeout    = 0;
		uint8_t     m_packet_header_flag = ASIO2_DEFAULT_HEADER_FLAG;
		uint32_t    m_max_packet_size    = ASIO2_MAX_PACKET_SIZE;
		long        m_auto_reconnect     = -1;
	};

}

#ifdef _MSC_VER
#	pragma warning(pop)
#endif

#endif // !__ASIO2_URL_PARSER_HPP__
