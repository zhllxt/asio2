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

#include <asio2/base/session_impl.hpp>
#include <asio2/base/session_mgr_impl.hpp>

namespace asio2
{

	class acceptor_impl : public std::enable_shared_from_this<acceptor_impl>
	{
		friend class server_impl;

	public:
		/**
		 * @construct
		 */
		acceptor_impl(
			std::shared_ptr<url_parser>                    url_parser_ptr,
			std::shared_ptr<listener_mgr>                  listener_mgr_ptr,
			std::shared_ptr<io_context_pool>               io_context_pool_ptr
		)
			: m_url_parser_ptr     (url_parser_ptr)
			, m_listener_mgr_ptr   (listener_mgr_ptr)
			, m_io_context_pool_ptr(io_context_pool_ptr)
		{
			m_io_context_ptr = m_io_context_pool_ptr->get_io_context_ptr();
			m_strand_ptr     = std::make_shared<boost::asio::io_context::strand>(*m_io_context_ptr);
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
		virtual bool start() = 0;

		/**
		 * @function : stop acceptor
		 * note : this function must be noblocking,if it's blocking,will cause circle lock in session_mgr's function stop_all and stop
		 */
		virtual void stop() = 0;

		/**
		 * @function : check whether the acceptor is started
		 */
		virtual bool is_started() = 0;

		/**
		 * @function : check whether the acceptor is stopped
		 */
		virtual bool is_stopped() = 0;

		/**
		 * @function : get the listen address
		 */
		virtual std::string get_listen_address() = 0;

		/**
		 * @function : get the listen port
		 */
		virtual unsigned short get_listen_port() = 0;

		/**
		 * @function : get the session_mgr pointer
		 */
		inline std::shared_ptr<session_mgr> get_session_mgr() { return m_session_mgr_ptr; }

	protected:
		/// url parser
		std::shared_ptr<url_parser>                        m_url_parser_ptr;

		/// listener manager
		std::shared_ptr<listener_mgr>                      m_listener_mgr_ptr;

		/// The io_context used to handle the socket event.
		std::shared_ptr<boost::asio::io_context>           m_io_context_ptr;

		/// The strand used to handle the socket event.
		std::shared_ptr<boost::asio::io_context::strand>   m_strand_ptr;

		/// the io_context_pool reference for socket event
		std::shared_ptr<io_context_pool>                   m_io_context_pool_ptr;

		/// session_mgr interface pointer,this object must be created after acceptor has created
		std::shared_ptr<session_mgr>                       m_session_mgr_ptr;

	};
}

#endif // !__ASIO2_ACCEPTOR_IMPL_HPP__
