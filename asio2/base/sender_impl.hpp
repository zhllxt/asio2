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

#include <asio2/base/transmitter_impl.hpp>

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
				// check if started and not stopped
				if (this->m_transmitter_impl_ptr->m_state >= state::starting)
				{
					assert(false);
					return false;
				}

				// call stop before start
				this->stop();

				// startup the io service thread 
				this->m_io_context_pool_ptr->run();

				// start connect
				return this->m_transmitter_impl_ptr->start();
			}
			catch (asio::system_error & e)
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
			// first close the transmitter
			m_transmitter_impl_ptr->stop();

			// stop the io_context
			m_io_context_pool_ptr->stop();
		}

		/**
		 * @function : check whether the sender is started
		 */
		virtual bool is_started()
		{
			return m_transmitter_impl_ptr->is_started();
		}

		/**
		 * @function : check whether the sender is stopped
		 */
		virtual bool is_stopped()
		{
			return m_transmitter_impl_ptr->is_stopped();
		}

		/**
		 * @function : send data
		 */
		virtual bool send(const std::string & ip, unsigned short port, std::shared_ptr<buffer<uint8_t>> buf_ptr)
		{
			return m_transmitter_impl_ptr->send(ip, port, buf_ptr);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(const std::string & ip, const std::string & port, std::shared_ptr<buffer<uint8_t>> buf_ptr)
		{
			return m_transmitter_impl_ptr->send(ip, port, buf_ptr);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(const std::string & ip, unsigned short port, const uint8_t * buf, std::size_t len)
		{
			return m_transmitter_impl_ptr->send(ip, port, buf, len);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(const std::string & ip, const std::string & port, const uint8_t * buf, std::size_t len)
		{
			return m_transmitter_impl_ptr->send(ip, port, buf, len);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(const std::string & ip, unsigned short port, const char * buf)
		{
			return m_transmitter_impl_ptr->send(ip, port, buf);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(const std::string & ip, const std::string & port, const char * buf)
		{
			return m_transmitter_impl_ptr->send(ip, port, buf);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(const std::string & ip, unsigned short port, const std::string & s)
		{
			return m_transmitter_impl_ptr->send(ip, port, s);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(const std::string & ip, const std::string & port, const std::string & s)
		{
			return m_transmitter_impl_ptr->send(ip, port, s);
		}

	public:
		/**
		 * @function : get the local address
		 */
		virtual std::string get_local_address()
		{
			return m_transmitter_impl_ptr->get_local_address();
		}

		/**
		 * @function : get the local port
		 */
		virtual unsigned short get_local_port()
		{
			return m_transmitter_impl_ptr->get_local_port();
		}

		/**
		 * @function : get the remote address
		 */
		virtual std::string get_remote_address()
		{
			return m_transmitter_impl_ptr->get_remote_address();
		}

		/**
		 * @function : get the remote port
		 */
		virtual unsigned short get_remote_port()
		{
			return m_transmitter_impl_ptr->get_remote_port();
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
		std::shared_ptr<transmitter_impl>                  m_transmitter_impl_ptr;

	};
}

#endif // !__ASIO2_SENDER_IMPL_HPP__
