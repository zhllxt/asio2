/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 *
 */

#ifndef __ASIO2_TCPS_ACCEPTOR_IMPL_HPP__
#define __ASIO2_TCPS_ACCEPTOR_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/ssl.hpp>

#include <asio2/tcp/tcp_acceptor_impl.hpp>

#include <asio2/tcp/tcps_session_impl.hpp>
#include <asio2/tcp/tcps_auto_session_impl.hpp>

namespace asio2
{

	template<class _session_impl_t>
	class tcps_acceptor_impl : public tcp_acceptor_impl<_session_impl_t>
	{
	public:

		typedef _session_impl_t session_impl_t;
		typedef typename _session_impl_t::pool_t pool_t;

		/**
		 * @construct
		 */
		tcps_acceptor_impl(
			io_service_pool_ptr io_service_pool_evt_ptr,
			io_service_pool_ptr io_service_pool_msg_ptr,
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr,
			std::shared_ptr<pool_t> recv_buf_pool_ptr
		)
			: tcp_acceptor_impl<_session_impl_t>(
				io_service_pool_evt_ptr,
				io_service_pool_msg_ptr,
				listener_mgr_ptr,
				url_parser_ptr,
				recv_buf_pool_ptr
				)
		{
		}

		/**
		 * @destruct
		 */
		virtual ~tcps_acceptor_impl()
		{
		}

		virtual void stop() override
		{
			tcp_acceptor_impl<_session_impl_t>::stop();

			m_context_ptr.reset();
		}

		tcps_acceptor_impl & set_context(std::shared_ptr<boost::asio::ssl::context> context_ptr)
		{
			m_context_ptr = context_ptr;
			return (*this);
		}

	protected:

		virtual std::shared_ptr<_session_impl_t> _prepare_session()
		{
			std::shared_ptr<_session_impl_t> session_ptr = tcp_acceptor_impl<_session_impl_t>::_prepare_session();

			if (session_ptr)
			{
				session_ptr->set_context(m_context_ptr);
				
				return session_ptr;
			}

			return nullptr;
		}

	protected:
		
		/// ssl context 
		std::shared_ptr<boost::asio::ssl::context> m_context_ptr;

	};


}

#endif // !__ASIO2_TCPS_ACCEPTOR_IMPL_HPP__
