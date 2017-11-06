/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_HTTP_SERVER_IMPL_HPP__
#define __ASIO2_HTTP_SERVER_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/tcp/tcp_server_impl.hpp>

#include <asio2/http/http_acceptor_impl.hpp>

namespace asio2
{

	template<class _acceptor_impl_t>
	class http_server_impl : public tcp_server_impl<_acceptor_impl_t>
	{
	public:

		/**
		 * @construct
		 */
		http_server_impl(
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr
		)
			: tcp_server_impl<_acceptor_impl_t>(listener_mgr_ptr, url_parser_ptr)
		{
			m_settings_ptr = std::make_shared<http::http_parser_settings>();

			/*http_cb      */m_settings_ptr->on_message_begin		= on_message_begin;
			/*http_data_cb */m_settings_ptr->on_url					= on_url;
			/*http_data_cb */m_settings_ptr->on_status				= on_status;
			/*http_data_cb */m_settings_ptr->on_header_field		= on_header_field;
			/*http_data_cb */m_settings_ptr->on_header_value		= on_header_value;
			/*http_cb      */m_settings_ptr->on_headers_complete	= on_headers_complete;
			/*http_data_cb */m_settings_ptr->on_body				= on_body;
			/*http_cb      */m_settings_ptr->on_message_complete	= on_message_complete;
			/*http_cb      */m_settings_ptr->on_chunk_header		= on_chunk_header;
			/*http_cb      */m_settings_ptr->on_chunk_complete		= on_chunk_complete;
		}

		/**
		 * @destruct
		 */
		virtual ~http_server_impl()
		{
		}

	protected:

		virtual bool _start_listen() override
		{
			try
			{
				this->m_acceptor_impl_ptr = std::make_shared<_acceptor_impl_t>(
					this->m_ioservice_pool_ptr,
					this->m_listener_mgr_ptr,
					this->m_url_parser_ptr,
					this->m_send_buf_pool_ptr,
					this->m_recv_buf_pool_ptr
					);

				std::dynamic_pointer_cast<_acceptor_impl_t>(this->m_acceptor_impl_ptr)->set_parser_settings(m_settings_ptr);

				return this->m_acceptor_impl_ptr->start();
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());
			}

			return false;
		}

	protected:
		static int on_message_begin(asio2::http::http_parser * parser)
		{
			return static_cast<_acceptor_impl_t::session_impl_t *>(parser->data)->on_message_begin(parser);
		}
		static int on_url(asio2::http::http_parser * parser, const char *at, size_t length)
		{
			return static_cast<_acceptor_impl_t::session_impl_t *>(parser->data)->on_url(parser, at, length);
		}
		static int on_status(asio2::http::http_parser * parser, const char *at, size_t length)
		{
			return static_cast<_acceptor_impl_t::session_impl_t *>(parser->data)->on_status(parser, at, length);
		}
		static int on_header_field(asio2::http::http_parser * parser, const char *at, size_t length)
		{
			return static_cast<_acceptor_impl_t::session_impl_t *>(parser->data)->on_header_field(parser, at, length);
		}
		static int on_header_value(asio2::http::http_parser * parser, const char *at, size_t length)
		{
			return static_cast<_acceptor_impl_t::session_impl_t *>(parser->data)->on_header_value(parser, at, length);
		}
		static int on_headers_complete(asio2::http::http_parser * parser)
		{
			return static_cast<_acceptor_impl_t::session_impl_t *>(parser->data)->on_headers_complete(parser);
		}
		static int on_body(asio2::http::http_parser * parser, const char *at, size_t length)
		{
			return static_cast<_acceptor_impl_t::session_impl_t *>(parser->data)->on_body(parser, at, length);
		}
		static int on_message_complete(asio2::http::http_parser * parser)
		{
			return static_cast<_acceptor_impl_t::session_impl_t *>(parser->data)->on_message_complete(parser);
		}
		static int on_chunk_header(asio2::http::http_parser * parser)
		{
			return static_cast<_acceptor_impl_t::session_impl_t *>(parser->data)->on_chunk_header(parser);
		}
		static int on_chunk_complete(asio2::http::http_parser * parser)
		{
			return static_cast<_acceptor_impl_t::session_impl_t *>(parser->data)->on_chunk_complete(parser);
		}

	protected:
		std::shared_ptr<http::http_parser_settings> m_settings_ptr;

	};

}

#endif // !__ASIO2_HTTP_SERVER_IMPL_HPP__
