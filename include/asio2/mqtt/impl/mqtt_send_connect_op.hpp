/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_SEND_CONNECT_OP_HPP__
#define __ASIO2_SEND_CONNECT_OP_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/iopool.hpp>
#include <asio2/base/define.hpp>

#include <asio2/base/detail/util.hpp>

#include <asio2/mqtt/message.hpp>

namespace asio2::detail
{
	template<class SocketT, class HandlerT>
	class mqtt_send_connect_op : public asio::coroutine
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

		std::vector<char>                connect_buffer{};

		std::unique_ptr<asio::streambuf> stream{ std::make_unique<asio::streambuf>() };

		template<class SKT, class H>
		mqtt_send_connect_op(asio::io_context& context, mqtt::message& connect_msg, SKT& skt, H&& h)
			: context_(context)
			, socket_ (skt)
			, handler_(std::forward<H>(h))
		{
			std::visit([this](auto& message) mutable { message.serialize(connect_buffer); }, connect_msg.base());

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

				ASIO_CORO_YIELD
				{
					auto buffer = asio::buffer(connect_buffer);
					asio::async_write(socket_, buffer, asio::transfer_exactly(buffer.size()), std::move(*this));
				}
				if (ec)
					goto end;

				// The server send a connack message or auth message.

				ASIO_CORO_YIELD
				{
					asio::streambuf& strbuf = *stream;
					asio::async_read_until(socket_, strbuf, mqtt::mqtt_match_role, std::move(*this));
				}
				if (ec)
					goto end;

			end:
				handler_(ec, std::move(stream));
			}
		}
	};

	// C++17 class template argument deduction guides
	template<class SKT, class H>
	mqtt_send_connect_op(asio::io_context&, mqtt::message&, SKT&, H)->mqtt_send_connect_op<SKT, H>;
}

#endif // !__ASIO2_SEND_CONNECT_OP_HPP__
