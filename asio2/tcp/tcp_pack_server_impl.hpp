/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_TCP_PACK_SERVER_IMPL_HPP__
#define __ASIO2_TCP_PACK_SERVER_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/tcp/tcp_server_impl.hpp>
#include <asio2/tcp/tcp_pack_acceptor_impl.hpp>

namespace asio2
{

	template<class _acceptor_impl_t>
	class tcp_pack_server_impl : public tcp_server_impl<_acceptor_impl_t>
	{
	public:
		typedef _acceptor_impl_t acceptor_impl_t;
		typedef typename acceptor_impl_t::parser_callback parser_callback;

		/**
		 * @construct
		 */
		tcp_pack_server_impl(
			std::shared_ptr<url_parser>                    url_parser_ptr,
			std::shared_ptr<listener_mgr>                  listener_mgr_ptr
		)
			: tcp_server_impl<_acceptor_impl_t>(url_parser_ptr, listener_mgr_ptr)
		{
		}

		/**
		 * @destruct
		 */
		virtual ~tcp_pack_server_impl()
		{
		}

		/**
		 * @function : start the server
		 * @return   : true - start successed , false - start failed
		 */
		virtual bool start() override
		{
			if (!std::static_pointer_cast<_acceptor_impl_t>(this->get_acceptor_impl())->m_pack_parser)
				throw std::runtime_error("must call set_pack_parser to specifies the data parser before start server on pack model");

			return tcp_server_impl<_acceptor_impl_t>::start();
		}

		/**
		 * @function : set the data parser under pack model
		 */
		tcp_pack_server_impl & set_pack_parser(const std::function<parser_callback> & parser)
		{
			std::static_pointer_cast<_acceptor_impl_t>(this->get_acceptor_impl())->m_pack_parser = parser;
			return (*this);
		}

	};

}

#endif // !__ASIO2_TCP_PACK_SERVER_IMPL_HPP__
