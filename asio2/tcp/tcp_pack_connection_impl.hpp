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

	template<class _pool_t>
	class tcp_pack_connection_impl : public tcp_connection_impl<_pool_t>
	{
	public:

		/**
		 * @construct
		 */
		explicit tcp_pack_connection_impl(
			io_service_ptr evt_send_ioservice_ptr,
			io_service_ptr evt_recv_ioservice_ptr,
			io_service_ptr msg_send_ioservice_ptr,
			io_service_ptr msg_recv_ioservice_ptr,
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr,
			std::shared_ptr<_pool_t> recv_buf_pool_ptr
		)
			: tcp_connection_impl<_pool_t>(
				evt_send_ioservice_ptr,
				evt_recv_ioservice_ptr,
				msg_send_ioservice_ptr,
				msg_recv_ioservice_ptr,
				listener_mgr_ptr,
				url_parser_ptr,
				recv_buf_pool_ptr
				)
		{
		}

		/**
		 * @destruct
		 */
		virtual ~tcp_pack_connection_impl()
		{
		}

		virtual bool start(bool async_connect = true) override
		{
			if (!m_pack_parser)
				throw std::runtime_error("must call set_pack_parser to specifies the data parser before start client under pack model");

			return tcp_connection_impl<_pool_t>::start(async_connect);
		}

		virtual void stop() override
		{
			tcp_connection_impl<_pool_t>::stop();

			m_pack_parser = nullptr;
		}

	public:

		/**
		 * @function : set the data parser under pack model
		 */
		template<typename _parser>
		tcp_pack_connection_impl & set_pack_parser(_parser parser)
		{
			m_pack_parser = parser;
			return (*this);
		}

	protected:

		virtual void _post_recv(std::shared_ptr<uint8_t> recv_buf_ptr, std::size_t bytes_recvd)
		{
			if (this->is_start())
			{
				if (bytes_recvd >= 0 && bytes_recvd < this->m_recv_buf_pool_ptr->get_requested_size())
				{
					this->m_socket_ptr->async_read_some(
						boost::asio::buffer(recv_buf_ptr.get() + bytes_recvd, this->m_recv_buf_pool_ptr->get_requested_size() - bytes_recvd),
						this->m_evt_recv_strand_ptr->wrap(std::bind(&tcp_pack_connection_impl::_handle_recv, std::static_pointer_cast<tcp_pack_connection_impl>(this->shared_from_this()),
							std::placeholders::_1,
							std::placeholders::_2,
							recv_buf_ptr
						)));
				}
				else
				{
					set_last_error(DEFAULT_EXCEPTION_CODE, "packet length is greater than pool_buffer_size,may be you need to set a larger pool_buffer_size");
					PRINT_EXCEPTION;
					assert(false);
					this->_fire_close(DEFAULT_EXCEPTION_CODE);
				}
			}
		}

		virtual void recurse_parse_data(std::shared_ptr<uint8_t> recv_buf_ptr, bool is_buf_header)
		{
			std::size_t ret = m_pack_parser(recv_buf_ptr, m_bytes_recvd);

			if (ret == NEED_MORE_DATA)
			{
				if (m_bytes_recvd >= this->m_recv_buf_pool_ptr->get_requested_size())
				{
					set_last_error(DEFAULT_EXCEPTION_CODE, "packet length is greater than pool_buffer_size,may be you need to set a larger pool_buffer_size");
					PRINT_EXCEPTION;
					assert(false);
					this->_fire_close(DEFAULT_EXCEPTION_CODE);
				}
				else
				{
					if (is_buf_header)
						_post_recv(recv_buf_ptr, m_bytes_recvd);
					else
					{
						std::shared_ptr<uint8_t> new_recv_buf_ptr = this->m_recv_buf_pool_ptr->get(0);
						std::memcpy((void*)new_recv_buf_ptr.get(), (const void *)recv_buf_ptr.get(), m_bytes_recvd);
						_post_recv(new_recv_buf_ptr, m_bytes_recvd);
					}
				}
			}
			else if (ret == INVALID_DATA)
			{
				this->_fire_close(DEFAULT_EXCEPTION_CODE);
			}
			else if (ret > 0 && ret <= m_bytes_recvd)
			{
				this->_fire_recv(recv_buf_ptr, ret);

				m_bytes_recvd -= ret;

				if (m_bytes_recvd == 0)
					tcp_connection_impl<_pool_t>::_post_recv();
				else if (m_bytes_recvd > 0)
				{
					std::shared_ptr<uint8_t> data_ptr(recv_buf_ptr.get() + ret, [](uint8_t *) {});
					recurse_parse_data(data_ptr, false);
				}
			}
			else if (ret <= 0 || ret > m_bytes_recvd)
			{
				set_last_error(DEFAULT_EXCEPTION_CODE, "pack parser return value indicate that the recvd data is invalid");
				PRINT_EXCEPTION;
				assert(false);
				this->_fire_close(DEFAULT_EXCEPTION_CODE);
			}
		}

		virtual void _handle_recv(const boost::system::error_code& ec, std::size_t bytes_recvd, std::shared_ptr<uint8_t> recv_buf_ptr) override
		{
			set_last_error(ec.value());

			// every times recv data,we update the last active time.
			this->reset_last_active_time();

			if (!ec)
			{
				m_bytes_recvd += bytes_recvd;

				recurse_parse_data(recv_buf_ptr, true);
			}
			else
			{
				this->_fire_close(ec.value());
			}

			// No new asynchronous operations are started. This means that all shared_ptr
			// references to the connection object will disappear and the object will be
			// destroyed automatically after this handler returns. The connection class's
			// destructor closes the socket.
		}

	protected:

		std::size_t m_bytes_recvd = 0;

		using parser_callback = std::size_t(std::shared_ptr<uint8_t> data_ptr, std::size_t len);

		std::function<parser_callback>       m_pack_parser;

	};

}

#endif // !__ASIO2_TCP_PACK_CONNECTION_IMPL_HPP__
