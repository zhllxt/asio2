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

#include <asio2/base/acceptor_impl.hpp>

namespace asio2
{

	class server_impl : public std::enable_shared_from_this<server_impl>
	{
	public:
		/**
		 * @construct
		 */
		explicit server_impl(
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
				// check if started and not stoped
				if (this->is_start())
				{
					assert(false);
					return false;
				}

				// startup the io service thread 
				m_io_context_pool_ptr->run();

				// start acceptor
				return m_acceptor_impl_ptr->start();
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());
			}

			return false;
		}

		/**
		 * @function : stop the server
		 */
		virtual void stop()
		{
			// first close the acceptor
			m_acceptor_impl_ptr->stop();

			// stop the io_context
			m_io_context_pool_ptr->stop();
		}

		/**
		 * @function : check whether the server is started 
		 */
		virtual bool is_start() { return (m_acceptor_impl_ptr->is_start()); }

		/**
		 * @function : send data
		 */
		virtual bool send(std::shared_ptr<buffer<uint8_t>> buf_ptr)
		{
			if (this->is_start())
			{
				this->m_acceptor_impl_ptr->get_session_mgr()->for_each_session([&](std::shared_ptr<session_impl> & session_ptr)
				{
					session_ptr->send(buf_ptr);
				});
				return true;
			}
			return false;
		}

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

		/**
		 * @function : get the acceptor_impl shared_ptr
		 */
		inline std::shared_ptr<acceptor_impl> get_acceptor_impl() { return m_acceptor_impl_ptr; }

		/**
		 * @function : get the listen address
		 */
		virtual std::string get_listen_address() { return this->m_acceptor_impl_ptr->get_listen_address(); }

		/**
		 * @function : get the listen port
		 */
		virtual unsigned short get_listen_port() { return this->m_acceptor_impl_ptr->get_listen_port(); }

		/**
		 * @function : get connected session count
		 */
		virtual std::size_t get_session_count()  { return this->m_acceptor_impl_ptr->get_session_mgr()->get_session_count(); }

		/**
		 * @function : 
		 * @param    : the user callback function,the session shared_ptr will pass to the function as a param,
		 *             the callback like this : void handler(asio2::session_ptr & session_ptr){...}
		 */
		virtual bool for_each_session(const std::function<void(std::shared_ptr<session_impl> & session_ptr)> & handler)
		{
			if (this->is_start())
			{
				this->m_acceptor_impl_ptr->get_session_mgr()->for_each_session(handler);
				return true;
			}
			return false;
		}

	protected:
		virtual std::size_t _get_io_context_pool_size()
		{
			// get io_context_pool_size from the url
			std::size_t size = std::thread::hardware_concurrency();
			auto val = m_url_parser_ptr->get_param_value("io_context_pool_size");
			if (!val.empty())
				size = static_cast<std::size_t>(std::strtoull(val.data(), nullptr, 10));
			if (size == 0)
				size = std::thread::hardware_concurrency();
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
		std::shared_ptr<acceptor_impl>                     m_acceptor_impl_ptr;
	};
}

#endif // !__ASIO2_SERVER_IMPL_HPP__
