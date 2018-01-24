/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_HTTP_CONNECTION_IMPL_HPP__
#define __ASIO2_HTTP_CONNECTION_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/http/http_response_parser.hpp>

#include <asio2/tcp/tcp_connection_impl.hpp>

namespace asio2
{

	class http_connection_impl : public tcp_connection_impl
	{
	public:
		/**
		 * @construct
		 */
		explicit http_connection_impl(
			std::shared_ptr<url_parser>       url_parser_ptr,
			std::shared_ptr<listener_mgr>     listener_mgr_ptr,
			std::shared_ptr<asio::io_context> send_io_context_ptr,
			std::shared_ptr<asio::io_context> recv_io_context_ptr
		)
			: tcp_connection_impl(
				url_parser_ptr,
				listener_mgr_ptr,
				send_io_context_ptr,
				recv_io_context_ptr
			)
		{
		}

		/**
		 * @destruct
		 */
		virtual ~http_connection_impl()
		{
		}

		virtual bool start(bool async_connect = true) override
		{
			m_response_parser.reset();

			return tcp_connection_impl::start(async_connect);
		}

		virtual void stop() override
		{
			tcp_connection_impl::stop();
		}

	protected:
		virtual void _handle_recv(const asio::error_code & ec, std::size_t bytes_recvd, std::shared_ptr<connection_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> buf_ptr) override
		{
			if (!ec)
			{
				// every times recv data,we update the last active time.
				this->reset_last_active_time();

				if (bytes_recvd == 0)
				{
					// recvd data len is 0,may be heartbeat packet.
				}
				else if (bytes_recvd > 0)
				{
					buf_ptr->write_bytes(bytes_recvd);
				}

				//if (!m_response_ptr)
				//	m_response_ptr = std::make_shared<http_response>();

				//http_response_parser::status ret = m_response_parser.parse(buf_ptr, m_response_ptr);
				//if /**/ (ret == http_response_parser::status::success)
				//{
				//	std::string sss((const char *)buf_ptr->read_begin(), buf_ptr->size());

				//	_fire_recv(m_response_ptr);
				//}
				//else if (ret == http_response_parser::status::indeterminate)
				//{
				//	this->_post_recv(this_ptr, m_response_ptr);

				//	return;
				//}
				//else if (ret == http_response_parser::status::fail)
				//{
				//	set_last_error(m_response_parser.get_http_errno());

				//	this->stop();

				//	return;
				//}

				//std::string sss((const char *)buf_ptr->read_begin(), buf_ptr->size());

				//this->_post_recv(this_ptr);
			}
			else
			{
				set_last_error(ec.value());

				// close this session
				this->stop();
			}

			// No new asynchronous operations are started. This means that all shared_ptr
			// references to the connection object will disappear and the object will be
			// destroyed automatically after this handler returns. The connection class's
			// destructor closes the socket.
		}

		/// must override all listener functions,and cast the m_listener_mgr_ptr to http_server_listener_mgr,
		/// otherwise it will crash when these listener was called.
	protected:
		virtual void _fire_connect(int error) override
		{
			static_cast<http_client_listener_mgr *>(m_listener_mgr_ptr.get())->notify_connect(error);
		}

		virtual void _fire_recv(std::shared_ptr<http_response> & response_ptr)
		{
			static_cast<http_client_listener_mgr *>(m_listener_mgr_ptr.get())->notify_recv(response_ptr);
		}

		virtual void _fire_send(std::shared_ptr<http_request> & request_ptr, int error)
		{
			static_cast<http_client_listener_mgr *>(m_listener_mgr_ptr.get())->notify_send(request_ptr, error);
		}

		virtual void _fire_close(int error) override
		{
			if (!m_fire_close_is_called.test_and_set(std::memory_order_acquire))
			{
				static_cast<http_client_listener_mgr *>(m_listener_mgr_ptr.get())->notify_close(error);
			}
		}

	protected:
		/// http parser
		http_response_parser           m_response_parser;

		std::shared_ptr<http_response> m_response_ptr;
	};

}

#endif // !__ASIO2_HTTP_CONNECTION_IMPL_HPP__
