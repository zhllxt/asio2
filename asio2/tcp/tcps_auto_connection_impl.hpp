/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_TCPS_AUTO_CONNECTION_IMPL_HPP__
#define __ASIO2_TCPS_AUTO_CONNECTION_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/tcp/tcps_connection_impl.hpp>

namespace asio2
{

	class tcps_auto_connection_impl : public tcps_connection_impl
	{
	public:

		/**
		 * @construct
		 */
		explicit tcps_auto_connection_impl(
			std::shared_ptr<url_parser>         url_parser_ptr,
			std::shared_ptr<listener_mgr>       listener_mgr_ptr,
			std::shared_ptr<asio::io_context>   send_io_context_ptr,
			std::shared_ptr<asio::io_context>   recv_io_context_ptr,
			std::shared_ptr<asio::ssl::context> ssl_context_ptr = nullptr
		)
			: tcps_connection_impl(
				url_parser_ptr,
				listener_mgr_ptr,
				send_io_context_ptr,
				recv_io_context_ptr,
				ssl_context_ptr
			)
		{
		}

		/**
		 * @destruct
		 */
		virtual ~tcps_auto_connection_impl()
		{
		}

		virtual bool start(bool async_connect = true) override
		{
			// reset the variable to default status
			m_body_len = 0;
			m_header = 0;
			m_recv_is_header = true;

			return tcps_connection_impl::start(async_connect);
		}

	protected:
		virtual void _post_recv(std::shared_ptr<connection_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> buf_ptr) override
		{
			if (this->is_started())
			{
				if (m_recv_is_header)
				{
					m_header = 0;
					// This function is used to asynchronously read a certain number of bytes of data from a stream.
					asio::async_read(this->m_socket,
						asio::buffer((void *)(&m_header), sizeof(m_header)),
						this->m_recv_strand_ptr->wrap(std::bind(&tcps_auto_connection_impl::_handle_recv, this,
							std::placeholders::_1, // error_code
							std::placeholders::_2, // bytes_recvd
							std::move(this_ptr),
							std::move(buf_ptr)
						)));
				}
				else
				{
					if (buf_ptr->remain() >= static_cast<std::size_t>(m_body_len))
					{
						const auto & buffer = asio::buffer(buf_ptr->write_begin(), m_body_len);
						// This function is used to asynchronously read a certain number of bytes of data from a stream.
						asio::async_read(this->m_socket, buffer,
							this->m_recv_strand_ptr->wrap(std::bind(&tcps_auto_connection_impl::_handle_recv, this,
								std::placeholders::_1, // error_code
								std::placeholders::_2, // bytes_recvd
								std::move(this_ptr),
								std::move(buf_ptr)
							)));
					}
					else
					{
						set_last_error((int)errcode::recv_buffer_size_too_small);
						PRINT_EXCEPTION;
						this->stop();
						assert(false);
					}
				}
			}
		}

		virtual void _handle_recv(const asio::error_code & ec, std::size_t bytes_recvd, std::shared_ptr<connection_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> buf_ptr) override
		{
			if (!ec)
			{
				// every times recv data,we update the last active time.
				this->reset_last_active_time();

				auto use_count = buf_ptr.use_count();

				if (m_recv_is_header)
				{
					uint8_t header_flag = (m_header & HEADER_FLAG_MASK);
					if (bytes_recvd == sizeof(m_header) && header_flag == m_url_parser_ptr->get_packet_header_flag())
					{
						m_body_len = (m_header >> HEADER_FLAG_BITS) & MAX_PACKET_SIZE;
						if (m_body_len > m_url_parser_ptr->get_recv_buffer_size() || m_body_len > m_url_parser_ptr->get_max_packet_size())
						{
							set_last_error((int)errcode::recv_buffer_size_too_small);
							PRINT_EXCEPTION;
							this->stop();
							assert(false);
							return;
						}
					}
					else
					{
						set_last_error((int)errcode::recvd_data_invalid);
						PRINT_EXCEPTION;
						this->stop();
						return;
					}
				}
				else
				{
					if (static_cast<std::size_t>(m_body_len) == bytes_recvd)
					{
						buf_ptr->write_bytes(bytes_recvd);

						this->_fire_recv(buf_ptr);
					}
					else
					{
						set_last_error((int)errcode::recvd_data_invalid);
						PRINT_EXCEPTION;
						this->stop();
						return;
					}
				}

				m_recv_is_header = !m_recv_is_header;

				if (m_recv_is_header == false)
				{
					this->_post_recv(std::move(this_ptr), std::move(buf_ptr));
				}
				else
				{
					if (use_count == buf_ptr.use_count())
					{
						buf_ptr->reset();
						this->_post_recv(std::move(this_ptr), std::move(buf_ptr));
					}
					else
					{
						this->_post_recv(std::move(this_ptr), std::make_shared<buffer<uint8_t>>(
							m_url_parser_ptr->get_recv_buffer_size(), malloc_recv_buffer(m_url_parser_ptr->get_recv_buffer_size()), 0));
					}
				}
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

		virtual void _post_send(std::shared_ptr<connection_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> buf_ptr) override
		{
			if (buf_ptr->size() > (std::size_t)m_url_parser_ptr->get_max_packet_size())
			{
				assert(false);
				return;
			}

			uint32_t header = static_cast<uint32_t>(((buf_ptr->size() << HEADER_FLAG_BITS) & ((uint32_t)(~HEADER_FLAG_MASK))) | m_url_parser_ptr->get_packet_header_flag());

			asio::error_code ec;
			std::size_t bytes_sent = asio::write(this->m_socket, asio::buffer((void *)&header, sizeof(header)), ec);

			set_last_error(ec.value());

			if (ec || bytes_sent != sizeof(header))
			{
				PRINT_EXCEPTION;

				this->stop();

				return;
			}

			tcps_connection_impl::_post_send(std::move(this_ptr), std::move(buf_ptr));
		}

	protected:
		volatile uint32_t m_body_len = 0;

		volatile uint32_t m_header = 0;

		volatile bool     m_recv_is_header = true;

	};

}

#endif // !__ASIO2_TCPS_AUTO_CONNECTION_IMPL_HPP__
