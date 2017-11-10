/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_CONNECTION_IMPL_HPP__
#define __ASIO2_CONNECTION_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <chrono>
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

	class connection_impl : public std::enable_shared_from_this<connection_impl>
	{
	public:
		
		/**
		 * @construct
		 */
		connection_impl(
			std::shared_ptr<io_service> send_ioservice_ptr,
			std::shared_ptr<io_service> recv_ioservice_ptr,
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr
		)
			: m_send_ioservice_ptr(send_ioservice_ptr)
			, m_recv_ioservice_ptr(recv_ioservice_ptr)
			, m_listener_mgr_ptr(listener_mgr_ptr)
			, m_url_parser_ptr(url_parser_ptr)
		{
			if (m_send_ioservice_ptr)
			{
				m_send_strand_ptr = std::make_shared<boost::asio::io_service::strand>(*m_send_ioservice_ptr);
			}
			if (m_recv_ioservice_ptr)
			{
				m_recv_strand_ptr = std::make_shared<boost::asio::io_service::strand>(*m_recv_ioservice_ptr);
			}
		}

		/**
		 * @destruct
		 */
		virtual ~connection_impl()
		{
		}

		/**
		 * @function : start session
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
		 * @function : stop session
		 * note : this function must be noblocking,if it's blocking,will cause circle lock in session_mgr's function stop_all and stop
		 */
		virtual void stop() = 0;

		/**
		 * @function : whether the session is started
		 */
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

		
		/**
		 * @function : get user data shared_ptr
		 */
		virtual std::shared_ptr<void> get_user_data() 
		{
			return m_user_data_ptr;
		}

		/**
		 * @function : set user data shared_ptr
		 */
		virtual void set_user_data(std::shared_ptr<void> user_data_ptr) 
		{
			m_user_data_ptr = user_data_ptr;
		}

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

		/**
		 * @function : colse the socket
		 */
		virtual void _close_socket() = 0;

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

		/// user data
		std::shared_ptr<void>                m_user_data_ptr;

	};
}

#endif // !__ASIO2_CONNECTION_IMPL_HPP__
