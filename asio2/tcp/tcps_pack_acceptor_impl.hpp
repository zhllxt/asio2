/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 *
 */

#ifndef __ASIO2_TCPS_PACK_ACCEPTOR_IMPL_HPP__
#define __ASIO2_TCPS_PACK_ACCEPTOR_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/tcp/tcps_acceptor_impl.hpp>
#include <asio2/tcp/tcps_pack_session_impl.hpp>

namespace asio2
{

	template<class _session_impl_t>
	class tcps_pack_acceptor_impl : public tcps_acceptor_impl<_session_impl_t>
	{
		template<class _acceptor_impl_t> friend class tcps_pack_server_impl;

	public:
		typedef _session_impl_t session_impl_t;
		typedef typename session_impl_t::parser_callback parser_callback;

		/**
		 * @construct
		 */
		tcps_pack_acceptor_impl(
			std::shared_ptr<url_parser>             url_parser_ptr,
			std::shared_ptr<listener_mgr>           listener_mgr_ptr,
			std::shared_ptr<io_context_pool>        io_context_pool_ptr,
			std::shared_ptr<asio::ssl::context>     ssl_context_ptr = nullptr
		)
			: tcps_acceptor_impl<_session_impl_t>(
				url_parser_ptr,
				listener_mgr_ptr,
				io_context_pool_ptr,
				ssl_context_ptr
				)
		{
		}

		/**
		 * @destruct
		 */
		virtual ~tcps_pack_acceptor_impl()
		{
		}

	protected:
		virtual std::shared_ptr<_session_impl_t> _make_session() override
		{
			try
			{
				return std::make_shared<_session_impl_t>(
					this->m_url_parser_ptr,
					this->m_listener_mgr_ptr,
					this->m_io_context_pool_ptr->get_io_context_ptr(),
					this->m_session_mgr_ptr,
					this->m_ssl_context_ptr,
					this->m_pack_parser
					);
			}
			// handle exception,may be is the exception "Too many open files" (exception code : 24)
			catch (asio::system_error & e)
			{
				set_last_error(e.code().value());

				PRINT_EXCEPTION;
			}
			return nullptr;
		}

	protected:
		std::function<parser_callback>       m_pack_parser;

	};


}

#endif // !__ASIO2_TCPS_PACK_ACCEPTOR_IMPL_HPP__
