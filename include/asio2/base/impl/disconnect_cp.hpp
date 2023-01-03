/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_DISCONNECT_COMPONENT_HPP__
#define __ASIO2_DISCONNECT_COMPONENT_HPP__

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
	class disconnect_cp
	{
	public:
		using self = disconnect_cp<derived_t, args_t>;

	public:
		/**
		 * @brief constructor
		 */
		disconnect_cp() noexcept {}

		/**
		 * @brief destructor
		 */
		~disconnect_cp() = default;

	protected:
		template<typename DeferEvent = defer_event<void, derived_t>, bool IsSession = args_t::is_session>
		typename std::enable_if_t<!IsSession, void>
		inline _do_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr,
			DeferEvent chain = defer_event<void, derived_t>{})
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io().running_in_this_thread());

			set_last_error(ec);

			state_t expected = state_t::started;
			if (derive.state().compare_exchange_strong(expected, state_t::stopping))
			{
				return derive._check_reconnect(ec, std::move(this_ptr), expected, std::move(chain));
			}

			expected = state_t::starting;
			if (derive.state().compare_exchange_strong(expected, state_t::stopping))
			{
				return derive._check_reconnect(ec, std::move(this_ptr), expected, std::move(chain));
			}
		}

		template<typename DeferEvent, bool IsSession = args_t::is_session>
		inline typename std::enable_if_t<!IsSession, void>
		_check_reconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr, state_t expected, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if (chain.empty())
			{
				derive._post_disconnect(ec, std::move(this_ptr), expected, defer_event
				{
					[&derive, this_ptr = derive.selfptr()](event_queue_guard<derived_t> g) mutable
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
				derive._post_disconnect(ec, std::move(this_ptr), expected, std::move(chain));
			}
		}

		template<typename DeferEvent, bool IsSession = args_t::is_session>
		inline typename std::enable_if_t<!IsSession, void>
		_post_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr, state_t old_state, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			derive._make_disconnect_timer(this_ptr);

			// All pending sending events will be cancelled after enter the callback below.
			derive.disp_event(
			[&derive, ec, this_ptr = std::move(this_ptr), old_state, e = chain.move_event()]
			(event_queue_guard<derived_t> g) mutable
			{
				set_last_error(ec);

				defer_event chain(std::move(e), std::move(g));

				// When the connection is disconnected, should we set the state to stopping or stopped?
				// If the state is set to stopped, then the user wants to use client.is_stopped() to 
				// determine whether the client has stopped. The result is inaccurate because the client
				// has not stopped completely, such as the timer is still running.
				// If the state is set to stopping, the user will fail to reconnect the client using
				// client.start(...) in the bind_disconnect callback. because the client.start(...)
				// function will detects the value of state and the client.start(...) will only executed
				// if the state is stopped.

				state_t expected = state_t::stopping;
				if (derive.state().compare_exchange_strong(expected, state_t::stopped))
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

				derive._stop_disconnect_timer();

				derive._handle_disconnect(ec, std::move(this_ptr), std::move(chain));

				// can't call derive._do_stop() here, it will cause the auto reconnect invalid when
				// server is closed. so we use the chain to determine whether we should call 
				// derive._do_stop()
			}, chain.move_guard());
		}

		template<typename DeferEvent = defer_event<void, derived_t>, bool IsSession = args_t::is_session>
		typename std::enable_if_t<IsSession, void>
		inline _do_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr,
			DeferEvent chain = defer_event<void, derived_t>{})
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			set_last_error(ec);

			// Normally, _do_disconnect function will be called in the io_context thread, but if some
			// exception occured, _do_disconnect maybe called in the "catch(){ ... }", then this maybe
			// not in the io_context thread.
			// When the stop() is called for each session in the server's _post_stop, this maybe also 
			// not in the io_context thread.
			// If the session_ptr->stop() is called not in io_context thread, then this will be not in
			// the io_context thread.
			// If we don't ensure this function is called in the io_context thread, the session status
			// maybe stopping in the bind_recv callback.

			//ASIO2_ASSERT(derive.io().running_in_this_thread());

			asio::dispatch(derive.io().context(), make_allocator(derive.wallocator(),
			[&derive, ec, this_ptr = std::move(this_ptr), chain = std::move(chain)]() mutable
			{
				ASIO2_ASSERT(derive.io().running_in_this_thread());

				state_t expected = state_t::started;
				if (derive.state().compare_exchange_strong(expected, state_t::stopping))
				{
					return derive._post_disconnect(ec, std::move(this_ptr), expected, std::move(chain));
				}

				expected = state_t::starting;
				if (derive.state().compare_exchange_strong(expected, state_t::stopping))
				{
					return derive._post_disconnect(ec, std::move(this_ptr), expected, std::move(chain));
				}
			}));
		}

		template<typename DeferEvent, bool IsSession = args_t::is_session>
		inline typename std::enable_if_t<IsSession, void>
		_post_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr, state_t old_state, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io().running_in_this_thread());

			derive._make_disconnect_timer(this_ptr);

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

				asio::post(derive.io().context(), make_allocator(derive.wallocator(),
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
						if (derive.state().compare_exchange_strong(expected, state_t::stopped))
						{
							if (old_state == state_t::started && erased)
								derive._fire_disconnect(const_cast<std::shared_ptr<derived_t>&>(this_ptr));
						}
						else
						{
							ASIO2_ASSERT(false);
						}

						// Third we can stop this session and close this socket now.
						asio::dispatch(derive.io().context(), make_allocator(derive.wallocator(),
						[&derive, ec, this_ptr = std::move(this_ptr), chain = std::move(chain)]
						() mutable
						{
							derive._stop_disconnect_timer();

							// call CRTP polymorphic stop
							derive._handle_disconnect(ec, std::move(this_ptr), std::move(chain));
						}));
					});
				}));
			}, chain.move_guard());
		}

		//template<typename DeferEvent>
		//inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		//{
		//	detail::ignore_unused(ec, this_ptr, chain);
		//}

	protected:
		inline void _make_disconnect_timer(std::shared_ptr<derived_t> this_ptr)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			asio::dispatch(derive.io().context(), make_allocator(derive.wallocator(),
			[this, this_ptr = std::move(this_ptr)]() mutable
			{
				derived_t& derive = static_cast<derived_t&>(*this);

				if (this->disconnect_timer_)
				{
					this->disconnect_timer_->cancel();
				}

				this->disconnect_timer_ = std::make_shared<safe_timer>(derive.io().context());

				derive._post_disconnect_timer(std::move(this_ptr), this->disconnect_timer_);
			}));
		}

		inline void _post_disconnect_timer(
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<safe_timer> timer_ptr)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// a new timer is maked, this is the prev timer, so return directly.
			if (timer_ptr.get() != this->disconnect_timer_.get())
				return;

			safe_timer* ptimer = timer_ptr.get();

			ptimer->timer.expires_after(std::chrono::milliseconds(10));
			ptimer->timer.async_wait(
			[&derive, this_ptr = std::move(this_ptr), timer_ptr = std::move(timer_ptr)]
			(const error_code & ec) mutable
			{
				derive._handle_disconnect_timer(ec, std::move(this_ptr), std::move(timer_ptr));
			});
		}

		inline void _handle_disconnect_timer(
			const error_code & ec, std::shared_ptr<derived_t> this_ptr, std::shared_ptr<safe_timer> timer_ptr)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT((!ec) || ec == asio::error::operation_aborted);

			// a new timer is maked, this is the prev timer, so return directly.
			if (timer_ptr.get() != this->disconnect_timer_.get())
				return;

			// member variable timer should't be empty
			if (!this->disconnect_timer_)
			{
				ASIO2_ASSERT(false);
				return;
			}

			// ec maybe zero when the timer canceled is true.
			if (ec == asio::error::operation_aborted || timer_ptr->canceled.test_and_set())
			{
				this->disconnect_timer_.reset();
				return;
			}

			timer_ptr->canceled.clear();

			if (derive.socket().is_open())
			{
				error_code ec_ignore{};

				derive.socket().shutdown(asio::socket_base::shutdown_both, ec_ignore);
				derive.socket().cancel(ec_ignore);
			}

			derive._post_disconnect_timer(std::move(this_ptr), std::move(timer_ptr));
		}

		inline void _stop_disconnect_timer()
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			derive.dispatch([this]() mutable
			{
				if (this->disconnect_timer_)
				{
					this->disconnect_timer_->cancel();
				}
			});
		}

	protected:
		/// If running in the ubuntu and gcc 9.4.0, when client is closed by server, then the
		/// callback of client's asio::async_write maybe never invoked, this will cause the
		/// next event never be called in the event queue, then cuase the client can't be exit
		/// forever. after test, we need called the socket.cancel multi times, so we used a 
		/// timer to do it.
		/// beacuse the disconnect timer is used only when disconnect, so we use a pointer
		/// to reduce memory space occupied when running
		std::shared_ptr<safe_timer> disconnect_timer_;
	};
}

#endif // !__ASIO2_DISCONNECT_COMPONENT_HPP__
