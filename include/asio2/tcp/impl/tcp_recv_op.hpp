/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
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

#include <asio2/base/error.hpp>
#include <asio2/base/detail/ecs.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t>
	class tcp_recv_op
	{
	protected:
		template<class, class = std::void_t<>>
		struct has_member_dgram : std::false_type {};

		template<class T>
		struct has_member_dgram<T, std::void_t<decltype(T::dgram_)>> : std::true_type {};

	public:
		/**
		 * @brief constructor
		 */
		tcp_recv_op() noexcept {}

		/**
		 * @brief destructor
		 */
		~tcp_recv_op() = default;

	protected:
		template<typename C>
		void _tcp_post_recv(std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			using condition_lowest_type = typename ecs_t<C>::condition_lowest_type;

			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io_->running_in_this_thread());

			if (!derive.is_started())
			{
				if (derive.state_ == state_t::started)
				{
					derive._do_disconnect(asio2::get_last_error(), std::move(this_ptr));
				}
				return;
			}

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(derive.post_recv_counter_.load() == 0);
			derive.post_recv_counter_++;
		#endif

			ASIO2_ASSERT(derive.reading_ == false);

			derive.reading_ = true;

			ecs_t<C>& e = *ecs;

			if constexpr (
				std::is_same_v<condition_lowest_type, asio::detail::transfer_all_t> ||
				std::is_same_v<condition_lowest_type, asio::detail::transfer_at_least_t> ||
				std::is_same_v<condition_lowest_type, asio::detail::transfer_exactly_t> ||
				std::is_same_v<condition_lowest_type, asio2::detail::hook_buffer_t>)
			{
				asio::async_read(derive.stream(), derive.buffer().base(), e.get_condition().lowest(),
					make_allocator(derive.rallocator(),
						[&derive, this_ptr = std::move(this_ptr), ecs = std::move(ecs)]
				(const error_code& ec, std::size_t bytes_recvd) mutable
				{
				#if defined(_DEBUG) || defined(DEBUG)
					derive.post_recv_counter_--;
				#endif

					derive.reading_ = false;

					derive._handle_recv(ec, bytes_recvd, std::move(this_ptr), std::move(ecs));
				}));
			}
			else
			{
				asio::async_read_until(derive.stream(), derive.buffer().base(), e.get_condition().lowest(),
					make_allocator(derive.rallocator(),
						[&derive, this_ptr = std::move(this_ptr), ecs = std::move(ecs)]
				(const error_code& ec, std::size_t bytes_recvd) mutable
				{
				#if defined(_DEBUG) || defined(DEBUG)
					derive.post_recv_counter_--;
				#endif

					derive.reading_ = false;

					derive._handle_recv(ec, bytes_recvd, std::move(this_ptr), std::move(ecs));
				}));
			}
		}

		template<typename C>
		void _tcp_dgram_fire_recv(
			const error_code& ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs)
		{
			detail::ignore_unused(ec);

			derived_t& derive = static_cast<derived_t&>(*this);

			const std::uint8_t* buffer = static_cast<const std::uint8_t*>(derive.buffer().data().data());
			if /**/ (std::uint8_t(buffer[0]) < std::uint8_t(254))
			{
				derive._fire_recv(this_ptr, ecs, std::string_view(reinterpret_cast<
					std::string_view::const_pointer>(buffer + 1), bytes_recvd - 1));
			}
			else if (std::uint8_t(buffer[0]) == std::uint8_t(254))
			{
				derive._fire_recv(this_ptr, ecs, std::string_view(reinterpret_cast<
					std::string_view::const_pointer>(buffer + 1 + 2), bytes_recvd - 1 - 2));
			}
			else
			{
				ASIO2_ASSERT(std::uint8_t(buffer[0]) == std::uint8_t(255));
				derive._fire_recv(this_ptr, ecs, std::string_view(reinterpret_cast<
					std::string_view::const_pointer>(buffer + 1 + 8), bytes_recvd - 1 - 8));
			}
		}

		template<typename C>
		void _tcp_handle_recv(
			const error_code& ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			using condition_lowest_type = typename ecs_t<C>::condition_lowest_type;

			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io_->running_in_this_thread());

			set_last_error(ec);

			// bytes_recvd : The number of bytes in the streambuf's get area up to and including the delimiter.

			// After testing, it is found that even if the "ec" is 0, the socket may have been closed already,
			// the stop function of the base class has been called already, and the member variable resources
			// have been destroyed already, so we need check the "derive.is_started()" to ensure that the user
			// maybe read write the member variable resources in the recv callback, and it will cause crash.
			// the code can't be like this:
			//if (!ec && derive.is_started())
			//{
			//	_fire_recv(...);
			//	_post_recv(...);
			//}
			//else
			//{
			//	// after call client.stop, and user call client.start again, when code run to here, the state
			//	// maybe changed to starting by the client.start, if we call _do_disconnect at here, the state
			//	// is changed to stopping again, this will break the client.start processing.
			//	_do_disconnect(...);
			//}
			if (!derive.is_started())
			{
				if (derive.state_ == state_t::started)
				{
					ASIO2_LOG_INFOR("_tcp_handle_recv with closed socket: {} {}", ec.value(), ec.message());

					derive._do_disconnect(ec, this_ptr);
				}

				derive._stop_readend_timer(std::move(this_ptr));

				return;
			}

			if (!ec)
			{
				// every times recv data,we update the last alive time.
				derive.update_alive_time();

				if constexpr (std::is_same_v<condition_lowest_type, use_dgram_t>)
				{
					if constexpr (has_member_dgram<derived_t>::value)
					{
						if (bytes_recvd == 0)
						{
							derive._do_disconnect(asio::error::no_data, this_ptr);
							derive._stop_readend_timer(std::move(this_ptr));
							return;
						}
					}
					else
					{
						ASIO2_ASSERT(false);
					}

					derive._tcp_dgram_fire_recv(ec, bytes_recvd, this_ptr, ecs);
				}
				else
				{
					if constexpr (!std::is_same_v<condition_lowest_type, asio2::detail::hook_buffer_t>)
					{
						derive._fire_recv(this_ptr, ecs, std::string_view(reinterpret_cast<
							std::string_view::const_pointer>(derive.buffer().data().data()), bytes_recvd));
					}
					else
					{
						derive._fire_recv(this_ptr, ecs, std::string_view(reinterpret_cast<
							std::string_view::const_pointer>(derive.buffer().data().data()),
							derive.buffer().size()));
					}
				}

				if constexpr (!std::is_same_v<condition_lowest_type, asio2::detail::hook_buffer_t>)
				{
					derive.buffer().consume(bytes_recvd);
				}
				else
				{
					std::ignore = true;
				}

				derive._post_recv(std::move(this_ptr), std::move(ecs));
			}
			else
			{
				ASIO2_LOG_DEBUG("_tcp_handle_recv with error: {} {}", ec.value(), ec.message());

				if (ec == asio::error::eof)
				{
					// /beast/http/impl/read.hpp
					//if (ec == net::error::eof)
					//{
					//	BHO_ASSERT(bytes_transferred == 0);
					//	...
					//}

					ASIO2_ASSERT(bytes_recvd == 0);

					if (bytes_recvd)
					{
						// http://www.purecpp.cn/detail?id=2303
						ASIO2_LOG_INFOR("_tcp_handle_recv with eof: {}", bytes_recvd);
					}
				}

				derive._do_disconnect(ec, this_ptr);

				derive._stop_readend_timer(std::move(this_ptr));
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
