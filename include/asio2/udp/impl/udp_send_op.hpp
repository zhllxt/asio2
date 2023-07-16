/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_UDP_SEND_OP_HPP__
#define __ASIO2_UDP_SEND_OP_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <future>
#include <utility>
#include <string_view>

#include <asio2/base/error.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t>
	class udp_send_op
	{
	public:
		/**
		 * @brief constructor
		 */
		udp_send_op() noexcept {}

		/**
		 * @brief destructor
		 */
		~udp_send_op() = default;

	protected:
		template<class Data, class Callback>
		inline bool _udp_send(Data& data, Callback&& callback)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(derive.post_send_counter_.load() == 0);
			derive.post_send_counter_++;
		#endif

			derive.stream().async_send(asio::buffer(data),
				make_allocator(derive.wallocator(),
					[
					#if defined(_DEBUG) || defined(DEBUG)
						&derive,
					#endif
						callback = std::forward<Callback>(callback)
					]
			(const error_code& ec, std::size_t bytes_sent) mutable
			{
			#if defined(_DEBUG) || defined(DEBUG)
				derive.post_send_counter_--;
			#endif

				set_last_error(ec);

				callback(ec, bytes_sent);
			}));
			return true;
		}

		template<class Endpoint, class Data, class Callback>
		inline bool _udp_send_to(Endpoint& endpoint, Data& data, Callback&& callback)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(derive.post_send_counter_.load() == 0);
			derive.post_send_counter_++;
		#endif

			// issue: when on the server, all udp session are used a same socket, so 
			// multiple sessions maybe use the same socket to send data concurrently
			// at the same time. But the test can't detect this problem. On windows
			// platform, the WSASendTo always success and return 0.

			derive.stream().async_send_to(asio::buffer(data), endpoint,
				make_allocator(derive.wallocator(),
					[
					#if defined(_DEBUG) || defined(DEBUG)
						&derive,
					#endif
						callback = std::forward<Callback>(callback)
					]
			(const error_code& ec, std::size_t bytes_sent) mutable
			{
			#if defined(_DEBUG) || defined(DEBUG)
				derive.post_send_counter_--;
			#endif

				set_last_error(ec);

				callback(ec, bytes_sent);
			}));
			return true;
		}

	protected:
	};
}

#endif // !__ASIO2_UDP_SEND_OP_HPP__
