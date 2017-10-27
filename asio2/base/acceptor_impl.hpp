/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 *
 */

#ifndef __ASIO2_ACCEPTOR_IMPL_HPP__
#define __ASIO2_ACCEPTOR_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <chrono>
#include <atomic>
#include <future>
#include <functional>

#include <boost/asio.hpp>
#include <boost/system/system_error.hpp>

#include <asio2/util/pool.hpp>

#include <asio2/base/error.hpp>
#include <asio2/base/io_service_pool.hpp>
#include <asio2/base/listener_mgr.hpp>
#include <asio2/base/url_parser.hpp>

namespace asio2
{

	class acceptor_impl : public std::enable_shared_from_this<acceptor_impl>
	{
	public:

		/**
		 * @construct
		 */
		acceptor_impl(
			io_service_ptr ioservice_ptr,
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr
		)
			: m_io_service_ptr(ioservice_ptr)
			, m_listener_mgr_ptr(listener_mgr_ptr)
			, m_url_parser_ptr(url_parser_ptr)
		{
			if (m_io_service_ptr)
			{
				m_strand_ptr = std::make_shared<boost::asio::io_service::strand>(*m_io_service_ptr);
			}
		}

		/**
		 * @destruct
		 */
		virtual ~acceptor_impl()
		{
		}

		/**
		 * @function : start acceptor
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
		 * @function : stop acceptor
		 * note : this function must be noblocking,if it's blocking,will cause circle lock in session_mgr's function stop_all and stop
		 */
		virtual void stop() = 0;

		/**
		 * @function : test whether the acceptor is started
		 */
		virtual bool is_start() = 0;

		/**
		 * @function : get the strand shared_ptr
		 */
		inline std::shared_ptr<boost::asio::io_service::strand> get_strand_ptr()
		{
			return m_strand_ptr;
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
		 * @function : get send pending packet size
		 */
		virtual std::size_t get_send_pending() { return static_cast<std::size_t>(0); }

		/**
		 * @function : get recv pending packet size
		 */
		virtual std::size_t get_recv_pending() { return static_cast<std::size_t>(0); }

	protected:

		/// hold the io_service shared_ptr,make sure the io_service is destroy after current object
		io_service_ptr m_io_service_ptr;

		/// asio's strand to ensure asio.socket multi thread safe
		std::shared_ptr<boost::asio::io_service::strand> m_strand_ptr;

		/// listener manager shared_ptr
		std::shared_ptr<listener_mgr>        m_listener_mgr_ptr;

		/// url parser shared_ptr
		std::shared_ptr<url_parser>          m_url_parser_ptr;

	};
}

#endif // !__ASIO2_ACCEPTOR_IMPL_HPP__
