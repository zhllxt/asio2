/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_TCP_PACK_SESSION_IMPL_HPP__
#define __ASIO2_TCP_PACK_SESSION_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/util/helper.hpp>

#include <asio2/tcp/tcp_session_impl.hpp>

namespace asio2
{

	template<class _pool_t>
	class tcp_pack_session_impl : public tcp_session_impl<_pool_t>
	{
		template<class _session_impl_t> friend class tcp_acceptor_impl;

		template<class _Kty, class _Session, class _Hasher, class _Keyeq> friend class session_mgr_t;

	public:

		typedef _pool_t pool_t;

		/**
		 * @construct
		 */
		explicit tcp_pack_session_impl(
			std::shared_ptr<io_service> ioservice_ptr,
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr,
			std::shared_ptr<pool_s> send_buf_pool_ptr,
			std::shared_ptr<pool_t> recv_buf_pool_ptr
		)
			: tcp_session_impl<_pool_t>(
				ioservice_ptr,
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
		virtual ~tcp_pack_session_impl()
		{
		}

		virtual void stop() override
		{
			tcp_session_impl<_pool_t>::stop();

			m_pack_parser = nullptr;
		}

		/**
		 * @function : set the data parser under pack model
		 */
		template<typename _parser>
		tcp_pack_session_impl & set_pack_parser(_parser parser)
		{
			m_pack_parser = parser;
			return (*this);
		}

	protected:
		virtual void _post_recv(std::shared_ptr<session_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> recv_buf_ptr)
		{
			if (this->is_start())
			{
				if (recv_buf_ptr->size() < recv_buf_ptr->capacity())
				{
					this->m_socket_ptr->async_read_some(
						boost::asio::buffer(recv_buf_ptr->data() + recv_buf_ptr->size(), recv_buf_ptr->capacity() - recv_buf_ptr->size()),
						this->m_strand_ptr->wrap(std::bind(&tcp_pack_session_impl::_handle_recv, std::static_pointer_cast<tcp_pack_session_impl>(this_ptr),
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
					this->_fire_close(this_ptr, (int)errcode::packet_length_too_large);
				}
			}
		}

		virtual void _recurse_parse_data(std::shared_ptr<session_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> recv_buf_ptr)
		{
			std::size_t ret = m_pack_parser(recv_buf_ptr);

			if /**/ (ret == recv_buf_ptr->size())
			{
				this->_fire_recv(this_ptr, recv_buf_ptr);

				tcp_session_impl<_pool_t>::_post_recv(this_ptr);
			}
			else if (ret == asio2::need_more_data)
			{
				if (recv_buf_ptr->size() >= recv_buf_ptr->capacity())
				{
					set_last_error((int)errcode::packet_length_too_large);
					PRINT_EXCEPTION;
					assert(false);
					this->_fire_close(this_ptr, (int)errcode::packet_length_too_large);
				}
				else
				{
					if (recv_buf_ptr->offset() > 0)
					{
						std::shared_ptr<buffer<uint8_t>> recv_ptr = this->m_recv_buf_pool_ptr->get(0);
						std::memcpy((void *)recv_ptr->data(), (const void *)recv_buf_ptr->data(), recv_buf_ptr->size());
						recv_ptr->resize(recv_buf_ptr->size());

						this->_post_recv(this_ptr, recv_ptr);
					}
					else
					{
						this->_post_recv(this_ptr, recv_buf_ptr);
					}
				}
			}
			else if (ret > 0 && ret < recv_buf_ptr->size())
			{
				std::shared_ptr<buffer<uint8_t>> recv_ptr = this->m_recv_buf_pool_ptr->get(0);
				std::memcpy((void *)recv_ptr->data(), (const void *)recv_buf_ptr->data(), ret);
				recv_ptr->resize(ret);

				this->_fire_recv(this_ptr, recv_ptr);

				recv_buf_ptr->reoffset(ret);

				_recurse_parse_data(this_ptr, recv_buf_ptr);
			}
			else if (ret == asio2::invalid_data || ret <= 0 || ret > recv_buf_ptr->size())
			{
				set_last_error((int)errcode::recvd_data_invalid);
				PRINT_EXCEPTION;
				assert(false);
				this->_fire_close(this_ptr, (int)errcode::recvd_data_invalid);
			}
		}

		virtual void _handle_recv(const boost::system::error_code& ec, std::size_t bytes_recvd, std::shared_ptr<session_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> recv_buf_ptr) override
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

				this->_fire_close(this_ptr, ec.value());
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

#endif // !__ASIO2_TCP_PACK_SESSION_IMPL_HPP__
