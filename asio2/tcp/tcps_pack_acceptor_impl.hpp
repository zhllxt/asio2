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
	public:

		typedef _session_impl_t session_impl_t;
		typedef typename _session_impl_t::pool_t pool_t;

		/**
		 * @construct
		 */
		tcps_pack_acceptor_impl(
			io_service_pool_ptr ioservice_pool_ptr,
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr,
			std::shared_ptr<pool_s> send_buf_pool_ptr,
			std::shared_ptr<pool_t> recv_buf_pool_ptr
		)
			: tcps_acceptor_impl<_session_impl_t>(
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
		virtual ~tcps_pack_acceptor_impl()
		{
		}

		virtual void stop() override
		{
			tcps_acceptor_impl<_session_impl_t>::stop();

			m_pack_parser = nullptr;
		}

		/**
		 * @function : set the data parser under pack model
		 */
		template<typename _parser>
		tcps_pack_acceptor_impl & set_pack_parser(_parser parser)
		{
			m_pack_parser = parser;
			return (*this);
		}

	protected:
		virtual std::shared_ptr<_session_impl_t> _prepare_session() override
		{
			std::shared_ptr<_session_impl_t> session_ptr = tcps_acceptor_impl<_session_impl_t>::_prepare_session();

			if (session_ptr)
			{
				session_ptr->set_pack_parser(m_pack_parser);
			}

			return session_ptr;
		}

	protected:
		
		using parser_callback = std::size_t(std::shared_ptr<buffer<uint8_t>> data_ptr);

		std::function<parser_callback>       m_pack_parser;

	};


}

#endif // !__ASIO2_TCPS_PACK_ACCEPTOR_IMPL_HPP__
