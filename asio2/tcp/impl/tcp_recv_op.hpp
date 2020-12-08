/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_TCP_RECV_OP_HPP__
#define __ASIO2_TCP_RECV_OP_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <future>
#include <utility>
#include <string_view>

#include <asio2/base/selector.hpp>
#include <asio2/base/error.hpp>
#include <asio2/base/detail/condition_wrap.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t = void>
	class tcp_recv_op
	{
	public:
		/**
		 * @constructor
		 */
		tcp_recv_op() {}

		/**
		 * @destructor
		 */
		~tcp_recv_op() = default;

	protected:
		template<typename MatchCondition>
		void _tcp_post_recv(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if (!derive.is_started())
				return;

			try
			{
				if constexpr (
					std::is_same_v<MatchCondition, asio::detail::transfer_all_t> ||
					std::is_same_v<MatchCondition, asio::detail::transfer_at_least_t> ||
					std::is_same_v<MatchCondition, asio::detail::transfer_exactly_t> ||
					std::is_same_v<MatchCondition, asio2::detail::hook_buffer_t>)
				{
					asio::async_read(derive.stream(), derive.buffer().base(), condition(),
						asio::bind_executor(derive.io().strand(), make_allocator(derive.rallocator(),
							[&derive, self_ptr = std::move(this_ptr), condition]
					(const error_code& ec, std::size_t bytes_recvd) mutable
					{
						derive._handle_recv(ec, bytes_recvd, std::move(self_ptr), std::move(condition));
					})));
				}
				else
				{
					asio::async_read_until(derive.stream(), derive.buffer().base(), condition(),
						asio::bind_executor(derive.io().strand(), make_allocator(derive.rallocator(),
							[&derive, self_ptr = std::move(this_ptr), condition]
					(const error_code& ec, std::size_t bytes_recvd) mutable
					{
						derive._handle_recv(ec, bytes_recvd, std::move(self_ptr), std::move(condition));
					})));
				}
			}
			catch (system_error & e)
			{
				set_last_error(e);
				derive._do_disconnect(e.code());
			}
		}

		template<typename MatchCondition>
		void _tcp_handle_recv(const error_code & ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			set_last_error(ec);

			// bytes_recvd : The number of bytes in the streambuf's get area up to and including the delimiter.
			if (!ec)
			{
				// every times recv data,we update the last alive time.
				derive.update_alive_time();

				if constexpr (std::is_same_v<MatchCondition, use_dgram_t>)
				{
					const std::uint8_t* buffer = static_cast<const std::uint8_t*>(derive.buffer().data().data());
					if /**/ (std::uint8_t(buffer[0]) < std::uint8_t(254))
					{
						derive._fire_recv(this_ptr, std::string_view(reinterpret_cast<
							std::string_view::const_pointer>(buffer + 1), bytes_recvd - 1), condition);
					}
					else if (std::uint8_t(buffer[0]) == std::uint8_t(254))
					{
						derive._fire_recv(this_ptr, std::string_view(reinterpret_cast<
							std::string_view::const_pointer>(buffer + 1 + 2), bytes_recvd - 1 - 2), condition);
					}
					else
					{
						ASIO2_ASSERT(std::uint8_t(buffer[0]) == std::uint8_t(255));
						derive._fire_recv(this_ptr, std::string_view(reinterpret_cast<
							std::string_view::const_pointer>(buffer + 1 + 8), bytes_recvd - 1 - 8), condition);
					}
				}
				else
				{
					if constexpr (!std::is_same_v<MatchCondition, asio2::detail::hook_buffer_t>)
					{
						derive._fire_recv(this_ptr, std::string_view(reinterpret_cast<
							std::string_view::const_pointer>(derive.buffer().data().data()), bytes_recvd), condition);
					}
					else
					{
						derive._fire_recv(this_ptr, std::string_view(reinterpret_cast<
							std::string_view::const_pointer>(derive.buffer().data().data()),
							derive.buffer().size()), condition);
					}
				}

				if constexpr (!std::is_same_v<MatchCondition, asio2::detail::hook_buffer_t>)
				{
					derive.buffer().consume(bytes_recvd);
				}
				else
				{
					std::ignore = true;
				}

				derive._post_recv(std::move(this_ptr), std::move(condition));
			}
			else
			{
				derive._do_disconnect(ec);
			}
			// If an error occurs then no new asynchronous operations are started. This
			// means that all shared_ptr references to the connection object will
			// disappear and the object will be destroyed automatically after this
			// handler returns. The connection class's destructor closes the socket.
		}

	protected:
	};
}

#endif // !__ASIO2_TCP_RECV_OP_HPP__
