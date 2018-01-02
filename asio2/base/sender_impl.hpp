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

#if !defined(NDEBUG) && !defined(DEBUG) && !defined(_DEBUG)
#	define NDEBUG
#endif

#include <cassert>
#include <memory>
#include <chrono>
#include <future>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>

#include <boost/asio.hpp>
#include <boost/system/system_error.hpp>

#include <asio2/util/buffer.hpp>
#include <asio2/util/def.hpp>
#include <asio2/util/helper.hpp>
#include <asio2/util/logger.hpp>
#include <asio2/util/pool.hpp>
#include <asio2/util/rwlock.hpp>
#include <asio2/util/spin_lock.hpp>

#include <asio2/base/socket_helper.hpp>
#include <asio2/base/io_context_pool.hpp>
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
			std::shared_ptr<url_parser>              url_parser_ptr,
			std::shared_ptr<listener_mgr>            listener_mgr_ptr
		)
			: m_url_parser_ptr     (url_parser_ptr)
			, m_listener_mgr_ptr   (listener_mgr_ptr)
		{
			m_io_context_pool_ptr = std::make_shared<io_context_pool>(_get_io_context_pool_size());

			m_send_io_context_ptr = m_io_context_pool_ptr->get_io_context_ptr();
			m_recv_io_context_ptr = m_io_context_pool_ptr->get_io_context_ptr();

			if (m_send_io_context_ptr)
			{
				m_send_strand_ptr = std::make_shared<boost::asio::io_context::strand>(*m_send_io_context_ptr);
			}
			if (m_recv_io_context_ptr)
			{
				m_recv_strand_ptr = std::make_shared<boost::asio::io_context::strand>(*m_recv_io_context_ptr);
			}
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
			try
			{
				// check if started and not stoped
				if (this->is_start())
				{
					assert(false);
					return false;
				}

				// startup the io service thread 
				m_io_context_pool_ptr->run();

				return true;
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());
			}

			return false;
		}

		/**
		 * @function : stop the sender
		 */
		virtual void stop()
		{
			// stop the io_context
			m_io_context_pool_ptr->stop();
		}

		/**
		 * @function : check whether the sender is started
		 */
		virtual bool is_start() = 0;

		/**
		 * @function : send data
		 */
		virtual bool send(std::string & ip, unsigned short port, std::shared_ptr<buffer<uint8_t>> buf_ptr)
		{
			auto sport = format("%u", port);
			return ((!ip.empty() && buf_ptr) ?
				this->send(ip, sport, buf_ptr) : false);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(std::string & ip, std::string & port, std::shared_ptr<buffer<uint8_t>> buf_ptr) = 0;

		/**
		 * @function : send data
		 */
		virtual bool send(std::string & ip, unsigned short port, const uint8_t * buf, std::size_t len)
		{
			auto sport = format("%u", port);
			return ((!ip.empty() && buf) ?
				this->send(ip, sport, buf, len) : false);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(std::string & ip, std::string & port, const uint8_t * buf, std::size_t len)
		{
			return ((!ip.empty() && !port.empty() && buf) ?
				this->send(ip, port, std::make_shared<buffer<uint8_t>>(len, malloc_send_buffer(len), buf, len)) : false);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(std::string & ip, unsigned short port, const char * buf)
		{
			auto sport = format("%u", port);
			return ((!ip.empty() && buf) ?
				this->send(ip, sport, reinterpret_cast<const uint8_t *>(buf), std::strlen(buf)) : false);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(std::string & ip, std::string & port, const char * buf)
		{
			return ((!ip.empty() && !port.empty() && buf) ?
				this->send(ip, port, reinterpret_cast<const uint8_t *>(buf), std::strlen(buf)) : false);
		}

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
		/**
		 * @function : get last active time 
		 */
		std::chrono::time_point<std::chrono::system_clock> get_last_active_time()
		{
			return m_last_active_time;
		}

		/**
		 * @function : reset last active time 
		 */
		void reset_last_active_time()
		{
			m_last_active_time = std::chrono::system_clock::now();
		}

		/**
		 * @function : get silence duration of milliseconds
		 */
		std::chrono::milliseconds::rep get_silence_duration()
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_last_active_time).count();
		}

	protected:
		virtual std::size_t _get_io_context_pool_size()
		{
			// get io_context_pool_size from the url
			std::size_t size = 2;
			auto val = m_url_parser_ptr->get_param_value("io_context_pool_size");
			if (!val.empty())
				size = static_cast<std::size_t>(std::strtoull(val.data(), nullptr, 10));
			if (size == 0)
				size = 2;
			return size;
		}

	protected:
		/// url parser
		std::shared_ptr<url_parser>                        m_url_parser_ptr;

		/// listener manager
		std::shared_ptr<listener_mgr>                      m_listener_mgr_ptr;

		/// the io_context_pool for socket event
		std::shared_ptr<io_context_pool>                   m_io_context_pool_ptr;

		/// The io_context used to handle the socket event.
		std::shared_ptr<boost::asio::io_context>           m_send_io_context_ptr;

		/// The io_context used to handle the socket event.
		std::shared_ptr<boost::asio::io_context>           m_recv_io_context_ptr;

		/// The strand used to handle the socket event.
		std::shared_ptr<boost::asio::io_context::strand>   m_send_strand_ptr;

		/// The strand used to handle the socket event.
		std::shared_ptr<boost::asio::io_context::strand>   m_recv_strand_ptr;

		/// use to check whether the user call stop in the listener
		volatile state                                     m_state = state::stopped;

		/// last active time 
		std::chrono::time_point<std::chrono::system_clock> m_last_active_time = std::chrono::system_clock::now();

	};
}

#endif // !__ASIO2_SENDER_IMPL_HPP__
