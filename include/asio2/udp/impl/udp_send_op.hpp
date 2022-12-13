/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
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
#include <asio2/base/detail/ecs.hpp>

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
						p = derive.selfptr(), callback = std::forward<Callback>(callback)
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

			// note: when on the server, all udp session are used a same socket, so 
			// multiple sessions maybe use the same socket to send data concurrently
			// at the same time.

			derive.stream().async_send_to(asio::buffer(data), endpoint,
				make_allocator(derive.wallocator(),
					[
					#if defined(_DEBUG) || defined(DEBUG)
						&derive,
					#endif
						p = derive.selfptr(), callback = std::forward<Callback>(callback)
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
