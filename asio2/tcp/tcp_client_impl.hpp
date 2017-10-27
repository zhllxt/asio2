/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_TCP_CLIENT_IMPL_HPP__
#define __ASIO2_TCP_CLIENT_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/tcp/tcp_connection_impl.hpp>

namespace asio2
{

	template<class _connection_impl_t>
	class tcp_client_impl : public client_impl
	{
	public:

		typedef typename _connection_impl_t::pool_t pool_t;

		/**
		 * @construct
		 */
		explicit tcp_client_impl(
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr
		)
			: client_impl(listener_mgr_ptr, url_parser_ptr)
		{
		}

		/**
		 * @destruct
		 */
		virtual ~tcp_client_impl()
		{
		}

		/**
		 * @function : start the client
		 * @param    : async_connect - asynchronous connect to the server or sync
		 * @return   : true  - start successed , false - start failed
		 */
		virtual bool start(bool async_connect = true) override
		{
			try
			{
				// check if started and not stoped
				if (is_start())
				{
					assert(false);
					return false;
				}

				// first call base class start function
				if (!client_impl::start(async_connect))
					return false;

				// init service pool size 
				m_io_service_pool_evt_ptr = std::make_shared<io_service_pool>(_get_io_service_pool_size());
				m_io_service_pool_msg_ptr = std::make_shared<io_service_pool>(_get_io_service_pool_size());

				// init recv buf pool 
				m_recv_buf_pool_ptr = std::make_shared<pool_t>(_get_pool_buffer_size());

				// startup the io service thread 
				m_io_service_thread_evt_ptr = std::make_shared<std::thread>(std::bind(&io_service_pool::run, m_io_service_pool_evt_ptr));
				m_io_service_thread_msg_ptr = std::make_shared<std::thread>(std::bind(&io_service_pool::run, m_io_service_pool_msg_ptr));

				// start connect
				return _start_connect(async_connect);
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value(), e.what());
			}
			catch (std::exception & e)
			{
				set_last_error(DEFAULT_EXCEPTION_CODE, e.what());
				PRINT_EXCEPTION;
			}

			return false;
		}

		/**
		 * @function : stop client,this function may be blocking 
		 */
		virtual void stop() override
		{
			if (m_connection_impl_ptr)
			{
				// when call connection's stop,just post a socket colse event,so the stop will return immediate
				m_connection_impl_ptr->stop();
				// we wait until the connection shared_ptr is the only instance managing the current object to ensuer 
				// all async event has been processed
				while (m_connection_impl_ptr.use_count() > 1)
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}

			if (m_io_service_pool_evt_ptr)
				m_io_service_pool_evt_ptr->stop();
			if (m_io_service_pool_msg_ptr)
				m_io_service_pool_msg_ptr->stop();

			if (m_io_service_thread_evt_ptr && m_io_service_thread_evt_ptr->joinable())
				m_io_service_thread_evt_ptr->join();
			if (m_io_service_thread_msg_ptr && m_io_service_thread_msg_ptr->joinable())
				m_io_service_thread_msg_ptr->join();

			// release the buffer pool malloced memory 
			if (m_recv_buf_pool_ptr)
				m_recv_buf_pool_ptr->destroy();
		}

		/**
		 * @function : whether the client is started
		 */
		virtual bool is_start()
		{
			return (
				(m_connection_impl_ptr && m_connection_impl_ptr->is_start()) &&
				(m_io_service_thread_evt_ptr && m_io_service_thread_evt_ptr->joinable()) &&
				(m_io_service_thread_msg_ptr && m_io_service_thread_msg_ptr->joinable())
				);
		}

		/**
		 * @function : send data
		 * @param    : send_buf_ptr - std::shared_ptr<uint8_t> object
		 *             len          - data len
		 */
		virtual bool send(std::shared_ptr<uint8_t> send_buf_ptr, std::size_t len) override
		{
			return (m_connection_impl_ptr ? m_connection_impl_ptr->send(send_buf_ptr, len) : false);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(const char * buf, std::size_t len) override
		{
			return (m_connection_impl_ptr ? m_connection_impl_ptr->send(buf, len) : false);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(const char * buf) override
		{
			return (m_connection_impl_ptr ? m_connection_impl_ptr->send(buf) : false);
		}

	public:

		/**
		 * @function : get the socket shared_ptr
		 */
		inline std::shared_ptr<boost::asio::ip::tcp::socket> get_socket_ptr()
		{
			return (m_connection_impl_ptr ? m_connection_impl_ptr->get_socket_ptr() : nullptr);
		}

		/**
		 * @function : get the local address
		 */
		virtual std::string get_local_address() override
		{
			return (m_connection_impl_ptr ? m_connection_impl_ptr->get_local_address() : std::string());
		}

		/**
		 * @function : get the local port
		 */
		virtual unsigned short get_local_port() override
		{
			return (m_connection_impl_ptr ? m_connection_impl_ptr->get_local_port() : 0);
		}

		/**
		 * @function : get the remote address
		 */
		virtual std::string get_remote_address() override
		{
			return (m_connection_impl_ptr ? m_connection_impl_ptr->get_remote_address() : std::string());
		}

		/**
		 * @function : get the remote port
		 */
		virtual unsigned short get_remote_port() override
		{
			return (m_connection_impl_ptr ? m_connection_impl_ptr->get_remote_port() : 0);
		}

	protected:

		virtual void _prepare_connection()
		{
			try
			{
				this->m_connection_impl_ptr = std::make_shared<_connection_impl_t>(
					this->m_io_service_pool_evt_ptr->get_io_service_ptr(),
					this->m_io_service_pool_evt_ptr->get_io_service_ptr(),
					this->m_io_service_pool_msg_ptr->get_io_service_ptr(),
					this->m_io_service_pool_msg_ptr->get_io_service_ptr(),
					this->m_listener_mgr_ptr,
					this->m_url_parser_ptr,
					this->m_recv_buf_pool_ptr
					);
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value(), e.what());

				this->m_connection_impl_ptr.reset();
			}
		}

		virtual bool _start_connect(bool async_connect)
		{
			this->_prepare_connection();

			return (this->m_connection_impl_ptr ? this->m_connection_impl_ptr->start(async_connect) : false);
		}

	protected:

		/// the m_io_service_pool_evt thread for socket event
		std::shared_ptr<std::thread> m_io_service_thread_evt_ptr;

		/// the m_io_service_pool_msg for msg handle
		std::shared_ptr<std::thread> m_io_service_thread_msg_ptr;

		/// the io_service_pool for socket event
		io_service_pool_ptr m_io_service_pool_evt_ptr;

		/// the io_service_pool for msg handle
		io_service_pool_ptr m_io_service_pool_msg_ptr;

		/// recv buffer pool for every session
		std::shared_ptr<pool_t> m_recv_buf_pool_ptr;

		/// connection 
		std::shared_ptr<_connection_impl_t> m_connection_impl_ptr;

	};

}

#endif // !__ASIO2_TCP_CLIENT_IMPL_HPP__
