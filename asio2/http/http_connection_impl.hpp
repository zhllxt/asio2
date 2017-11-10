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

	template<class _pool_t>
	class http_connection_impl : public tcp_connection_impl<_pool_t>
	{
	public:

		typedef _pool_t pool_t;

		/**
		 * @construct
		 */
		explicit http_connection_impl(
			std::shared_ptr<io_service> send_ioservice_ptr,
			std::shared_ptr<io_service> recv_ioservice_ptr,
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr,
			std::shared_ptr<pool_s> send_buf_pool_ptr,
			std::shared_ptr<pool_t> recv_buf_pool_ptr
		)
			: tcp_connection_impl<_pool_t>(
				send_ioservice_ptr,
				recv_ioservice_ptr,
				listener_mgr_ptr,
				url_parser_ptr,
				send_buf_pool_ptr,
				recv_buf_pool_ptr
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

			return tcp_connection_impl<_pool_t>::start(async_connect);
		}

		virtual void stop() override
		{
			tcp_connection_impl<_pool_t>::stop();
		}

		http_connection_impl & set_request_pool(std::shared_ptr<object_pool<http_request>> request_pool_ptr)
		{
			//m_request_pool_ptr = request_pool_ptr;
			return (*this);
		}

		http_connection_impl & set_response_pool(std::shared_ptr<object_pool<http_response>> response_pool_ptr)
		{
			m_response_pool_ptr = response_pool_ptr;
			return (*this);
		}

	protected:
		virtual void _post_recv(std::shared_ptr<connection_impl> this_ptr) override
		{
			_post_recv(this_ptr, nullptr);
		}

		virtual void _post_recv(std::shared_ptr<connection_impl> this_ptr, std::shared_ptr<http_response> response_ptr)
		{
			if (is_start())
			{
				// every times post recv event,we get the recv buffer from the buffer pool
				std::shared_ptr<buffer<uint8_t>> recv_buf_ptr = m_recv_buf_pool_ptr->get(0);

				m_socket_ptr->async_read_some(
					boost::asio::buffer(recv_buf_ptr->data(), recv_buf_ptr->capacity()),
					m_recv_strand_ptr->wrap(std::bind(&http_connection_impl::_handle_recv, std::static_pointer_cast<http_connection_impl>(this_ptr),
						std::placeholders::_1, // error_code
						std::placeholders::_2, // bytes_recvd
						this_ptr,
						recv_buf_ptr,
						response_ptr
					)));
			}
		}

		virtual void _handle_recv(const boost::system::error_code& ec, std::size_t bytes_recvd, std::shared_ptr<connection_impl> this_ptr, 
			std::shared_ptr<buffer<uint8_t>> recv_buf_ptr, std::shared_ptr<http_response> response_ptr)
		{
			if (!ec)
			{
				// every times recv data,we update the last active time.
				reset_last_active_time();

				if (bytes_recvd == 0)
				{
					// recvd data len is 0,may be heartbeat packet.
				}
				else if (bytes_recvd > 0)
				{
					recv_buf_ptr->resize(bytes_recvd);
				}

				if (!response_ptr)
					response_ptr = m_response_pool_ptr->get();

				http_response_parser::status ret = m_response_parser.parse(recv_buf_ptr, response_ptr);
				if /**/ (ret == http_response_parser::status::success)
				{
					std::string sss((const char *)recv_buf_ptr->data(), recv_buf_ptr->size());

					_fire_recv(response_ptr);
				}
				else if (ret == http_response_parser::status::indeterminate)
				{
					this->_post_recv(this_ptr, response_ptr);

					return;
				}
				else if (ret == http_response_parser::status::fail)
				{
					set_last_error(m_response_parser.get_http_errno());

					_fire_close(m_response_parser.get_http_errno());

					return;
				}

				std::string sss((const char *)recv_buf_ptr->data(), recv_buf_ptr->size());

				this->_post_recv(this_ptr);
			}
			else
			{
				set_last_error(ec.value());

				// close this session
				_fire_close(ec.value());
			}

			// No new asynchronous operations are started. This means that all shared_ptr
			// references to the connection object will disappear and the object will be
			// destroyed automatically after this handler returns. The connection class's
			// destructor closes the socket.
		}

		virtual void _post_send(std::shared_ptr<buffer<uint8_t>> send_buf_ptr)
		{
			if (is_start())
			{
				boost::system::error_code ec;
				boost::asio::write(*m_socket_ptr, boost::asio::buffer(send_buf_ptr->data(), send_buf_ptr->size()), ec);
				set_last_error(ec.value());
				//_fire_send(send_buf_ptr, ec.value());

				if (ec)
				{
					PRINT_EXCEPTION;

					_fire_close(ec.value());
				}
			}
		}

		/// must override all listener functions,and cast the m_listener_mgr_ptr to http_server_listener_mgr,
		/// otherwise it will crash when these listener was called.
	protected:
		virtual void _fire_connect(int error) override
		{
			std::dynamic_pointer_cast<http_client_listener_mgr>(m_listener_mgr_ptr)->notify_connect(error);
		}

		virtual void _fire_recv(std::shared_ptr<http_response> response_ptr)
		{
			std::static_pointer_cast<http_client_listener_mgr>(m_listener_mgr_ptr)->notify_recv(response_ptr);
		}

		virtual void _fire_send(std::shared_ptr<http_request> request_ptr, int error)
		{
			std::static_pointer_cast<http_client_listener_mgr>(m_listener_mgr_ptr)->notify_send(request_ptr, error);
		}

		virtual void _fire_close(int error) override
		{
			if (!m_fire_close_is_called.test_and_set(std::memory_order_acquire))
			{
				std::dynamic_pointer_cast<http_client_listener_mgr>(m_listener_mgr_ptr)->notify_close(error);

				_close_socket();
			}
		}

	protected:
		http_response_parser m_response_parser;

		std::shared_ptr<object_pool<http_response>> m_response_pool_ptr;

	};

}

#endif // !__ASIO2_HTTP_CONNECTION_IMPL_HPP__
