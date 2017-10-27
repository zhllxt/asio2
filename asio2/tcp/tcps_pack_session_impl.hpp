/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_TCPS_PACK_SESSION_IMPL_HPP__
#define __ASIO2_TCPS_PACK_SESSION_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/util/helper.hpp>
#include <asio2/tcp/tcps_session_impl.hpp>

namespace asio2
{

	template<class _pool_t>
	class tcps_pack_session_impl : public tcps_session_impl<_pool_t>
	{
		template<class _session_impl_t> friend class tcp_acceptor_impl;

		template<class _Kty, class _Session, class _Hasher, class _Keyeq> friend class session_mgr_t;

	public:

		/**
		 * @construct
		 * @param    : io_service_evt - the io_service used to handle the socket event
		 *           : io_service_msg - the io_service used to handle the received msg
		 */
		explicit tcps_pack_session_impl(
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
		}

		/**
		 * @destruct
		 */
		virtual ~tcps_pack_session_impl()
		{
		}

		virtual void stop() override
		{
			tcps_session_impl<_pool_t>::stop();

			m_pack_parser = nullptr;
		}

		/**
		 * @function : set the data parser under pack model
		 */
		template<typename _parser>
		tcps_pack_session_impl & set_pack_parser(_parser parser)
		{
			m_pack_parser = parser;
			return (*this);
		}

	protected:

		/**
		 * @function : reset the resource to default status
		 */
		virtual void _reset() override
		{
			tcps_session_impl<_pool_t>::_reset();
			
			m_bytes_recvd = 0;

			m_pack_parser = nullptr;
		}

		virtual void _post_recv(std::shared_ptr<uint8_t> recv_buf_ptr, std::size_t bytes_recvd)
		{
			if (this->is_start())
			{
				if (bytes_recvd >= 0 && bytes_recvd < this->m_recv_buf_pool_ptr->get_requested_size())
				{
					this->m_socket_ptr->async_read_some(
						boost::asio::buffer(recv_buf_ptr.get() + bytes_recvd, this->m_recv_buf_pool_ptr->get_requested_size() - bytes_recvd),
						this->m_evt_strand_ptr->wrap(std::bind(&tcps_pack_session_impl::_handle_recv, std::static_pointer_cast<tcps_pack_session_impl>(this->shared_from_this()),
							std::placeholders::_1, // error_code
							std::placeholders::_2, // bytes_recvd
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
					tcps_session_impl<_pool_t>::_post_recv();
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
				this->_fire_close(boost::asio::error::invalid_argument);
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

			// If an error occurs then no new asynchronous operations are started. This
			// means that all shared_ptr references to the connection object will
			// disappear and the object will be destroyed automatically after this
			// handler returns. The connection class's destructor closes the socket.
		}

	protected:

		std::size_t m_bytes_recvd = 0;

		using parser_callback = std::size_t(std::shared_ptr<uint8_t> data_ptr, std::size_t len);

		std::function<parser_callback>       m_pack_parser;

	};

}

#endif // !__ASIO2_TCPS_PACK_SESSION_IMPL_HPP__
