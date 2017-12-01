/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_TCPS_AUTO_SESSION_IMPL_HPP__
#define __ASIO2_TCPS_AUTO_SESSION_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/util/helper.hpp>
#include <asio2/tcp/tcps_session_impl.hpp>

namespace asio2
{

	template<class _pool_t>
	class tcps_auto_session_impl : public tcps_session_impl<_pool_t>
	{
		template<class _session_impl_t> friend class tcp_acceptor_impl;

		template<class _Kty, class _Session, class _Hasher, class _Keyeq> friend class session_mgr_t;

	public:

		typedef _pool_t pool_t;

		/**
		 * @construct
		 */
		explicit tcps_auto_session_impl(
			std::shared_ptr<io_service> ioservice_ptr,
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr,
			std::shared_ptr<pool_s> send_buf_pool_ptr,
			std::shared_ptr<pool_t> recv_buf_pool_ptr,
			std::shared_ptr<boost::asio::ssl::context> context_ptr = nullptr
		)
			: tcps_session_impl<_pool_t>(
				ioservice_ptr,
				listener_mgr_ptr,
				url_parser_ptr,
				send_buf_pool_ptr,
				recv_buf_pool_ptr,
				context_ptr
				)
		{
			m_max_packet_size = this->_get_max_packet_size();
			m_header_flag = this->_get_packet_header_flag();
		}

		/**
		 * @destruct
		 */
		virtual ~tcps_auto_session_impl()
		{
		}

		virtual bool start() override
		{
			// reset the variable to default status
			m_body_len = 0;
			m_header = 0;
			m_recv_is_header = true;
			m_sent_is_header = true;

			return tcps_session_impl<_pool_t>::start();
		}

	protected:
		virtual uint32_t _get_max_packet_size()
		{
			// get max_packet_size from the url
			uint32_t max_packet_size = MAX_PACKET_SIZE;
			std::string str_max_packet_size = this->m_url_parser_ptr->get_param_value("max_packet_size");
			if (!str_max_packet_size.empty())
				max_packet_size = static_cast<uint32_t>(std::atoi(str_max_packet_size.c_str()));
			if (max_packet_size < 8 || max_packet_size > MAX_PACKET_SIZE)
			{
				assert(false);
				max_packet_size = MAX_PACKET_SIZE;
			}
			return max_packet_size;
		}

		virtual uint8_t _get_packet_header_flag()
		{
			// get packet_header_flag from the url
			uint8_t packet_header_flag = DEFAULT_HEADER_FLAG;
			std::string str_packet_header_flag = this->m_url_parser_ptr->get_param_value("packet_header_flag");
			if (!str_packet_header_flag.empty())
				packet_header_flag = static_cast<uint8_t>(std::atoi(str_packet_header_flag.c_str()));
			if (packet_header_flag == 0 || packet_header_flag > MAX_HEADER_FLAG)
			{
				assert(false);
				packet_header_flag = DEFAULT_HEADER_FLAG;
			}
			return packet_header_flag;
		}

	protected:

		virtual void _post_recv(std::shared_ptr<session_impl> this_ptr) override
		{
			if (this->is_start())
			{
				if (m_recv_is_header)
				{
					m_header = 0;
					boost::asio::async_read(*this->m_socket_ptr,
						boost::asio::buffer((uint8_t*)(&m_header), sizeof(m_header)),
						this->m_strand_ptr->wrap(std::bind(&tcps_auto_session_impl::_handle_recv, std::static_pointer_cast<tcps_auto_session_impl>(this_ptr),
							std::placeholders::_1, // error_code
							std::placeholders::_2, // bytes_recvd
							this_ptr,
							nullptr
						)));
				}
				else
				{
					std::size_t buf_len = static_cast<std::size_t>(get_power_number(m_body_len));

					// every times post recv event,we get the recv buffer from the buffer pool
					std::shared_ptr<buffer<uint8_t>> recv_buf_ptr = this->m_recv_buf_pool_ptr->get(buf_len);

					boost::asio::async_read(*this->m_socket_ptr,
						boost::asio::buffer(recv_buf_ptr->data(), m_body_len),
						this->m_strand_ptr->wrap(std::bind(&tcps_auto_session_impl::_handle_recv, std::static_pointer_cast<tcps_auto_session_impl>(this_ptr),
							std::placeholders::_1, // error_code
							std::placeholders::_2, // bytes_recvd
							this_ptr,
							recv_buf_ptr
						)));
				}
			}
		}

		virtual void _handle_recv(const boost::system::error_code& ec, std::size_t bytes_recvd, std::shared_ptr<session_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> recv_buf_ptr) override
		{
			if (!ec)
			{
				// every times recv data,we update the last active time.
				this->reset_last_active_time();

				if (m_recv_is_header)
				{
					uint8_t header_flag = (m_header & HEADER_FLAG_MASK);
					if (bytes_recvd == sizeof(m_header) && header_flag == m_header_flag)
					{
						m_body_len = (m_header >> HEADER_FLAG_BITS) & MAX_PACKET_SIZE;
						if (m_body_len > m_max_packet_size)
						{
							set_last_error((int)errcode::packet_length_too_large);
							PRINT_EXCEPTION;
							assert(false);
							this->_fire_close(this_ptr, (int)errcode::packet_length_too_large);
							return;
						}
					}
					else
					{
						set_last_error((int)errcode::recvd_data_invalid);
						PRINT_EXCEPTION;
						this->_fire_close(this_ptr, (int)errcode::recvd_data_invalid);
						return;
					}
				}
				else
				{
					if (m_body_len == bytes_recvd)
					{
						recv_buf_ptr->resize(bytes_recvd);

						this->_fire_recv(this_ptr, recv_buf_ptr);
					}
					else
					{
						set_last_error((int)errcode::recvd_data_invalid);
						PRINT_EXCEPTION;
						this->_fire_close(this_ptr, (int)errcode::recvd_data_invalid);
						return;
					}
				}

				m_recv_is_header = !m_recv_is_header;

				this->_post_recv(this_ptr);
			}
			else
			{
				set_last_error(ec.value());

				this->_fire_close(this_ptr, ec.value());
			}

			// If an error occurs then no new asynchronous operations are started. This
			// means that all shared_ptr references to the connection object will
			// disappear and the object will be destroyed automatically after this
			// handler returns. The connection class's destructor closes the socket.
		}

		virtual void _post_send(std::shared_ptr<session_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> send_buf_ptr) override
		{
			if (send_buf_ptr->size() > (std::size_t)m_max_packet_size)
			{
				assert(false);
				return;
			}

			if (this->is_start())
			{
				uint32_t header = static_cast<uint32_t>(((send_buf_ptr->size() << HEADER_FLAG_BITS) & ((uint32_t)(~HEADER_FLAG_MASK))) | m_header_flag);

				boost::system::error_code ec;
				std::size_t bytes_sent = boost::asio::write(*this->m_socket_ptr, boost::asio::buffer((void *)&header, sizeof(header)), ec);

				set_last_error(ec.value());

				if (ec)
				{
					set_last_error(ec.value());

					PRINT_EXCEPTION;

					this->_fire_close(this_ptr, ec.value());

					this->stop();

					return;
				}

				if (bytes_sent != sizeof(header))
				{
					set_last_error((int)errcode::send_data_failed);

					PRINT_EXCEPTION;

					this->_fire_close(this_ptr, (int)errcode::send_data_failed);

					this->stop();

					return;
				}
			}

			tcps_session_impl<_pool_t>::_post_send(this_ptr, send_buf_ptr);
		}

	protected:

		uint32_t          m_max_packet_size = MAX_PACKET_SIZE;

		uint8_t           m_header_flag     = DEFAULT_HEADER_FLAG;

		volatile uint32_t m_body_len = 0;

		volatile uint32_t m_header   = 0;

		volatile bool     m_recv_is_header = true;

		volatile bool     m_sent_is_header = true;

	};

}

#endif // !__ASIO2_TCPS_AUTO_SESSION_IMPL_HPP__
