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
			m_request_pool_ptr = std::make_shared<object_pool<http_request>>();
		}

		/**
		 * @destruct
		 */
		virtual ~http_server_impl()
		{
		}

		virtual void stop() override
		{
			tcp_server_impl<_acceptor_impl_t>::stop();

			if (m_request_pool_ptr)
				m_request_pool_ptr->destroy();
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

				std::dynamic_pointer_cast<_acceptor_impl_t>(this->m_acceptor_impl_ptr)->set_request_pool(m_request_pool_ptr);

				return this->m_acceptor_impl_ptr->start();
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());
			}

			return false;
		}

	protected:

		std::shared_ptr<object_pool<http_request>> m_request_pool_ptr;

	};

}

#endif // !__ASIO2_HTTP_SERVER_IMPL_HPP__
