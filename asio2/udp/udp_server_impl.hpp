/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_UDP_SERVER_IMPL_HPP__
#define __ASIO2_UDP_SERVER_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/server_impl.hpp>

#include <asio2/udp/udp_acceptor_impl.hpp>

namespace asio2
{

	template<class _acceptor_impl_t>
	class udp_server_impl : public server_impl
	{
	public:
		typedef _acceptor_impl_t acceptor_impl_t;
		typedef typename _acceptor_impl_t::session_impl_t session_impl_t;

		/**
		 * @construct
		 */
		udp_server_impl(
			std::shared_ptr<url_parser>                    url_parser_ptr,
			std::shared_ptr<listener_mgr>                  listener_mgr_ptr
		)
			: server_impl(url_parser_ptr, listener_mgr_ptr)
		{
			m_acceptor_impl_ptr = std::make_shared<_acceptor_impl_t>(url_parser_ptr, listener_mgr_ptr, m_io_context_pool_ptr);
		}

		/**
		 * @destruct
		 */
		virtual ~udp_server_impl()
		{
		}

	};
}

#endif // !__ASIO2_UDP_SERVER_IMPL_HPP__
