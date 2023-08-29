/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_HTTP_RECV_OP_HPP__
#define __ASIO2_HTTP_RECV_OP_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <future>
#include <utility>
#include <string_view>

#include <asio2/external/asio.hpp>
#include <asio2/external/beast.hpp>

#include <asio2/base/error.hpp>

#include <asio2/http/detail/http_util.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t>
	class http_recv_op
	{
	public:
		using body_type   = typename args_t::body_t;
		using buffer_type = typename args_t::buffer_t;

		/**
		 * @brief constructor
		 */
		http_recv_op() noexcept {}

		/**
		 * @brief destructor
		 */
		~http_recv_op() = default;

	protected:
		template<typename C>
		void _http_session_post_recv(std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if (derive.is_http())
			{
				// Make the request empty before reading,
				// otherwise the operation behavior is undefined.
				derive.req_.reset();

			#if defined(_DEBUG) || defined(DEBUG)
				ASIO2_ASSERT(derive.post_recv_counter_.load() == 0);
				derive.post_recv_counter_++;
			#endif

				ASIO2_ASSERT(derive.reading_ == false);

				derive.reading_ = true;

				// Read a request
				http::async_read(derive.stream(), derive.buffer().base(), derive.req_,
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
			else
			{
			#if defined(_DEBUG) || defined(DEBUG)
				ASIO2_ASSERT(derive.post_recv_counter_.load() == 0);
				derive.post_recv_counter_++;
			#endif

				ASIO2_ASSERT(derive.reading_ == false);

				derive.reading_ = true;

				// Read a message into our buffer
				derive.ws_stream().async_read(derive.buffer().base(),
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
		}

		template<typename C>
		void _http_client_post_recv(std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// Make the request empty before reading,
			// otherwise the operation behavior is undefined.
			derive.rep_.reset();

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(derive.post_recv_counter_.load() == 0);
			derive.post_recv_counter_++;
		#endif

			ASIO2_ASSERT(derive.reading_ == false);

			derive.reading_ = true;

			// Receive the HTTP response
			http::async_read(derive.stream(), derive.buffer().base(), derive.rep_,
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
		void _http_post_recv(std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
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

			if constexpr (args_t::is_session)
			{
				derive._http_session_post_recv(std::move(this_ptr), std::move(ecs));
			}
			else
			{
				derive._http_client_post_recv(std::move(this_ptr), std::move(ecs));
			}
		}

		template<typename C>
		void _http_session_handle_recv(
			const error_code& ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			detail::ignore_unused(ec, bytes_recvd);

			derived_t& derive = static_cast<derived_t&>(*this);

			if (derive.is_http())
			{
				derive.req_.url_.reset(derive.req_.target());

				derive.rep_.result(http::status::unknown);
				derive.rep_.keep_alive(derive.req_.keep_alive());

				if (derive._check_upgrade(this_ptr, ecs))
					return;

				derive._fire_recv(this_ptr, ecs);

				// note : can't read write the variable of "req_" after _fire_recv, it maybe
				// cause crash, eg :
				// user called response.defer() in the recv callback, and pass the defer_ptr
				// into another thread, then code run to here, at this time, the "req_" maybe
				// read write in two thread : this thread and "another thread".
				// note : can't call "_do_disconnect" at here, beacuse if user has called
				// response.defer() in the recv callback, this session maybe closed before
				// the response is sent to the client.
				//if (derive.req_.need_eof() || !derive.req_.keep_alive())
				//{
				//	derive._do_disconnect(asio::error::operation_aborted, derive.selfptr());
				//	return;
				//}
			}
			else
			{
				derive.req_.ws_frame_type_ = websocket::frame::message;
				derive.req_.ws_frame_data_ = { reinterpret_cast<std::string_view::const_pointer>(
					derive.buffer().data().data()), bytes_recvd };

				derive._fire_recv(this_ptr, ecs);

				derive.buffer().consume(derive.buffer().size());

				derive._post_recv(std::move(this_ptr), std::move(ecs));
			}
		}

		template<typename C>
		void _http_client_handle_recv(
			const error_code& ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			detail::ignore_unused(ec, bytes_recvd);

			derived_t& derive = static_cast<derived_t&>(*this);

			derive._fire_recv(this_ptr, ecs);

			derive._post_recv(std::move(this_ptr), std::move(ecs));
		}

		template<typename C>
		void _http_handle_recv(
			const error_code& ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io_->running_in_this_thread());

			set_last_error(ec);

			if (!derive.is_started())
			{
				if (derive.state_ == state_t::started)
				{
					ASIO2_LOG_INFOR("_http_handle_recv with closed socket: {} {}", ec.value(), ec.message());

					derive._do_disconnect(ec, this_ptr);
				}

				derive._stop_readend_timer(std::move(this_ptr));

				return;
			}

			if (!ec)
			{
				// every times recv data,we update the last alive time.
				derive.update_alive_time();

				if constexpr (args_t::is_session)
				{
					derive._http_session_handle_recv(ec, bytes_recvd, std::move(this_ptr), std::move(ecs));
				}
				else
				{
					derive._http_client_handle_recv(ec, bytes_recvd, std::move(this_ptr), std::move(ecs));
				}
			}
			else
			{
				ASIO2_LOG_DEBUG("_http_handle_recv with error: {} {}", ec.value(), ec.message());

				// This means they closed the connection
				//if (ec == http::error::end_of_stream)
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

#endif // !__ASIO2_HTTP_RECV_OP_HPP__
