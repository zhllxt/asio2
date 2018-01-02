/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_UDP_CLIENT_IMPL_HPP__
#define __ASIO2_UDP_CLIENT_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/client_impl.hpp>
#include <asio2/udp/udp_connection_impl.hpp>

namespace asio2
{

	template<class _connection_impl_t>
	class udp_client_impl : public client_impl
	{
	public:
		/**
		 * @construct
		 */
		explicit udp_client_impl(
			std::shared_ptr<url_parser>                    url_parser_ptr,
			std::shared_ptr<listener_mgr>                  listener_mgr_ptr
		)
			: client_impl(url_parser_ptr, listener_mgr_ptr)
		{
			m_connection_impl_ptr = std::make_shared<_connection_impl_t>(url_parser_ptr, listener_mgr_ptr,
				m_io_context_pool_ptr->get_io_context_ptr(), m_io_context_pool_ptr->get_io_context_ptr());
		}

		/**
		 * @destruct
		 */
		virtual ~udp_client_impl()
		{
		}

	};
}

#endif // !__ASIO2_UDP_CLIENT_IMPL_HPP__
