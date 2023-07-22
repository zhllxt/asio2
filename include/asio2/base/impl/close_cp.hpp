/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_CLOSE_COMPONENT_HPP__
#define __ASIO2_CLOSE_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <future>
#include <utility>
#include <string_view>

#include <asio2/base/iopool.hpp>
#include <asio2/base/listener.hpp>

#include <asio2/base/impl/event_queue_cp.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t>
	class close_cp
	{
	public:
		using self = close_cp<derived_t, args_t>;

	public:
		/**
		 * @brief constructor
		 */
		close_cp() noexcept {}

		/**
		 * @brief destructor
		 */
		~close_cp() = default;

	protected:
		template<typename DeferEvent>
		inline void _do_close(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io_->running_in_this_thread());

			ASIO2_LOG_DEBUG("close_cp::_do_close enter: {} {}", ec.value(), ec.message());

			// Normally, do close function will be called in the io_context thread, but if some
			// exception occured, do close maybe called in the "catch(){ ... }", then this maybe
			// not in the io_context thread.
			// When the stop() is called for each session in the server's _post_stop, this maybe also 
			// not in the io_context thread.
			// If the session_ptr->stop() is called not in io_context thread, then this will be not in
			// the io_context thread.
			// If we don't ensure this function is called in the io_context thread, the session status
			// maybe stopping in the bind_recv callback.

			// use disp event to change the state to avoid this problem:
			// 1. when the event queue is not empty, call client stop, then the stop event will pushed
			//    into the event queue.
			// 2. in the io_context thread, the do close is called directly (without event queue),
			//    it will change the state to stopping.
			// 3. when the client _do stop is called, the state is not equal to stopped, it is equal to
			//    stopping.

			derive.disp_event(
			[&derive, ec, this_ptr = std::move(this_ptr), e = chain.move_event()]
			(event_queue_guard<derived_t> g) mutable
			{
				set_last_error(ec);

				defer_event chain(std::move(e), std::move(g));

				ASIO2_LOG_DEBUG("close_cp::_do_close leave: {} {} state={}",
					ec.value(), ec.message(), detail::to_string(derive.state_.load()));

				state_t expected = state_t::started;
				if (derive.state_.compare_exchange_strong(expected, state_t::stopping))
				{
					return derive._post_close(ec, std::move(this_ptr), expected, std::move(chain));
				}

				expected = state_t::starting;
				if (derive.state_.compare_exchange_strong(expected, state_t::stopping))
				{
					return derive._post_close(ec, std::move(this_ptr), expected, std::move(chain));
				}
			}, chain.move_guard());
		}

		template<typename DeferEvent, bool IsSession = args_t::is_session>
		inline typename std::enable_if_t<!IsSession, void>
		_post_close(const error_code& ec, std::shared_ptr<derived_t> this_ptr, state_t old_state, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io_->running_in_this_thread());

			ASIO2_LOG_DEBUG("close_cp::_post_close: {} {}", ec.value(), ec.message());

			// All pending sending events will be cancelled after enter the callback below.
			derive.disp_event(
			[&derive, ec, this_ptr = std::move(this_ptr), old_state, e = chain.move_event()]
			(event_queue_guard<derived_t> g) mutable
			{
				set_last_error(ec);

				defer_event chain(std::move(e), std::move(g));

				// When the connection is closed, should we set the state to stopping or stopped?
				// If the state is set to stopped, then the user wants to use client.is_stopped() to 
				// determine whether the client has stopped. The result is inaccurate because the client
				// has not stopped completely, such as the timer is still running.
				// If the state is set to stopping, the user will fail to reconnect the client using
				// client.start(...) in the bind_disconnect callback. because the client.start(...)
				// function will detects the value of state and the client.start(...) will only executed
				// if the state is stopped.

				state_t expected = state_t::stopping;
				if (derive.state_.compare_exchange_strong(expected, state_t::stopped))
				{
					if (old_state == state_t::started)
					{
						derive._fire_disconnect(this_ptr);
					}
				}
				else
				{
					ASIO2_ASSERT(false);
				}

				if (chain.empty())
				{
					derive._handle_close(ec, this_ptr, defer_event
					{
						[&derive, this_ptr](event_queue_guard<derived_t> g) mutable
						{
							// Use disp_event to ensure that reconnection will not executed until
							// all events are completed.
							derive.disp_event([&derive, this_ptr = std::move(this_ptr)]
							(event_queue_guard<derived_t> g) mutable
							{
								detail::ignore_unused(this_ptr, g);

								if (derive.reconnect_enable_)
									derive._wake_reconnect_timer();
								else
									derive._stop_reconnect_timer();
							}, std::move(g));
						}, chain.move_guard()
					});
				}
				else
				{
					derive._handle_close(ec, std::move(this_ptr), std::move(chain));
				}

				// can't call derive._do_stop() here, it will cause the auto reconnect invalid when
				// server is closed. so we use the chain to determine whether we should call 
				// derive._do_stop()
			}, chain.move_guard());
		}

		template<typename DeferEvent, bool IsSession = args_t::is_session>
		inline typename std::enable_if_t<IsSession, void>
		_post_close(const error_code& ec, std::shared_ptr<derived_t> this_ptr, state_t old_state, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io_->running_in_this_thread());

			ASIO2_LOG_DEBUG("close_cp::_post_close: {} {}", ec.value(), ec.message());

			// close the socket by post a event
			// asio don't allow operate the same socket in multi thread,if you close socket
			// in one thread and another thread is calling socket's async_... function,it 
			// will crash.so we must care for operate the socket. when need close the 
			// socket, we use the context to post a event, make sure the socket's close 
			// operation is in the same thread.

			// First ensure that all send and recv events are not executed again
			derive.disp_event(
			[&derive, ec, old_state, this_ptr = std::move(this_ptr), e = chain.move_event()]
			(event_queue_guard<derived_t> g) mutable
			{
				// All pending sending events will be cancelled when code run to here.

				// We must use the asio::post function to execute the task, otherwise :
				// when the server acceptor thread is same as this session thread,
				// when the server stop, will call sessions_.for_each -> session_ptr->stop() ->
				// derived().disp_event -> sessions_.erase => this can leads to a dead lock

				defer_event chain(std::move(e), std::move(g));

				asio::post(derive.io_->context(), make_allocator(derive.wallocator(),
				[&derive, ec, old_state, this_ptr = std::move(this_ptr), chain = std::move(chain)]
				() mutable
				{
					// Second ensure that this session has removed from the session map.
					derive.sessions_.erase(this_ptr,
					[&derive, ec, old_state, this_ptr, chain = std::move(chain)]
					(bool erased) mutable
					{
						set_last_error(ec);

						state_t expected = state_t::stopping;
						if (derive.state_.compare_exchange_strong(expected, state_t::stopped))
						{
							if (old_state == state_t::started && erased)
								derive._fire_disconnect(const_cast<std::shared_ptr<derived_t>&>(this_ptr));
						}
						else
						{
							ASIO2_ASSERT(false);
						}

						// Third we can stop this session and close this socket now.
						asio::dispatch(derive.io_->context(), make_allocator(derive.wallocator(),
						[&derive, ec, this_ptr = std::move(this_ptr), chain = std::move(chain)]
						() mutable
						{
							// call CRTP polymorphic stop
							derive._handle_close(ec, std::move(this_ptr), std::move(chain));
						}));
					});
				}));
			}, chain.move_guard());
		}

		template<typename DeferEvent>
		inline void _handle_close(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io_->running_in_this_thread());

			ASIO2_LOG_DEBUG("close_cp::_handle_close: {} {}", ec.value(), ec.message());

			derive._post_disconnect(ec, std::move(this_ptr), std::move(chain));
		}
	};
}

#endif // !__ASIO2_CLOSE_COMPONENT_HPP__
