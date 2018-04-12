/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_TCPS_CLIENT_IMPL_HPP__
#define __ASIO2_TCPS_CLIENT_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/tcp/tcps_connection_impl.hpp>
#include <asio2/tcp/tcp_client_impl.hpp>

namespace asio2
{

	template<class _connection_impl_t>
	class tcps_client_impl : public tcp_client_impl<_connection_impl_t>
	{
	public:
		/**
		 * @construct
		 */
		explicit tcps_client_impl(
			std::shared_ptr<url_parser>        url_parser_ptr,
			std::shared_ptr<listener_mgr>      listener_mgr_ptr,
			asio::ssl::context::method  method,
			asio::ssl::context::options options
		)
			: tcp_client_impl<_connection_impl_t>(url_parser_ptr, nullptr)
		{
			this->m_listener_mgr_ptr = listener_mgr_ptr;
			try
			{
				this->m_ssl_context_ptr = std::make_shared<asio::ssl::context>(method);
				this->m_ssl_context_ptr->set_options(options);

				this->m_connection_impl_ptr = std::make_shared<_connection_impl_t>(url_parser_ptr, listener_mgr_ptr,
					nullptr, this->m_io_context_pool_ptr->get_io_context_ptr(), this->m_ssl_context_ptr);
			}
			catch (asio::system_error & e)
			{
				set_last_error(e.code().value());
				ASIO2_DUMP_EXCEPTION_LOG_IMPL;
				m_ssl_context_ptr.reset();
			}
		}

		/**
		 * @destruct
		 */
		virtual ~tcps_client_impl()
		{
		}

		virtual bool start(bool async_connect = true) override
		{
			return ((this->m_ssl_context_ptr && this->m_connection_impl_ptr) ? tcp_client_impl<_connection_impl_t>::start(async_connect) : false);
		}

		virtual void stop() override
		{
			tcp_client_impl<_connection_impl_t>::stop();
		}

		inline std::shared_ptr<asio::ssl::context> get_ssl_context() { return this->m_ssl_context_ptr; }

	protected:
		/// ssl context 
		std::shared_ptr<asio::ssl::context> m_ssl_context_ptr;

	};

}

#endif // !__ASIO2_TCPS_CLIENT_IMPL_HPP__
