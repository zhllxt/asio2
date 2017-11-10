/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_TCPS_PACK_CONNECTION_IMPL_HPP__
#define __ASIO2_TCPS_PACK_CONNECTION_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/tcp/tcps_connection_impl.hpp>

namespace asio2
{

	template<class _pool_t>
	class tcps_pack_connection_impl : public tcps_connection_impl<_pool_t>
	{
	public:

		typedef _pool_t pool_t;

		/**
		 * @construct
		 */
		explicit tcps_pack_connection_impl(
			std::shared_ptr<io_service> send_ioservice_ptr,
			std::shared_ptr<io_service> recv_ioservice_ptr,
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr,
			std::shared_ptr<pool_s> send_buf_pool_ptr,
			std::shared_ptr<pool_t> recv_buf_pool_ptr
		)
			: tcps_connection_impl<_pool_t>(
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
		virtual ~tcps_pack_connection_impl()
		{
		}

		virtual bool start(bool async_connect = true) override
		{
			if (!m_pack_parser)
				throw std::runtime_error("must call set_pack_parser to specifies the data parser before start client under pack model");

			return tcps_connection_impl<_pool_t>::start(async_connect);
		}

		virtual void stop() override
		{
			tcps_connection_impl<_pool_t>::stop();

			m_pack_parser = nullptr;
		}

	public:

		/**
		 * @function : set the data parser under pack model
		 */
		template<typename _parser>
		tcps_pack_connection_impl & set_pack_parser(_parser parser)
		{
			m_pack_parser = parser;
			return (*this);
		}

	protected:

		virtual void _post_recv(std::shared_ptr<connection_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> recv_buf_ptr)
		{
			if (this->is_start())
			{
				if (recv_buf_ptr->size() < recv_buf_ptr->capacity())
				{
					this->m_socket_ptr->async_read_some(
						boost::asio::buffer(recv_buf_ptr->data() + recv_buf_ptr->size(), recv_buf_ptr->capacity() - recv_buf_ptr->size()),
						this->m_recv_strand_ptr->wrap(std::bind(&tcps_pack_connection_impl::_handle_recv, std::static_pointer_cast<tcps_pack_connection_impl>(this_ptr),
							std::placeholders::_1, // error_code
							std::placeholders::_2, // bytes_recvd
							this_ptr,
							recv_buf_ptr
						)));
				}
				else
				{
					set_last_error((int)errcode::packet_length_too_large);
					PRINT_EXCEPTION;
					assert(false);
					this->_fire_close((int)errcode::packet_length_too_large);
				}
			}
		}

		virtual void _recurse_parse_data(std::shared_ptr<connection_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> recv_buf_ptr)
		{
			std::size_t ret = m_pack_parser(recv_buf_ptr);

			if /**/ (ret == recv_buf_ptr->size())
			{
				this->_fire_recv(recv_buf_ptr);

				tcps_connection_impl<_pool_t>::_post_recv(this_ptr);
			}
			else if (ret == asio2::need_more_data)
			{
				if (recv_buf_ptr->size() >= recv_buf_ptr->capacity())
				{
					set_last_error((int)errcode::packet_length_too_large);
					PRINT_EXCEPTION;
					assert(false);
					this->_fire_close((int)errcode::packet_length_too_large);
				}
				else
				{
					if (recv_buf_ptr->offset() > 0)
					{
						std::shared_ptr<buffer<uint8_t>> recv_ptr = this->m_recv_buf_pool_ptr->get(0);
						std::memcpy((void *)recv_ptr->data(), (const void *)recv_buf_ptr->data(), recv_buf_ptr->size());
						recv_ptr->resize(recv_buf_ptr->size());

						_post_recv(this_ptr, recv_ptr);
					}
					else
					{
						_post_recv(this_ptr, recv_buf_ptr);
					}
				}
			}
			else if (ret > 0 && ret < recv_buf_ptr->size())
			{
				std::shared_ptr<buffer<uint8_t>> recv_ptr = this->m_recv_buf_pool_ptr->get(0);
				std::memcpy((void *)recv_ptr->data(), (const void *)recv_buf_ptr->data(), ret);
				recv_ptr->resize(ret);

				this->_fire_recv(recv_ptr);

				recv_buf_ptr->reoffset(ret);

				_recurse_parse_data(this_ptr, recv_buf_ptr);
			}
			else if (ret == asio2::invalid_data || ret <= 0 || ret > recv_buf_ptr->size())
			{
				set_last_error((int)errcode::recvd_data_invalid);
				PRINT_EXCEPTION;
				assert(false);
				this->_fire_close((int)errcode::recvd_data_invalid);
			}
		}

		virtual void _handle_recv(const boost::system::error_code& ec, std::size_t bytes_recvd, std::shared_ptr<connection_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> recv_buf_ptr) override
		{
			if (!ec)
			{
				// every times recv data,we update the last active time.
				this->reset_last_active_time();

				recv_buf_ptr->resize(recv_buf_ptr->size() + bytes_recvd);

				_recurse_parse_data(this_ptr, recv_buf_ptr);
			}
			else
			{
				set_last_error(ec.value());

				this->_fire_close(ec.value());
			}

			// No new asynchronous operations are started. This means that all shared_ptr
			// references to the connection object will disappear and the object will be
			// destroyed automatically after this handler returns. The connection class's
			// destructor closes the socket.
		}

	protected:

		using parser_callback = std::size_t(std::shared_ptr<buffer<uint8_t>> data_ptr);

		std::function<parser_callback>       m_pack_parser;

	};

}

#endif // !__ASIO2_TCPS_PACK_CONNECTION_IMPL_HPP__
