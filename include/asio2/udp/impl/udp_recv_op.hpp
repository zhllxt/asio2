/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_UDP_RECV_OP_HPP__
#define __ASIO2_UDP_RECV_OP_HPP__

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
	class udp_recv_op
	{
	public:
		/**
		 * @brief constructor
		 */
		udp_recv_op() noexcept {}

		/**
		 * @brief destructor
		 */
		~udp_recv_op() = default;

	protected:
		template<typename C>
		void _udp_post_recv(std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
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

			derive.socket().async_receive(derive.buffer().prepare(derive.buffer().pre_size()),
				make_allocator(derive.rallocator(),
					[&derive, this_ptr = std::move(this_ptr), ecs = std::move(ecs)]
			(const error_code & ec, std::size_t bytes_recvd) mutable
			{
			#if defined(_DEBUG) || defined(DEBUG)
				derive.post_recv_counter_--;
			#endif

				derive.reading_ = false;

				derive._handle_recv(ec, bytes_recvd, std::move(this_ptr), std::move(ecs));
			}));
		}

		template<typename C>
		inline void _udp_do_handle_recv(
			const error_code& ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs)
		{
			using condition_lowest_type = typename ecs_t<C>::condition_lowest_type;

			derived_t& derive = static_cast<derived_t&>(*this);

			if (!ec)
			{
				std::string_view data = std::string_view(static_cast<std::string_view::const_pointer>
					(derive.buffer().data().data()), bytes_recvd);

				if constexpr (!std::is_same_v<typename ecs_t<C>::condition_lowest_type, use_kcp_t>)
				{
					derive._fire_recv(this_ptr, ecs, data);
				}
				else
				{
					if (derive.kcp_stream_)
					{
						derive.kcp_stream_->_kcp_handle_recv(ec, data, this_ptr, ecs);
					}
					else
					{
						ASIO2_ASSERT(false);
						derive._do_disconnect(asio::error::invalid_argument, this_ptr);
						derive._stop_readend_timer(std::move(this_ptr));
					}
				}
			}
			else
			{
				ASIO2_LOG_DEBUG("_udp_handle_recv with error: {} {}", ec.value(), ec.message());

				if (ec == asio::error::eof)
				{
					ASIO2_ASSERT(bytes_recvd == 0);

					if (bytes_recvd)
					{
						// http://www.purecpp.cn/detail?id=2303
						ASIO2_LOG_INFOR("_udp_handle_recv with eof: {}", bytes_recvd);
					}
				}
			}
		}

		template<typename C>
		inline void _udp_session_handle_recv(
			const error_code& ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			derive._udp_do_handle_recv(ec, bytes_recvd, this_ptr, ecs);
		}

		template<typename C>
		void _udp_client_handle_recv(
			const error_code& ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if (ec == asio::error::operation_aborted || ec == asio::error::connection_refused)
			{
				derive._do_disconnect(ec, this_ptr);
				derive._stop_readend_timer(std::move(this_ptr));
				return;
			}

			derive.buffer().commit(bytes_recvd);

			derive._udp_do_handle_recv(ec, bytes_recvd, this_ptr, ecs);

			derive.buffer().consume(derive.buffer().size());

			if (bytes_recvd == derive.buffer().pre_size())
			{
				derive.buffer().pre_size((std::min)(derive.buffer().pre_size() * 2, derive.buffer().max_size()));
			}

			derive._post_recv(this_ptr, ecs);
		}

		template<typename C>
		void _udp_handle_recv(
			const error_code& ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io_->running_in_this_thread());

			set_last_error(ec);

			if (!derive.is_started())
			{
				if (derive.state_ == state_t::started)
				{
					derive._do_disconnect(ec, this_ptr);
				}

				derive._stop_readend_timer(std::move(this_ptr));

				return;
			}

			// every times recv data,we update the last alive time.
			if (!ec)
			{
				derive.update_alive_time();
			}

			if constexpr (args_t::is_session)
			{
				derive._udp_session_handle_recv(ec, bytes_recvd, this_ptr, ecs);
			}
			else
			{
				derive._udp_client_handle_recv(ec, bytes_recvd, this_ptr, ecs);
			}
		}

	protected:
	};
}

#endif // !__ASIO2_UDP_RECV_OP_HPP__
