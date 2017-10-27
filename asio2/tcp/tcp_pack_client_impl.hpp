/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_TCP_PACK_CLIENT_IMPL_HPP__
#define __ASIO2_TCP_PACK_CLIENT_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/tcp/tcp_client_impl.hpp>

#include <asio2/tcp/tcp_pack_connection_impl.hpp>

namespace asio2
{

	template<class _connection_impl_t>
	class tcp_pack_client_impl : public tcp_client_impl<_connection_impl_t>
	{
	public:

		/**
		 * @construct
		 */
		explicit tcp_pack_client_impl(
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr
		)
			: tcp_client_impl<_connection_impl_t>(listener_mgr_ptr, url_parser_ptr)
		{
		}

		/**
		 * @destruct
		 */
		virtual ~tcp_pack_client_impl()
		{
		}

		virtual bool start(bool async_connect = true) override
		{
			if (!this->m_pack_parser)
				throw std::runtime_error("must call set_pack_parser to specifies the data parser before start client under pack model");

			return tcp_client_impl<_connection_impl_t>::start(async_connect);
		}

		virtual void stop() override
		{
			tcp_client_impl<_connection_impl_t>::stop();

			m_pack_parser = nullptr;
		}

	public:

		/**
		 * @function : set the data parser under pack model
		 */
		template<typename _parser>
		tcp_pack_client_impl & set_pack_parser(_parser parser)
		{
			this->m_pack_parser = parser;
			return (*this);
		}

	protected:

		virtual void _prepare_connection() override
		{
			tcp_client_impl<_connection_impl_t>::_prepare_connection();

			if (this->m_connection_impl_ptr)
			{
				std::dynamic_pointer_cast<_connection_impl_t>(this->m_connection_impl_ptr)->set_pack_parser(this->m_pack_parser);
			}
		}

	protected:

		using parser_callback = std::size_t(std::shared_ptr<uint8_t> data_ptr, std::size_t len);

		std::function<parser_callback>       m_pack_parser;

	};

}

#endif // !__ASIO2_TCP_PACK_CLIENT_IMPL_HPP__
