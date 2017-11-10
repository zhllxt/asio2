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
		typedef typename _session_impl_t::pool_t pool_t;

		/**
		 * @construct
		 */
		http_acceptor_impl(
			io_service_pool_ptr ioservice_pool_ptr,
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr,
			std::shared_ptr<pool_s> send_buf_pool_ptr,
			std::shared_ptr<pool_t> recv_buf_pool_ptr
		)
			: tcp_acceptor_impl<_session_impl_t>(
				ioservice_pool_ptr,
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
		virtual ~http_acceptor_impl()
		{
		}

		http_acceptor_impl & set_request_pool(std::shared_ptr<object_pool<http_request>> request_pool_ptr)
		{
			m_request_pool_ptr = request_pool_ptr;
			return (*this);
		}

	protected:
		virtual std::shared_ptr<_session_impl_t> _prepare_session() override
		{
			std::shared_ptr<_session_impl_t> session_ptr = tcp_acceptor_impl<_session_impl_t>::_prepare_session();

			if (session_ptr)
			{
				session_ptr->set_request_pool(m_request_pool_ptr);
			}

			return session_ptr;
		}

		/// must override all listener functions,and cast the m_listener_mgr_ptr to http_server_listener_mgr,
		/// otherwise it will crash when these listener was called.
	protected:
		virtual void _fire_listen() override
		{
			std::dynamic_pointer_cast<http_server_listener_mgr>(m_listener_mgr_ptr)->notify_listen();
		}

		virtual void _fire_accept(std::shared_ptr<_session_impl_t> session_ptr) override
		{
			std::static_pointer_cast<http_server_listener_mgr>(m_listener_mgr_ptr)->notify_accept(session_ptr);
		}

		virtual void _fire_shutdown(int error) override
		{
			std::dynamic_pointer_cast<http_server_listener_mgr>(m_listener_mgr_ptr)->notify_shutdown(error);
		}

	protected:

		std::shared_ptr<object_pool<http_request>> m_request_pool_ptr;

	};


}

#endif // !__ASIO2_HTTP_ACCEPTOR_IMPL_HPP__
