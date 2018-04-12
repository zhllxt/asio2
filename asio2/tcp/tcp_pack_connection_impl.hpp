/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_TCP_PACK_CONNECTION_IMPL_HPP__
#define __ASIO2_TCP_PACK_CONNECTION_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/tcp/tcp_connection_impl.hpp>

namespace asio2
{

	class tcp_pack_connection_impl : public tcp_connection_impl
	{
		template<class _connection_impl_t> friend class tcp_pack_client_impl;

	public:
		using parser_callback = std::size_t(std::shared_ptr<buffer<uint8_t>> & buf_ptr);

		/**
		 * @construct
		 */
		explicit tcp_pack_connection_impl(
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
		virtual ~tcp_pack_connection_impl()
		{
		}

	protected:
		virtual void _handle_recv(const asio::error_code & ec, std::size_t bytes_recvd, std::shared_ptr<connection_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> buf_ptr) override
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
					buf_ptr->write_bytes(bytes_recvd);
				}

				_recurse_parse_data(this_ptr, buf_ptr);
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

		virtual void _recurse_parse_data(std::shared_ptr<connection_impl> & this_ptr, std::shared_ptr<buffer<uint8_t>> & buf_ptr)
		{
			std::size_t ret = m_pack_parser(buf_ptr);

			if /**/ (ret == buf_ptr->size())
			{
				auto use_count = buf_ptr.use_count();

				this->_fire_recv(buf_ptr);

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
			else if (ret == asio2::need_more_data)
			{
				if (buf_ptr->read_pos() > 0)
				{
					this->_post_recv(std::move(this_ptr), std::make_shared<buffer<uint8_t>>(m_url_parser_ptr->get_recv_buffer_size(),
						malloc_recv_buffer(m_url_parser_ptr->get_recv_buffer_size()), buf_ptr->read_begin(), buf_ptr->size()));
				}
				else
				{
					if (buf_ptr->remain() == 0)
					{
						set_last_error((int)errcode::recv_buffer_size_too_small);
						ASIO2_DUMP_EXCEPTION_LOG_IMPL;
						this->stop();
						assert(false);
					}
					else
					{
						this->_post_recv(std::move(this_ptr), std::move(buf_ptr));
					}
				}
			}
			else if (ret > 0 && ret < buf_ptr->size())
			{
				auto recv_ptr = std::make_shared<buffer<uint8_t>>(ret, malloc_recv_buffer(ret), buf_ptr->read_begin(), ret);
				this->_fire_recv(recv_ptr);

				buf_ptr->read_bytes(ret);

				_recurse_parse_data(this_ptr, buf_ptr);
			}
			else// if (ret == asio2::invalid_data || ret <= 0 || ret > buf_ptr->size())
			{
				set_last_error((int)errcode::recvd_data_invalid);
				ASIO2_DUMP_EXCEPTION_LOG_IMPL;
				this->stop();
				assert(false);
			}
		}

	protected:
		std::function<parser_callback>       m_pack_parser;

	};

}

#endif // !__ASIO2_TCP_PACK_CONNECTION_IMPL_HPP__
