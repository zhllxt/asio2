/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_CLIENT_IMPL_HPP__
#define __ASIO2_CLIENT_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdlib>

#include <memory>
#include <future>
#include <functional>

#include <boost/asio.hpp>
#include <boost/system/system_error.hpp>

#include <asio2/base/io_service_pool.hpp>
#include <asio2/base/error.hpp>
#include <asio2/base/def.hpp>
#include <asio2/base/listener_mgr.hpp>
#include <asio2/base/url_parser.hpp>

namespace asio2
{

	class client_impl : public std::enable_shared_from_this<client_impl>
	{
	public:
		/**
		 * @construct
		 */
		client_impl(
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
		virtual ~client_impl()
		{
		}

		/**
		 * @function : start the client
		 * @param    : async_connect - asynchronous connect to the server or sync
		 * @return   : true  - start successed , false - start failed
		 */
		virtual bool start(bool async_connect = true)
		{
			// if you want use shared_from_this() function in the derived from enable_shared_from_this class,
			// the object must created with "new" mode on the heap,if you create the object on the stack,
			// it will throw a exception,and can't create object use shared_from_this(),but in many 
			// member functions,it has to use shared_from_this() to create object.
			// so here,at debug mode,we check if user create the object correct
			// note : can't call shared_from_this() in construct function
			// note : can't call shared_from_this() in shared_ptr custom deleter function 
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
		 * @function : stop the client
		 */
		virtual void stop() = 0;

		virtual bool is_start() = 0;

		/**
		 * @function : send data
		 * @param    : send_buf_ptr - std::shared_ptr<uint8_t> object
		 *             len          - data len
		 */
		virtual bool send(std::shared_ptr<uint8_t> send_buf_ptr, std::size_t len) = 0;

		/**
		 * @function : send data
		 */
		virtual bool send(const char * buf, std::size_t len) = 0;

		/**
		 * @function : send data
		 */
		virtual bool send(const char * buf) = 0;

	public:
		/**
		 * @function : get the local address
		 */
		virtual std::string get_local_address() = 0;

		/**
		 * @function : get the local port
		 */
		virtual unsigned short get_local_port() = 0;

		/**
		 * @function : get the remote address
		 */
		virtual std::string get_remote_address() = 0;

		/**
		 * @function : get the remote port
		 */
		virtual unsigned short get_remote_port() = 0;

	public:
		///**
		// * @function : get last active time 
		// */
		//virtual std::chrono::time_point<std::chrono::steady_clock> get_last_active_time() = 0;

		///**
		// * @function : reset last active time 
		// */
		//virtual void reset_last_active_time() = 0;

		///**
		// * @function : get silence duration of seconds
		// */
		//virtual std::chrono::seconds::rep get_silence_duration() = 0;

	protected:

		virtual std::size_t _get_io_service_pool_size()
		{
			// get io_service_pool_size from the url
			std::size_t io_service_pool_size = 2;
			std::string str_io_service_pool_size = m_url_parser_ptr->get_param_value("io_service_pool_size");
			if (!str_io_service_pool_size.empty())
				io_service_pool_size = static_cast<std::size_t>(std::atoi(str_io_service_pool_size.c_str()));
			if (io_service_pool_size == 0)
				io_service_pool_size = 2;
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
		std::shared_ptr<listener_mgr>        m_listener_mgr_ptr;

		/// url parser shared_ptr
		std::shared_ptr<url_parser>          m_url_parser_ptr;

	};
}

#endif // !__ASIO2_CLIENT_IMPL_HPP__
