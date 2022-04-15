/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_SEND_CONNECT_OP_HPP__
#define __ASIO2_SEND_CONNECT_OP_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/external/asio.hpp>
#include <asio2/base/iopool.hpp>
#include <asio2/base/error.hpp>
#include <asio2/base/define.hpp>

#include <asio2/base/detail/util.hpp>

#include <asio2/mqtt/mqtt_protocol_util.hpp>

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
		asio::io_context::strand& strand_;

		SocketT&       socket_;
		HandlerT       handler_;

		std::vector<char>                connect_buffer{};

		std::unique_ptr<asio::streambuf> stream{ std::make_unique<asio::streambuf>() };

		template<class SKT, class H>
		mqtt_send_connect_op(
			asio::io_context& context, asio::io_context::strand& strand,
			std::variant<mqtt::v3::connect, mqtt::v4::connect, mqtt::v5::connect>& connect_msg,
			SKT& skt, H&& h
		)
			: context_(context)
			, strand_ (strand )
			, socket_ (skt)
			, handler_(std::forward<H>(h))
		{
			std::visit([this](auto& message) mutable { message.serialize(connect_buffer); }, connect_msg);

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
					asio::async_write(socket_, buffer, asio::transfer_exactly(buffer.size()),
						asio::bind_executor(strand_, std::move(*this)));
				}
				if (ec)
					goto end;

				// The server send a connack message or auth message.

				ASIO_CORO_YIELD
				{
					asio::streambuf& strbuf = *stream;
					asio::async_read_until(socket_, strbuf, mqtt::mqtt_match_role,
						asio::bind_executor(strand_, std::move(*this)));
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
	mqtt_send_connect_op(asio::io_context&, asio::io_context::strand&,
		std::variant<mqtt::v3::connect, mqtt::v4::connect, mqtt::v5::connect>&,
		SKT&, H)->mqtt_send_connect_op<SKT, H>;
}

#endif // !__ASIO2_SEND_CONNECT_OP_HPP__
