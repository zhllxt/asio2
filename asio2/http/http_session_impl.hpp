/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_HTTP_SESSION_IMPL_HPP__
#define __ASIO2_HTTP_SESSION_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/http/http_parser.h>

#include <asio2/tcp/tcp_session_impl.hpp>

namespace asio2
{

	template<class _pool_t>
	class http_session_impl : public tcp_session_impl<_pool_t>
	{
		template<class _session_impl_t> friend class tcp_acceptor_impl;

		template<class _acceptor_impl_t> friend class http_server_impl;

		template<class _Kty, class _Session, class _Hasher, class _Keyeq> friend class session_mgr_t;

	public:

		typedef _pool_t pool_t;

		/**
		 * @construct
		 */
		explicit http_session_impl(
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
			m_parser.data = this;
		}

		/**
		 * @destruct
		 */
		virtual ~http_session_impl()
		{
		}

		virtual bool start() override
		{
			http::http_parser_init(&m_parser, http::HTTP_REQUEST);

			return tcp_session_impl<_pool_t>::start();
		}

		inline http_session_impl & set_parser_settings(std::shared_ptr<http::http_parser_settings> settings_ptr)
		{
			m_settings_ptr = settings_ptr;
			return (*this);
		}

	protected:
		virtual void _handle_recv(const boost::system::error_code& ec, std::size_t bytes_recvd, std::shared_ptr<session_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> recv_buf_ptr) override
		{
			if (!ec)
			{
				// every times recv data,we update the last active time.
				reset_last_active_time();

				recv_buf_ptr->resize(bytes_recvd);

				http::http_parser_execute(&m_parser, m_settings_ptr.get(), (const char *)recv_buf_ptr->data(), recv_buf_ptr->size());

				if (m_parser.http_errno != http::HPE_OK)
				{
					set_last_error((int)(m_parser.http_errno | HTTP_ERROR_CODE_MASK));

					_fire_close(this_ptr, (int)(m_parser.http_errno | HTTP_ERROR_CODE_MASK));

					return;
				}

				_post_recv(this_ptr);
			}
			else
			{
				set_last_error(ec.value());

				_fire_close(this_ptr, ec.value());
			}

			// If an error occurs then no new asynchronous operations are started. This
			// means that all shared_ptr references to the connection object will
			// disappear and the object will be destroyed automatically after this
			// handler returns. The connection class's destructor closes the socket.
		}

	protected:
		virtual int on_message_begin(asio2::http::http_parser * parser)
		{
			return 0;
		}
		virtual int on_url(asio2::http::http_parser * parser, const char *at, size_t length)
		{
			return 0;
		}
		virtual int on_status(asio2::http::http_parser * parser, const char *at, size_t length)
		{
			return 0;
		}
		virtual int on_header_field(asio2::http::http_parser * parser, const char *at, size_t length)
		{
			return 0;
		}
		virtual int on_header_value(asio2::http::http_parser * parser, const char *at, size_t length)
		{
			return 0;
		}
		virtual int on_headers_complete(asio2::http::http_parser * parser)
		{
			return 0;
		}
		virtual int on_body(asio2::http::http_parser * parser, const char *at, size_t length)
		{
			return 0;
		}
		virtual int on_message_complete(asio2::http::http_parser * parser)
		{
			return 0;
		}
		virtual int on_chunk_header(asio2::http::http_parser * parser)
		{
			return 0;
		}
		virtual int on_chunk_complete(asio2::http::http_parser * parser)
		{
			return 0;
		}

	protected:
		http::http_parser            m_parser;

		std::shared_ptr<http::http_parser_settings> m_settings_ptr;
	};

}

#endif // !__ASIO2_HTTP_SESSION_IMPL_HPP__
