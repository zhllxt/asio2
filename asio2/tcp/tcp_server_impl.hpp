/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_TCP_SERVER_IMPL_HPP__
#define __ASIO2_TCP_SERVER_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)


#include <memory>
#include <thread>
#include <atomic>

#include <asio2/base/server_impl.hpp>
#include <asio2/base/io_service_pool.hpp>
#include <asio2/base/listener_mgr.hpp>

#include <asio2/tcp/tcp_acceptor_impl.hpp>

namespace asio2
{

	template<class _acceptor_impl_t>
	class tcp_server_impl : public server_impl
	{
	public:

		typedef _acceptor_impl_t acceptor_impl_t;
		typedef typename _acceptor_impl_t::session_impl_t session_impl_t;
		typedef typename _acceptor_impl_t::pool_t pool_t;

		/**
		 * @construct
		 */
		tcp_server_impl(
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr
		)
			: server_impl(listener_mgr_ptr, url_parser_ptr)
		{
		}

		/**
		 * @destruct
		 */
		virtual ~tcp_server_impl()
		{
		}

		/**
		 * @function : start the server
		 * @return   : true - start successed , false - start failed
		 */
		virtual bool start() override
		{
			try
			{
				// check if started and not stoped
				if (is_start())
				{
					assert(false);
					return false;
				}

				// call base start function first
				if (!server_impl::start())
					return false;

				// init buf pool 
				m_recv_buf_pool_ptr = std::make_shared<pool_t>(_get_pool_buffer_size());
				m_send_buf_pool_ptr = std::make_shared<pool_s>();

				// init service pool size  and startup the io service thread 
				m_ioservice_pool_ptr = std::make_shared<io_service_pool>(_get_io_service_pool_size());
				m_ioservice_thread_ptr = std::make_shared<std::thread>(std::bind(&io_service_pool::run, m_ioservice_pool_ptr));

				// start listen
				return _start_listen();
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());
			}
			catch (std::exception &)
			{
			}

			return false;
		}

		/**
		 * @function : stop the server
		 */
		virtual void stop() override
		{
			// first close the acceptor
			if (m_acceptor_impl_ptr)
				m_acceptor_impl_ptr->stop();

			// stop the io_service
			if (m_ioservice_pool_ptr)
				m_ioservice_pool_ptr->stop();

			// wait for the io_service thread finish
			if (m_ioservice_thread_ptr && m_ioservice_thread_ptr->joinable())
				m_ioservice_thread_ptr->join();

			// release the buffer pool malloced memory 
			if (m_send_buf_pool_ptr)
				m_send_buf_pool_ptr->destroy();
			if (m_recv_buf_pool_ptr)
				m_recv_buf_pool_ptr->destroy();
		}

		/**
		 * @function : get whether the client is started
		 */
		virtual bool is_start() override
		{
			return (
				(m_acceptor_impl_ptr && m_acceptor_impl_ptr->is_start()) &&
				(m_ioservice_thread_ptr && m_ioservice_thread_ptr->joinable())
				);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(std::shared_ptr<buffer<uint8_t>> send_buf_ptr) override
		{
			if (is_start())
			{
				m_acceptor_impl_ptr->get_session_mgr_ptr()->for_each_session([send_buf_ptr](std::shared_ptr<session_impl_t> session_ptr)
				{
					session_ptr->send(send_buf_ptr);
				});
				return true;
			}
			return false;
		}

		/**
		 * @function : send data
		 */
		virtual bool send(const uint8_t * buf, std::size_t len) override
		{
			if (is_start())
			{
				m_acceptor_impl_ptr->get_session_mgr_ptr()->for_each_session([buf, len](std::shared_ptr<session_impl_t> session_ptr)
				{
					session_ptr->send(buf, len);
				});
				return true;
			}
			return false;
		}

		/**
		 * @function : get the listen address
		 */
		virtual std::string get_listen_address() override
		{
			return (m_acceptor_impl_ptr ? m_acceptor_impl_ptr->get_listen_address() : std::string());
		}

		/**
		 * @function : get the listen port
		 */
		virtual unsigned short get_listen_port() override
		{
			return (m_acceptor_impl_ptr ? m_acceptor_impl_ptr->get_listen_port() : 0);
		}

		/**
		 * @function : get connected session count
		 */
		virtual std::size_t get_session_count() override
		{
			return ((is_start()) ? m_acceptor_impl_ptr->get_session_mgr_ptr()->get_connected_session_count() : 0);
		}

		/**
		 * @function : get the acceptor impl shared_ptr
		 */
		inline std::shared_ptr<_acceptor_impl_t> get_acceptor_impl_ptr()
		{
			return m_acceptor_impl_ptr;
		}

		/**
		 * @function : 
		 * @param    : the user callback function,the session shared_ptr will pass to the function as a param,
		 *             the callback like this : void handler(std::shared_ptr<asio2::session> session_ptr){...}
		 */
		template<typename _handler>
		bool for_each_session(_handler handler)
		{
			if (is_start())
			{
				m_acceptor_impl_ptr->get_session_mgr_ptr()->for_each_session(handler);
				return true;
			}
			return false;
		}

	protected:

		virtual bool _start_listen()
		{
			try
			{
				m_acceptor_impl_ptr = std::make_shared<_acceptor_impl_t>(
					m_ioservice_pool_ptr,
					m_listener_mgr_ptr,
					m_url_parser_ptr,
					m_send_buf_pool_ptr,
					m_recv_buf_pool_ptr
					);

				return m_acceptor_impl_ptr->start();
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());
			}

			return false;
		}

	protected:
		/// the m_io_service_pool_evt thread for socket event
		std::shared_ptr<std::thread> m_ioservice_thread_ptr;

		/// the io_service_pool for socket event
		io_service_pool_ptr m_ioservice_pool_ptr;

		/// send buffer pool 
		std::shared_ptr<pool_s> m_send_buf_pool_ptr;

		/// recv buffer pool for every session
		std::shared_ptr<pool_t> m_recv_buf_pool_ptr;

		/// acceptor to recv client connection
		std::shared_ptr<_acceptor_impl_t> m_acceptor_impl_ptr;

	};

}

#endif // !__ASIO2_TCP_SERVER_IMPL_HPP__
