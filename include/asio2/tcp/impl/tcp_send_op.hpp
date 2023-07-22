/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_TCP_SEND_OP_HPP__
#define __ASIO2_TCP_SEND_OP_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <future>
#include <utility>
#include <string_view>

#include <asio2/base/error.hpp>
#include <asio2/base/detail/buffer_wrap.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t>
	class tcp_send_op
	{
	protected:
		template<class, class = std::void_t<>>
		struct has_member_dgram : std::false_type {};

		template<class T>
		struct has_member_dgram<T, std::void_t<decltype(T::dgram_)>> : std::true_type {};

		//template<class T>
		//struct has_member_dgram<T, std::void_t<decltype(T::dgram_), std::enable_if_t<
		//	std::is_same_v<decltype(T::dgram_), bool>>>> : std::true_type {};

	public:
		/**
		 * @brief constructor
		 */
		tcp_send_op() noexcept {}

		/**
		 * @brief destructor
		 */
		~tcp_send_op() = default;

	protected:
		template<class Data, class Callback>
		inline bool _tcp_send(Data& data, Callback&& callback)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if constexpr (has_member_dgram<derived_t>::value)
			{
				if (derive.dgram_)
				{
					return derive._tcp_send_dgram(asio::buffer(data), std::forward<Callback>(callback));
				}
			}
			else
			{
				std::ignore = true;
			}

			return derive._tcp_send_general(asio::buffer(data), std::forward<Callback>(callback));
		}

		template<class Buffer, class Callback>
		inline bool _tcp_send_dgram(Buffer&& buffer, Callback&& callback)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			int bytes = 0;
			std::unique_ptr<std::uint8_t[]> head;

			// why don't use std::string for "head"?
			// beacuse std::string has a SSO(Small String Optimization) mechanism
			// https://stackoverflow.com/questions/34788789/disable-stdstrings-sso
			// std::string str;
			// str.reserve(sizeof(str) + 1);

			// note : need ensure big endian and little endian
			if (buffer.size() < std::size_t(254))
			{
				bytes = 1;
				head = std::make_unique<std::uint8_t[]>(bytes);
				head[0] = static_cast<std::uint8_t>(buffer.size());
			}
			else if (buffer.size() <= (std::numeric_limits<std::uint16_t>::max)())
			{
				bytes = 3;
				head = std::make_unique<std::uint8_t[]>(bytes);
				head[0] = static_cast<std::uint8_t>(254);
				std::uint16_t size = static_cast<std::uint16_t>(buffer.size());
				std::memcpy(&head[1], reinterpret_cast<const void*>(&size), sizeof(std::uint16_t));
				// use little endian
				if (!is_little_endian())
				{
					swap_bytes<sizeof(std::uint16_t)>(&head[1]);
				}
			}
			else
			{
				ASIO2_ASSERT(buffer.size() > (std::numeric_limits<std::uint16_t>::max)());
				bytes = 9;
				head = std::make_unique<std::uint8_t[]>(bytes);
				head[0] = static_cast<std::uint8_t>(255);
				std::uint64_t size = buffer.size();
				std::memcpy(&head[1], reinterpret_cast<const void*>(&size), sizeof(std::uint64_t));
				// use little endian
				if (!is_little_endian())
				{
					swap_bytes<sizeof(std::uint64_t)>(&head[1]);
				}
			}

			std::array<asio::const_buffer, 2> buffers
			{
				asio::buffer(reinterpret_cast<const void*>(head.get()), bytes),
				std::forward<Buffer>(buffer)
			};

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(derive.post_send_counter_.load() == 0);
			derive.post_send_counter_++;
		#endif

			asio::async_write(derive.stream(), buffers, make_allocator(derive.wallocator(),
			[&derive, bytes, head = std::move(head), callback = std::forward<Callback>(callback)]
			(const error_code& ec, std::size_t bytes_sent) mutable
			{
			#if defined(_DEBUG) || defined(DEBUG)
				derive.post_send_counter_--;
			#endif

				set_last_error(ec);

				if (ec)
				{
					callback(ec, bytes_sent);

					// must stop, otherwise re-sending will cause header confusion
					if (derive.state_ == state_t::started)
					{
						derive._do_disconnect(ec, derive.selfptr());
					}
				}
				else
				{
					callback(ec, bytes_sent - bytes);
				}
			}));
			return true;
		}

		template<class Buffer, class Callback>
		inline bool _tcp_send_general(Buffer&& buffer, Callback&& callback)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(derive.post_send_counter_.load() == 0);
			derive.post_send_counter_++;
		#endif

			asio::async_write(derive.stream(), buffer, make_allocator(derive.wallocator(),
			[&derive, callback = std::forward<Callback>(callback)]
			(const error_code& ec, std::size_t bytes_sent) mutable
			{
			#if defined(_DEBUG) || defined(DEBUG)
				derive.post_send_counter_--;
			#endif

				set_last_error(ec);

				callback(ec, bytes_sent);

				if (ec)
				{
					// must stop, otherwise re-sending will cause body confusion
					if (derive.state_ == state_t::started)
					{
						derive._do_disconnect(ec, derive.selfptr());
					}
				}
			}));
			return true;
		}

	protected:
	};
}

#endif // !__ASIO2_TCP_SEND_OP_HPP__
