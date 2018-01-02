/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 *
 */

#ifndef __ASIO2_HTTP_ACCEPTOR_IMPL_HPP__
#define __ASIO2_HTTP_ACCEPTOR_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/tcp/tcp_acceptor_impl.hpp>

#include <asio2/http/http_session_impl.hpp>

namespace asio2
{

	template<class _session_impl_t>
	class http_acceptor_impl : public tcp_acceptor_impl<_session_impl_t>
	{
	public:
		typedef _session_impl_t session_impl_t;

		/**
		 * @construct
		 */
		http_acceptor_impl(
			std::shared_ptr<url_parser>                    url_parser_ptr,
			std::shared_ptr<listener_mgr>                  listener_mgr_ptr,
			std::shared_ptr<io_context_pool>               io_context_pool_ptr
		)
			: tcp_acceptor_impl<_session_impl_t>(url_parser_ptr, listener_mgr_ptr, io_context_pool_ptr)
		{
		}

		/**
		 * @destruct
		 */
		virtual ~http_acceptor_impl()
		{
		}

		/// must override all listener functions,and cast the m_listener_mgr_ptr to http_server_listener_mgr,
		/// otherwise it will crash when these listener was called.
	protected:
		virtual void _fire_listen() override
		{
			dynamic_cast<http_server_listener_mgr *>(this->m_listener_mgr_ptr.get())->notify_listen();
		}

		virtual void _fire_shutdown(int error) override
		{
			dynamic_cast<http_server_listener_mgr *>(this->m_listener_mgr_ptr.get())->notify_shutdown(error);
		}

	};


}

#endif // !__ASIO2_HTTP_ACCEPTOR_IMPL_HPP__
