/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_HTTP_CONNECTION_IMPL_HPP__
#define __ASIO2_HTTP_CONNECTION_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/tcp/tcp_connection_impl.hpp>

namespace asio2
{

	template<class _pool_t>
	class http_connection_impl : public tcp_connection_impl<_pool_t>
	{
	public:

		typedef _pool_t pool_t;

		/**
		 * @construct
		 */
		explicit http_connection_impl(
			std::shared_ptr<io_service> send_ioservice_ptr,
			std::shared_ptr<io_service> recv_ioservice_ptr,
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr,
			std::shared_ptr<pool_s> send_buf_pool_ptr,
			std::shared_ptr<pool_t> recv_buf_pool_ptr
		)
			: tcp_connection_impl<_pool_t>(
				send_ioservice_ptr,
				recv_ioservice_ptr,
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
		virtual ~http_connection_impl()
		{
		}


	};

}

#endif // !__ASIO2_HTTP_CONNECTION_IMPL_HPP__
