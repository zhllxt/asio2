/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_HTTP_CLIENT_IMPL_HPP__
#define __ASIO2_HTTP_CLIENT_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/tcp/tcp_client_impl.hpp>

#include <asio2/http/http_connection_impl.hpp>

namespace asio2
{

	template<class _connection_impl_t>
	class http_client_impl : public tcp_client_impl<_connection_impl_t>
	{
	public:

		typedef typename _connection_impl_t::pool_t pool_t;

		/**
		 * @construct
		 */
		explicit http_client_impl(
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr
		)
			: tcp_client_impl<_connection_impl_t>(listener_mgr_ptr, url_parser_ptr)
		{
			m_request_pool_ptr  = std::make_shared<object_pool<http_request>>();
			m_response_pool_ptr = std::make_shared<object_pool<http_response>>();
		}

		/**
		 * @destruct
		 */
		virtual ~http_client_impl()
		{
		}

		virtual void stop() override
		{
			tcp_client_impl<_connection_impl_t>::stop();

			if (m_request_pool_ptr)
				m_request_pool_ptr->destroy();
			if (m_response_pool_ptr)
				m_response_pool_ptr->destroy();
		}

	protected:

		virtual void _prepare_connection() override
		{
			tcp_client_impl<_connection_impl_t>::_prepare_connection();

			if (this->m_connection_impl_ptr)
			{
				std::dynamic_pointer_cast<_connection_impl_t>(this->m_connection_impl_ptr)->set_request_pool(this->m_request_pool_ptr);
				std::dynamic_pointer_cast<_connection_impl_t>(this->m_connection_impl_ptr)->set_response_pool(this->m_response_pool_ptr);
			}
		}

	protected:

		std::shared_ptr<object_pool<http_request>>  m_request_pool_ptr;

		std::shared_ptr<object_pool<http_response>> m_response_pool_ptr;

	};

}

#endif // !__ASIO2_HTTP_CLIENT_IMPL_HPP__
