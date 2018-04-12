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

#include <asio2/base/connection_impl.hpp>

namespace asio2
{

	class client_impl : public std::enable_shared_from_this<client_impl>
	{
	public:
		/**
		 * @construct
		 */
		client_impl(
			std::shared_ptr<url_parser>                    url_parser_ptr,
			std::shared_ptr<listener_mgr>                  listener_mgr_ptr
		)
			: m_url_parser_ptr  (url_parser_ptr)
			, m_listener_mgr_ptr(listener_mgr_ptr)
		{
			m_io_context_pool_ptr = std::make_shared<io_context_pool>(_get_io_context_pool_size());
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
			try
			{
				// check if started and not stopped
				if (this->m_connection_impl_ptr->m_state >= state::starting)
				{
					assert(false);
					return false;
				}

				// call stop before start
				this->stop();

				// startup the io service thread 
				this->m_io_context_pool_ptr->run();

				// start connect
				return this->m_connection_impl_ptr->start(async_connect);
			}
			catch (asio::system_error & e)
			{
				set_last_error(e.code().value());
			}

			return false;
		}

		/**
		 * @function : stop the client
		 */
		virtual void stop()
		{
			// first close the connection
			m_connection_impl_ptr->stop();

			// stop the io_context
			m_io_context_pool_ptr->stop();
		}

		/**
		 * @function : check whether the client is started
		 */
		virtual bool is_started()
		{
			return m_connection_impl_ptr->is_started();
		}

		/**
		 * @function : check whether the client is stopped
		 */
		virtual bool is_stopped()
		{
			return m_connection_impl_ptr->is_stopped();
		}

		/**
		 * @function : send data
		 */
		virtual bool send(std::shared_ptr<buffer<uint8_t>> buf_ptr)
		{
			return m_connection_impl_ptr->send(std::move(buf_ptr));
		}

		/**
		 * @function : send data
		 */
		virtual bool send(const uint8_t * buf, std::size_t len)
		{
			return m_connection_impl_ptr->send(buf, len);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(const char * buf)
		{
			return m_connection_impl_ptr->send(buf);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(const std::string & s)
		{
			return m_connection_impl_ptr->send(s);
		}

	public:
		/**
		 * @function : get the local address
		 */
		virtual std::string get_local_address()
		{
			return m_connection_impl_ptr->get_local_address();
		}

		/**
		 * @function : get the local port
		 */
		virtual unsigned short get_local_port()
		{
			return m_connection_impl_ptr->get_local_port();
		}

		/**
		 * @function : get the remote address
		 */
		virtual std::string get_remote_address()
		{
			return m_connection_impl_ptr->get_remote_address();
		}

		/**
		 * @function : get the remote port
		 */
		virtual unsigned short get_remote_port()
		{
			return m_connection_impl_ptr->get_remote_port();
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

		/// acceptor_impl pointer,this object must be created after server has created
		std::shared_ptr<connection_impl>                   m_connection_impl_ptr;

	};
}

#endif // !__ASIO2_CLIENT_IMPL_HPP__
