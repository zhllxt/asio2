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

		/**
		 * @construct
		 * @param    : io_service_evt - the io_service used to handle the socket event
		 *           : io_service_msg - the io_service used to handle the received msg
		 */
		explicit tcps_auto_session_impl(
			io_service_ptr evt_ioservice_ptr,
			io_service_ptr msg_ioservice_ptr,
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr,
			std::shared_ptr<_pool_t> recv_buf_pool_ptr
		)
			: tcps_session_impl<_pool_t>(
				evt_ioservice_ptr,
				msg_ioservice_ptr,
				listener_mgr_ptr,
				url_parser_ptr,
				recv_buf_pool_ptr
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
	
	protected:
		/**
		 * @function : reset the resource to default status
		 */
		virtual void _reset() override
		{
			tcps_session_impl<_pool_t>::_reset();

			m_body_len = 0;
			m_header = 0;
			m_recv_is_header = true;
			m_sent_is_header = true;
		}

	protected:
		virtual uint32_t _get_max_packet_size()
		{
			// get max_packet_size from the url
			uint32_t max_packet_size = DEFAULT_MAX_PACKET_SIZE;
			std::string str_max_packet_size = this->m_url_parser_ptr->get_param_value("max_packet_size");
			if (!str_max_packet_size.empty())
				max_packet_size = static_cast<uint32_t>(std::atoi(str_max_packet_size.c_str()));
			if (max_packet_size < 8 || max_packet_size > DEFAULT_MAX_PACKET_SIZE)
			{
				assert(false);
				max_packet_size = DEFAULT_MAX_PACKET_SIZE;
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
			if (packet_header_flag == 0 || packet_header_flag > DEFAULT_MAX_HEADER_FLAG)
			{
				assert(false);
				packet_header_flag = DEFAULT_HEADER_FLAG;
			}
			return packet_header_flag;
		}

	protected:

		virtual void _post_recv() override 
		{
			if (this->is_start())
			{
				if (m_recv_is_header)
				{
					m_header = 0;
					boost::asio::async_read(*this->m_socket_ptr,
						boost::asio::buffer((uint8_t*)(&m_header), sizeof(m_header)),
						this->m_evt_strand_ptr->wrap(std::bind(&tcps_auto_session_impl::_handle_recv, std::static_pointer_cast<tcps_auto_session_impl>(this->shared_from_this()),
							std::placeholders::_1, // error_code
							std::placeholders::_2, // bytes_recvd
							nullptr
						)));
				}
				else
				{
					std::size_t buf_len = static_cast<std::size_t>(asio2::get_power_number(m_body_len));

					// every times post recv event,we get the recv buffer from the buffer pool
					std::shared_ptr<uint8_t> recv_buf_ptr = this->m_recv_buf_pool_ptr->get(buf_len);

					boost::asio::async_read(*this->m_socket_ptr,
						boost::asio::buffer(recv_buf_ptr.get(), m_body_len),
						this->m_evt_strand_ptr->wrap(std::bind(&tcps_auto_session_impl::_handle_recv, std::static_pointer_cast<tcps_auto_session_impl>(this->shared_from_this()),
							std::placeholders::_1, // error_code
							std::placeholders::_2, // bytes_recvd
							recv_buf_ptr
						)));
				}
			}
		}

		virtual void _handle_recv(const boost::system::error_code& ec, std::size_t bytes_recvd, std::shared_ptr<uint8_t> recv_buf_ptr) override
		{
			set_last_error(ec.value());

			// every times recv data,we update the last active time.
			this->reset_last_active_time();

			if (!ec)
			{
				if (m_recv_is_header)
				{
					uint8_t header_flag = (m_header & HEADER_FLAG_MASK);
					if (bytes_recvd == sizeof(m_header) && header_flag == m_header_flag)
					{
						m_body_len = (m_header >> PACKET_SIZE_OFFSET_BITS) & DEFAULT_MAX_PACKET_SIZE;
						if (m_body_len > m_max_packet_size)
						{
							set_last_error(DEFAULT_EXCEPTION_CODE, "body length is invalid");
							PRINT_EXCEPTION;
							assert(false);
							this->_fire_close(DEFAULT_EXCEPTION_CODE);
							return;
						}
					}
					else
					{
						set_last_error(DEFAULT_EXCEPTION_CODE, "packet header flag is invalid");
						PRINT_EXCEPTION;
						this->_fire_close(DEFAULT_EXCEPTION_CODE);
						return;
					}
				}
				else
				{
					if (m_body_len == bytes_recvd)
					{
						this->_fire_recv(recv_buf_ptr, bytes_recvd);
					}
					else
					{
						set_last_error(DEFAULT_EXCEPTION_CODE, "packet length is invalid");
						PRINT_EXCEPTION;
						this->_fire_close(DEFAULT_EXCEPTION_CODE);
						return;
					}
				}

				m_recv_is_header = !m_recv_is_header;

				this->_post_recv();
			}
			else
			{
				this->_fire_close(ec.value());
			}

			// If an error occurs then no new asynchronous operations are started. This
			// means that all shared_ptr references to the connection object will
			// disappear and the object will be destroyed automatically after this
			// handler returns. The connection class's destructor closes the socket.
		}

		virtual void _post_send(std::shared_ptr<uint8_t> send_buf_ptr, std::size_t len) override
		{
			if (len > (std::size_t)m_max_packet_size)
			{
				assert(false);
				return;
			}

			if (this->is_start())
			{
				uint32_t header = static_cast<uint32_t>(((len << PACKET_SIZE_OFFSET_BITS) & PACKET_SIZE_MASK) | m_header_flag);
				std::shared_ptr<uint8_t> header_buf_ptr(new uint8_t[sizeof(header)], std::default_delete<uint8_t[]>());
				std::memcpy((void*)header_buf_ptr.get(), (const void*)(&header), sizeof(header));

				boost::system::error_code ec;
				std::size_t bytes_sent = boost::asio::write(*this->m_socket_ptr, boost::asio::buffer(header_buf_ptr.get(), sizeof(header)), ec);

				set_last_error(ec.value());

				if (ec)
				{
					set_last_error(ec.value());

					PRINT_EXCEPTION;

					this->_fire_close(ec.value());

					this->stop();

					return;
				}

				if (bytes_sent != sizeof(header))
				{
					set_last_error(DEFAULT_EXCEPTION_CODE, "sent bytes is not enough for header size");

					PRINT_EXCEPTION;

					this->_fire_close(DEFAULT_EXCEPTION_CODE);

					this->stop();

					return;
				}
			}

			tcps_session_impl<_pool_t>::_post_send(send_buf_ptr, len);
		}

	protected:

		uint32_t          m_max_packet_size = DEFAULT_MAX_PACKET_SIZE;

		uint8_t           m_header_flag     = DEFAULT_HEADER_FLAG;

		volatile uint32_t m_body_len = 0;

		volatile uint32_t m_header   = 0;

		volatile bool     m_recv_is_header = true;

		volatile bool     m_sent_is_header = true;

	};

}

#endif // !__ASIO2_TCPS_AUTO_SESSION_IMPL_HPP__
