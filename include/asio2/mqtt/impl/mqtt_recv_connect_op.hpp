/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_RECV_CONNECT_OP_HPP__
#define __ASIO2_RECV_CONNECT_OP_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/iopool.hpp>
#include <asio2/base/define.hpp>

#include <asio2/base/detail/util.hpp>

#include <asio2/mqtt/core.hpp>

namespace asio2::detail
{
	template<class SocketT, class HandlerT>
	class mqtt_recv_connect_op : public asio::coroutine
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;

	public:
		asio::io_context        & context_;

		SocketT&       socket_;
		HandlerT       handler_;

		std::unique_ptr<asio::streambuf> stream{ std::make_unique<asio::streambuf>() };

		template<class SKT, class H>
		mqtt_recv_connect_op(asio::io_context& context, SKT& skt, H&& h)
			: context_(context)
			, socket_ (skt)
			, handler_(std::forward<H>(h))
		{
			(*this)();
		}

		template<typename = void>
		void operator()(error_code ec = {}, std::size_t bytes_transferred = 0)
		{
			detail::ignore_unused(ec, bytes_transferred);

			// There is no need to use a timeout timer because there is already has
			// connect_timeout_cp

			ASIO_CORO_REENTER(*this)
			{
				// The client connects to the server, and sends a connect message

				// The server wait for recv the connect message

				ASIO_CORO_YIELD
				{
					asio::streambuf& strbuf = *stream;
					asio::async_read_until(socket_, strbuf, mqtt::mqtt_match_role, std::move(*this));
				}

				handler_(ec, std::move(stream));
			}
		}
	};

	// C++17 class template argument deduction guides
	template<class SKT, class H>
	mqtt_recv_connect_op(asio::io_context&, SKT&, H)->mqtt_recv_connect_op<SKT, H>;
}

#endif // !__ASIO2_RECV_CONNECT_OP_HPP__
