/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_SESSION_IMPL_HPP__
#define __ASIO2_SESSION_IMPL_HPP__

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

#include <asio/asio.hpp>
#include <asio/system_error.hpp>

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
#include <asio2/base/session_mgr.hpp>

namespace asio2
{

	class session_impl : public std::enable_shared_from_this<session_impl>
	{
		template<class _key, class _hasher, class _equaler> friend class session_mgr_t;

	public:
		/**
		 * @construct
		 */
		session_impl(
			std::shared_ptr<url_parser>            url_parser_ptr,
			std::shared_ptr<listener_mgr>          listener_mgr_ptr,
			std::shared_ptr<asio::io_context>      io_context_ptr,
			std::shared_ptr<session_mgr>           session_mgr_ptr
		)
			: m_url_parser_ptr   (url_parser_ptr)
			, m_listener_mgr_ptr (listener_mgr_ptr)
			, m_io_context_ptr   (io_context_ptr)
			, m_timer            (*io_context_ptr)
			, m_session_mgr_ptr  (session_mgr_ptr)
		{
			if (m_io_context_ptr)
			{
				m_strand_ptr = std::make_shared<asio::io_context::strand>(*m_io_context_ptr);
			}
		}

		/**
		 * @destruct
		 */
		virtual ~session_impl()
		{
		}

		/**
		 * @function : start session
		 */
		virtual bool start() = 0;

		/**
		 * @function : stop session
		 * note : this function must be noblocking,if it's blocking,will cause circle lock in session_mgr's function stop_all and stop
		 */
		virtual void stop() = 0;

		/**
		 * @function : check whether the session is started
		 */
		virtual bool is_started() = 0;

		/**
		 * @function : check whether the server is stopped
		 */
		virtual bool is_stopped() = 0;

		/**
		 * @function : send data
		 */
		virtual bool send(std::shared_ptr<buffer<uint8_t>> buf_ptr) = 0;

		/**
		 * @function : send data
		 */
		virtual bool send(const uint8_t * buf, std::size_t len)
		{
			return (buf ? this->send(std::make_shared<buffer<uint8_t>>(len, malloc_send_buffer(len), buf, len)) : false);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(const char * buf)
		{
			return (buf ? this->send(reinterpret_cast<const uint8_t *>(buf), std::strlen(buf)) : false);
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
		
		/**
		 * @function : get user data 
		 */
		inline std::size_t get_user_data()
		{
			return m_user_data;
		}

		/**
		 * @function : set user data
		 */
		inline void set_user_data(std::size_t user_data)
		{
			m_user_data = user_data;
		}

	public:
		/**
		 * @function : get last active time 
		 */
		inline std::chrono::time_point<std::chrono::system_clock> get_last_active_time()
		{
			return m_last_active_time;
		}

		/**
		 * @function : reset last active time 
		 */
		inline void reset_last_active_time()
		{
			m_last_active_time = std::chrono::system_clock::now();
		}

		/**
		 * @function : get silence duration of milliseconds
		 */
		inline std::chrono::milliseconds::rep get_silence_duration()
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_last_active_time).count();
		}

		/**
		 * @function : get build connection time 
		 */
		inline std::chrono::time_point<std::chrono::system_clock> get_connect_time()
		{
			return m_connect_time;
		}

		/**
		 * @function : get connection duration of milliseconds
		 */
		inline std::chrono::milliseconds::rep get_connect_duration()
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_connect_time).count();
		}

	protected:
		/**
		 * @function : used for the unorder_map key
		 */
		virtual void * _get_key() = 0;

	protected:
		/// asio::strand ,used to ensure socket multi thread safe,we must ensure that only one operator
		/// can operate the same socket at the same time,and strand can enuser that the event will
		/// be processed in the order of post, eg : strand.post(1);strand.post(2); the 2 will processed
		/// certaion after the 1,if 1 is block,the 2 won't be processed,util the 1 is processed completed
		/// more details see : http://bbs.csdn.net/topics/390931471

		/// url parser
		std::shared_ptr<url_parser>                        m_url_parser_ptr;

		/// listener manager
		std::shared_ptr<listener_mgr>                      m_listener_mgr_ptr;

		/// The io_context used to handle the socket event.
		std::shared_ptr<asio::io_context>                  m_io_context_ptr;

		/// The strand used to handle the socket event.
		std::shared_ptr<asio::io_context::strand>          m_strand_ptr;

		/// timer for session silence time out
		asio::steady_timer                                 m_timer;

		/// user data
		std::size_t                                        m_user_data         = 0;

		/// use to check whether the user call stop in the listener
		volatile state                                     m_state             = state::stopped;

		/// session_mgr interface pointer,this object must be created after acceptor has created
		std::shared_ptr<session_mgr>                       m_session_mgr_ptr;

		/// last active time 
		std::chrono::time_point<std::chrono::system_clock> m_last_active_time  = std::chrono::system_clock::now();

		/// build connection time
		std::chrono::time_point<std::chrono::system_clock> m_connect_time      = std::chrono::system_clock::now();

	};

	using session     = session_impl;
	using session_ptr = std::shared_ptr<session_impl>;


}

#endif // !__ASIO2_SESSION_IMPL_HPP__
