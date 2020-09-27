/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
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

#include <asio2/base/selector.hpp>
#include <asio2/base/iopool.hpp>
#include <asio2/base/error.hpp>
#include <asio2/base/listener.hpp>
#include <asio2/base/detail/condition_wrap.hpp>

#include <asio2/base/component/event_queue_cp.hpp>

#include <asio2/util/defer.hpp>

namespace asio2::detail
{
	template<class derived_t, class socket_t, bool isSession>
	class disconnect_cp
	{
	public:
		using self = disconnect_cp<derived_t, socket_t, isSession>;

	public:
		/**
		 * @constructor
		 */
		disconnect_cp() : derive(static_cast<derived_t&>(*this)) {}

		/**
		 * @destructor
		 */
		~disconnect_cp() = default;

	protected:
		template<bool IsSession = isSession>
		typename std::enable_if_t<!IsSession, void>
		inline _check_reconnect(const error_code& ec, std::shared_ptr<defer>& defer_task)
		{
			if (defer_task)
				return;

			// Enhance the reliability of auto reconnection
			defer_task = std::make_shared<defer>([this, ec, this_ptr = derive.selfptr()]() mutable
			{
				auto task = [this, ec, this_ptr = std::move(this_ptr)](event_guard<derived_t>&& g) mutable
				{
					detail::ignore_unused(this, ec, this_ptr, g);

					derive._wake_reconnect_timer();
				};

				// Use push_event to ensure that reconnection will not exxcuted until
				// all events are completed.
				derive.push_event([this, t = std::move(task)](event_guard<derived_t>&& g) mutable
				{
					auto task = [g = std::move(g), t = std::move(t)]() mutable
					{
						t(std::move(g));
					};
					derive.post(std::move(task));
					return true;
				});
			});
		}

		template<bool IsSession = isSession>
		typename std::enable_if_t<!IsSession, void>
		inline _do_disconnect(const error_code& ec, std::shared_ptr<defer> defer_task = {})
		{
			state_t expected = state_t::started;
			if (derive.state().compare_exchange_strong(expected, state_t::stopping))
			{
				derive._check_reconnect(ec, defer_task);

				return derive._post_disconnect(ec, derive.selfptr(), expected, std::move(defer_task));
			}

			expected = state_t::starting;
			if (derive.state().compare_exchange_strong(expected, state_t::stopping))
			{
				derive._check_reconnect(ec, defer_task);

				return derive._post_disconnect(ec, derive.selfptr(), expected, std::move(defer_task));
			}
		}

		template<bool IsSession = isSession>
		typename std::enable_if_t<IsSession, void>
		inline _do_disconnect(const error_code& ec, std::shared_ptr<defer> defer_task = {})
		{
			state_t expected = state_t::started;
			if (derive.state().compare_exchange_strong(expected, state_t::stopping))
				return derive._post_disconnect(ec, derive.selfptr(), expected, std::move(defer_task));

			expected = state_t::starting;
			if (derive.state().compare_exchange_strong(expected, state_t::stopping))
				return derive._post_disconnect(ec, derive.selfptr(), expected, std::move(defer_task));
		}

		template<bool IsSession = isSession>
		typename std::enable_if_t<!IsSession, void>
		inline _post_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr,
			state_t old_state, std::shared_ptr<defer> defer_task = {})
		{
			auto task = [this, ec, this_ptr, old_state, defer_task = std::move(defer_task)]
			(event_guard<derived_t>&& g) mutable
			{
				set_last_error(ec);

				state_t expected = state_t::stopping;
				if (derive.state().compare_exchange_strong(expected, state_t::stopped))
				{
					if (old_state == state_t::started)
					{
						derive._fire_disconnect(this_ptr, ec);
					}

					derive._handle_disconnect(ec, std::move(this_ptr));
				}
				else
				{
					ASIO2_ASSERT(false);
				}

				// can't call derive._do_stop() here, it will cause the auto reconnect invalid when
				// server is closed. so we use the defer_task to determine whether we should call 
				// derive._do_stop()
			};

			// All pending sending events will be cancelled after enter the send strand below.
			derive.push_event([this, t = std::move(task)](event_guard<derived_t>&& g) mutable
			{
				auto task = [g = std::move(g), t = std::move(t)]() mutable
				{
					t(std::move(g));
				};
				derive.post(std::move(task));
				return true;
			});
		}

		template<bool IsSession = isSession>
		typename std::enable_if_t<IsSession, void>
		inline _post_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr,
			state_t old_state, std::shared_ptr<defer> defer_task = {})
		{
			// close the socket by post a event
			// asio don't allow operate the same socket in multi thread,if you close socket
			// in one thread and another thread is calling socket's async_... function,it 
			// will crash.so we must care for operate the socket. when need close the 
			// socket ,we use the strand to post a event,make sure the socket's close 
			// operation is in the same thread.

			// First ensure that all send and recv events are not executed again
			auto task = [this, this_ptr = std::move(this_ptr), defer_task = std::move(defer_task),
				ec, old_state](event_guard<derived_t>&& g) mutable
			{
				// All pending sending events will be cancelled when code run to here.

				// Second ensure that this session has removed from the session map.
				derive.sessions_.erase(this_ptr, [this, this_ptr, g = std::move(g),
					defer_task = std::move(defer_task), ec, old_state](bool) mutable
				{
					set_last_error(ec);

					state_t expected = state_t::stopping;
					if (derive.state().compare_exchange_strong(expected, state_t::stopped))
					{
						if (old_state == state_t::started)
							derive._fire_disconnect(const_cast<std::shared_ptr<derived_t>&>(this_ptr));
					}
					else
					{
						ASIO2_ASSERT(false);
					}

					// Third we can stop this session and close this socket now.
					derive.post([this, self_ptr = std::move(this_ptr), g = std::move(g),
						defer_task = std::move(defer_task), ec]() mutable
					{
						// call CRTP polymorphic stop
						derive._handle_disconnect(ec, std::move(self_ptr));
					});
				});
			};

			derive.push_event([this, t = std::move(task)](event_guard<derived_t>&& g) mutable
			{
				auto task = [g = std::move(g), t = std::move(t)]() mutable
				{
					t(std::move(g));
				};
				// We must use the asio::post function to execute the task, otherwise :
				// when the server acceptor thread is same as this session thread,
				// when the server stop, will call sessions_.foreach -> session_ptr->stop() ->
				// derived().push_event -> sessions_.erase => this can leads to a dead lock
				derive.post(std::move(task));
				return true;
			});
		}

		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			detail::ignore::unused(ec, this_ptr);
		}

	protected:
		derived_t                     & derive;
	};
}

#endif // !__ASIO2_DISCONNECT_COMPONENT_HPP__
