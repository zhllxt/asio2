/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_UDP_SENDER_IMPL_HPP__
#define __ASIO2_UDP_SENDER_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/sender_impl.hpp>
#include <asio2/udp/udp_transmitter_impl.hpp>

namespace asio2
{

	template<class _transmitter_impl_t>
	class udp_sender_impl : public sender_impl
	{
	public:
		/**
		 * @construct
		 */
		explicit udp_sender_impl(
			std::shared_ptr<url_parser>              url_parser_ptr,
			std::shared_ptr<listener_mgr>            listener_mgr_ptr
		)
			: sender_impl(url_parser_ptr, listener_mgr_ptr)
		{
			if (listener_mgr_ptr)
			{
				this->m_transmitter_impl_ptr = std::make_shared<_transmitter_impl_t>(url_parser_ptr, listener_mgr_ptr,
					m_io_context_pool_ptr->get_io_context_ptr(), m_io_context_pool_ptr->get_io_context_ptr());
			}
		}

		/**
		 * @destruct
		 */
		virtual ~udp_sender_impl()
		{
		}

	};
}

#endif // !__ASIO2_UDP_SENDER_IMPL_HPP__
