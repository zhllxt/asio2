/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_SERVER_IMPL_HPP__
#define __ASIO2_SERVER_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <future>
#include <functional>

#include <boost/asio.hpp>
#include <boost/system/system_error.hpp>

#include <asio2/util/buffer.hpp>
#include <asio2/util/buffer_pool.hpp>
#include <asio2/util/multi_buffer_pool.hpp>

#include <asio2/base/io_service_pool.hpp>
#include <asio2/base/error.hpp>
#include <asio2/base/def.hpp>
#include <asio2/base/listener_mgr.hpp>
#include <asio2/base/url_parser.hpp>

namespace asio2
{

	class server_impl : public std::enable_shared_from_this<server_impl>
	{
	public:
		/**
		 * @construct
		 * @param    : listener_mgr_ptr - 
		 * @param    : url_parser_ptr - url parser shared_ptr
		 */
		explicit server_impl(
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr
		)
			: m_listener_mgr_ptr(listener_mgr_ptr)
			, m_url_parser_ptr(url_parser_ptr)
		{
		}

		/**
		 * @destruct
		 */
		virtual ~server_impl()
		{
		}

		/**
		 * @function : start the server
		 * @return   : true - start successed , false - start failed
		 */
		virtual bool start()
		{
			try
			{
				shared_from_this();
			}
			catch (std::bad_weak_ptr &)
			{
				assert(false);
				return false;
			}
			return true;
		}

		/**
		 * @function : stop the server
		 */
		virtual void stop() = 0;

		virtual bool is_start() = 0;

		/**
		 * @function : send data
		 */
		virtual bool send(std::shared_ptr<buffer<uint8_t>> buf_ptr) = 0;

		/**
		 * @function : send data
		 */
		virtual bool send(const uint8_t * buf, std::size_t len) = 0;

		/**
		 * @function : send data
		 */
		virtual bool send(const char * buf)
		{
			return this->send(reinterpret_cast<const uint8_t *>(buf), std::strlen(buf));
		}

		/**
		 * @function : get the listen address
		 */
		virtual std::string get_listen_address() = 0;

		/**
		 * @function : get the listen port
		 */
		virtual unsigned short get_listen_port() = 0;

		/**
		 * @function : get connected session count
		 */
		virtual std::size_t get_session_count() = 0;

	protected:

		virtual std::size_t _get_io_service_pool_size()
		{
			// get io_service_pool_size from the url
			std::size_t io_service_pool_size = std::thread::hardware_concurrency();
			std::string str_io_service_pool_size = m_url_parser_ptr->get_param_value("io_service_pool_size");
			if (!str_io_service_pool_size.empty())
				io_service_pool_size = static_cast<std::size_t>(std::atoi(str_io_service_pool_size.c_str()));
			if (io_service_pool_size == 0)
				io_service_pool_size = std::thread::hardware_concurrency();
			return io_service_pool_size;
		}

		virtual std::size_t _get_pool_buffer_size()
		{
			// get pool_buffer_size from the url
			std::size_t pool_buffer_size = 1024;
			std::string str_pool_buffer_size = m_url_parser_ptr->get_param_value("pool_buffer_size");
			if (!str_pool_buffer_size.empty())
			{
				pool_buffer_size = static_cast<std::size_t>(std::atoi(str_pool_buffer_size.c_str()));
				if (str_pool_buffer_size.find_last_of('k') != std::string::npos)
					pool_buffer_size *= 1024;
				else if (str_pool_buffer_size.find_last_of('m') != std::string::npos)
					pool_buffer_size *= 1024 * 1024;
			}
			if (pool_buffer_size < 16)
				pool_buffer_size = 1024;
			return pool_buffer_size;
		}


	protected:

		/// listener manager shared_ptr
		std::shared_ptr<listener_mgr> m_listener_mgr_ptr;

		/// url parser shared_ptr
		std::shared_ptr<url_parser>   m_url_parser_ptr;

	};
}

#endif // !__ASIO2_SERVER_IMPL_HPP__
