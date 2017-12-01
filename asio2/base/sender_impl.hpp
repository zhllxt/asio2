/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_SENDER_IMPL_HPP__
#define __ASIO2_SENDER_IMPL_HPP__

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

	class sender_impl : public std::enable_shared_from_this<sender_impl>
	{
	public:
		/**
		 * @construct
		 */
		sender_impl(
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
		virtual ~sender_impl()
		{
		}

		/**
		 * @function : start the sender
		 * @param    : async_connect - asynchronous connect to the server or sync
		 * @return   : true  - start successed , false - start failed
		 */
		virtual bool start()
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
		 * @function : stop the sender
		 */
		virtual void stop() = 0;

		virtual bool is_start() = 0;

		/**
		 * @function : send data
		 */
		virtual bool send(std::string ip, unsigned short port, std::shared_ptr<buffer<uint8_t>> buf_ptr) = 0;

		/**
		 * @function : send data
		 */
		virtual bool send(std::string ip, std::string port, std::shared_ptr<buffer<uint8_t>> buf_ptr)
		{
			if (!ip.empty() && !port.empty())
				return this->send(ip, static_cast<unsigned short>(std::atoi(port.c_str())), buf_ptr);
			return false;
		}

		/**
		 * @function : send data
		 */
		virtual bool send(std::string ip, unsigned short port, const uint8_t * buf, std::size_t len) = 0;

		/**
		 * @function : send data
		 */
		virtual bool send(std::string ip, std::string port, const uint8_t * buf, std::size_t len)
		{
			if (!ip.empty() && !port.empty())
				return this->send(ip, static_cast<unsigned short>(std::atoi(port.c_str())), buf, len);
			return false;
		}

		/**
		 * @function : send data
		 */
		virtual bool send(std::string ip, unsigned short port, const char * buf)
		{
			if (!ip.empty())
				return this->send(ip, port, reinterpret_cast<const uint8_t *>(buf), std::strlen(buf));
			return false;
		}

		/**
		 * @function : send data
		 */
		virtual bool send(std::string ip, std::string port, const char * buf)
		{
			if (!ip.empty() && !port.empty())
				return this->send(ip, static_cast<unsigned short>(std::atoi(port.c_str())), reinterpret_cast<const uint8_t *>(buf), std::strlen(buf));
			return false;
		}

	public:
		/**
		 * @function : set socket's recv buffer size.
		 *             when packet lost rate is high,you can set the recv buffer size to a big value to avoid it.
		 */
		virtual bool set_recv_buffer_size(int size) = 0;

		/**
		 * @function : set socket's send buffer size
		 */
		virtual bool set_send_buffer_size(int size) = 0;

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
		/**
		 * @function : get last active time 
		 */
		std::chrono::time_point<std::chrono::steady_clock> get_last_active_time()
		{
			return m_last_active_time;
		}

		/**
		 * @function : reset last active time 
		 */
		void reset_last_active_time()
		{
			m_last_active_time = std::chrono::steady_clock::now();
		}

		/**
		 * @function : get silence duration of milliseconds
		 */
		std::chrono::milliseconds::rep get_silence_duration()
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_last_active_time).count();
		}

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

		virtual void _set_send_buffer_size_from_url()
		{
			// set send buffer size from url params
			std::string str_send_buffer_size = m_url_parser_ptr->get_param_value("send_buffer_size");
			if (!str_send_buffer_size.empty())
			{
				int send_buffer_size = std::atoi(str_send_buffer_size.c_str());
				if (str_send_buffer_size.find_last_of('k') != std::string::npos)
					send_buffer_size *= 1024;
				else if (str_send_buffer_size.find_last_of('m') != std::string::npos)
					send_buffer_size *= 1024 * 1024;
				if (send_buffer_size < 1024)
					send_buffer_size = 1024;
				this->set_send_buffer_size(send_buffer_size);
			}
		}

		virtual void _set_recv_buffer_size_from_url()
		{
			std::string str_recv_buffer_size = m_url_parser_ptr->get_param_value("recv_buffer_size");
			if (!str_recv_buffer_size.empty())
			{
				int recv_buffer_size = std::atoi(str_recv_buffer_size.c_str());
				if (str_recv_buffer_size.find_last_of('k') != std::string::npos)
					recv_buffer_size *= 1024;
				else if (str_recv_buffer_size.find_last_of('m') != std::string::npos)
					recv_buffer_size *= 1024 * 1024;
				if (recv_buffer_size < 1024)
					recv_buffer_size = 1024;
				this->set_recv_buffer_size(recv_buffer_size);
			}
		}

	protected:

		std::shared_ptr<io_service> m_send_ioservice_ptr;
		std::shared_ptr<io_service> m_recv_ioservice_ptr;

		/// asio::strand shared_ptr,used to ensure socket multi thread safe,we must ensure that only
		/// one operator can operate socket at the same time,and strand can enuser that the event will
		/// be processed in the order of post, eg : strand.post(1);strand.post(2); the 2 will processed
		/// certaion after the 1,if 1 is block,the 2 won't be processed,util the 1 is processed completed
		/// more details see : http://bbs.csdn.net/topics/390931471
		std::shared_ptr<boost::asio::io_service::strand> m_send_strand_ptr;
		std::shared_ptr<boost::asio::io_service::strand> m_recv_strand_ptr;

		/// last active time 
		std::chrono::time_point<std::chrono::steady_clock> m_last_active_time = std::chrono::steady_clock::now();

		/// listener manager shared_ptr
		std::shared_ptr<listener_mgr>        m_listener_mgr_ptr;

		/// url parser shared_ptr
		std::shared_ptr<url_parser>          m_url_parser_ptr;

	};
}

#endif // !__ASIO2_SENDER_IMPL_HPP__
