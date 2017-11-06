/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_TCPS_PACK_SERVER_IMPL_HPP__
#define __ASIO2_TCPS_PACK_SERVER_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/ssl.hpp>

#include <asio2/tcp/tcps_server_impl.hpp>

#include <asio2/tcp/tcps_pack_acceptor_impl.hpp>

namespace asio2
{

	template<class _acceptor_impl_t>
	class tcps_pack_server_impl : public tcps_server_impl<_acceptor_impl_t>
	{
	public:

		/**
		 * @construct
		 */
		tcps_pack_server_impl(
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr
		)
			: tcps_server_impl<_acceptor_impl_t>(listener_mgr_ptr, url_parser_ptr)
		{
		}

		/**
		 * @destruct
		 */
		virtual ~tcps_pack_server_impl()
		{
		}

		virtual bool start() override
		{
			if (!m_pack_parser)
				throw std::runtime_error("must call set_pack_parser to specifies the data parser before start server under pack model");

			return tcps_server_impl<_acceptor_impl_t>::start();
		}

		virtual void stop() override
		{
			tcps_server_impl<_acceptor_impl_t>::stop();

			m_pack_parser = nullptr;
		}

		/**
		 * @function : set the data parser under pack model
		 */
		template<typename _parser>
		tcps_pack_server_impl & set_pack_parser(_parser parser)
		{
			m_pack_parser = parser;
			return (*this);
		}

	protected:

		virtual bool _start_listen() override
		{
			try
			{
				this->m_acceptor_impl_ptr = std::make_shared<_acceptor_impl_t>(
					this->m_ioservice_pool_ptr,
					this->m_listener_mgr_ptr,
					this->m_url_parser_ptr,
					this->m_send_buf_pool_ptr,
					this->m_recv_buf_pool_ptr
					);

				std::dynamic_pointer_cast<_acceptor_impl_t>(this->m_acceptor_impl_ptr)->set_pack_parser(m_pack_parser);

				return this->m_acceptor_impl_ptr->start();
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());
			}

			return false;
		}

	protected:

		using parser_callback = std::size_t(std::shared_ptr<buffer<uint8_t>> data_ptr);

		std::function<parser_callback>       m_pack_parser;

	};

}

#endif // !__ASIO2_TCPS_PACK_SERVER_IMPL_HPP__
