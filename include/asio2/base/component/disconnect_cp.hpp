/*
 * COPYRIGHT (C) 2017-2021, zhllxt
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

#include <asio2/3rd/asio.hpp>
#include <asio2/base/iopool.hpp>
#include <asio2/base/error.hpp>
#include <asio2/base/listener.hpp>
#include <asio2/base/detail/condition_wrap.hpp>

#include <asio2/base/component/event_queue_cp.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t>
	class disconnect_cp
	{
	public:
		using self = disconnect_cp<derived_t, args_t>;

	public:
		/**
		 * @constructor
		 */
		disconnect_cp() noexcept {}

		/**
		 * @destructor
		 */
		~disconnect_cp() = default;

	protected:
		template<typename DeferEvent = defer_event<>, bool IsSession = args_t::is_session>
		typename std::enable_if_t<!IsSession, void>
		inline _do_disconnect(const error_code& ec, DeferEvent&& chain = defer_event{ nullptr })
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io().strand().running_in_this_thread());

			state_t expected = state_t::started;
			if (derive.state().compare_exchange_strong(expected, state_t::stopping))
			{
				ASIO2_LOG(spdlog::level::debug, "enter _do_disconnect : {}",
					magic_enum::enum_name(state_t::started));

				return derive._check_reconnect(ec, expected, std::forward<DeferEvent>(chain));
			}

			expected = state_t::starting;
			if (derive.state().compare_exchange_strong(expected, state_t::stopping))
			{
				ASIO2_LOG(spdlog::level::debug, "enter _do_disconnect : {}",
					magic_enum::enum_name(state_t::starting));

				return derive._check_reconnect(ec, expected, std::forward<DeferEvent>(chain));
			}

			ASIO2_LOG(spdlog::level::debug, "enter _do_disconnect : {}",
				magic_enum::enum_name(derive.state_.load()));
		}

		template<typename DeferEvent, bool IsSession = args_t::is_session>
		typename std::enable_if_t<!IsSession, void>
		inline _check_reconnect(const error_code& ec, state_t expected, DeferEvent&& chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if (chain.empty())
			{
				derive._post_disconnect(ec, derive.selfptr(), expected, defer_event
				{
					[&derive]() mutable
					{
						ASIO2_LOG(spdlog::level::debug, "post _wake_reconnect_timer : {}",
							magic_enum::enum_name(derive.state_.load()));

						// Use push_event to ensure that reconnection will not executed until
						// all events are completed.
						derive.push_event([&derive, this_ptr = derive.selfptr()]
						(event_queue_guard<derived_t>&& g) mutable
						{
							detail::ignore_unused(this_ptr, g);

							ASIO2_LOG(spdlog::level::debug, "call _wake_reconnect_timer : {}",
								magic_enum::enum_name(derive.state_.load()));

							derive._wake_reconnect_timer();
						});
					}
				});
			}
			else
			{
				derive._post_disconnect(ec, derive.selfptr(), expected, std::forward<DeferEvent>(chain));
			}
		}

		template<typename DeferEvent = defer_event<>, bool IsSession = args_t::is_session>
		typename std::enable_if_t<!IsSession, void>
		inline _post_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr,
			state_t old_state, DeferEvent&& chain = defer_event{ nullptr })
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_LOG(spdlog::level::debug, "post disconnect : {}",
				magic_enum::enum_name(derive.state_.load()));

			// All pending sending events will be cancelled after enter the send strand below.
			derive.push_event(
			[&derive, ec, this_ptr = std::move(this_ptr), old_state, chain = std::forward<DeferEvent>(chain)]
			(event_queue_guard<derived_t>&& g) mutable
			{
				detail::ignore_unused(g);

				set_last_error(ec);

				ASIO2_LOG(spdlog::level::debug, "exec disconnect : {}",
					magic_enum::enum_name(derive.state_.load()));

				// When the connection is disconnected, should we set the state to stopping or stopped?
				// If the state is set to stopped, then the user wants to use client.is_stopped() to 
				// determine whether the client has stopped. The result is inaccurate because the client
				// has not stopped completely, such as the timer is still running.
				// If the state is set to stopping, the user will fail to reconnect the client using
				// client.start(...) in the bind_disconnect callback. because the client.start(...)
				// function will detects the value of state and the client.start(...) will only executed
				// if the state is stopped.
				state_t expected = state_t::stopping;
				if (derive.state().compare_exchange_strong(expected, state_t::stopping))
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

				derive._handle_disconnect(ec, std::move(this_ptr));

				// can't call derive._do_stop() here, it will cause the auto reconnect invalid when
				// server is closed. so we use the chain to determine whether we should call 
				// derive._do_stop()
			});
		}

		template<typename DeferEvent = defer_event<>, bool IsSession = args_t::is_session>
		typename std::enable_if_t<IsSession, void>
		inline _do_disconnect(const error_code& ec, DeferEvent&& chain = defer_event{ nullptr })
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			//ASIO2_ASSERT(derive.io().strand().running_in_this_thread());

			state_t expected = state_t::started;
			if (derive.state().compare_exchange_strong(expected, state_t::stopping))
				return derive._post_disconnect(ec, derive.selfptr(), expected, std::forward<DeferEvent>(chain));

			expected = state_t::starting;
			if (derive.state().compare_exchange_strong(expected, state_t::stopping))
				return derive._post_disconnect(ec, derive.selfptr(), expected, std::forward<DeferEvent>(chain));
		}

		template<typename DeferEvent = defer_event<>, bool IsSession = args_t::is_session>
		typename std::enable_if_t<IsSession, void>
		inline _post_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr,
			state_t old_state, DeferEvent&& chain = defer_event{ nullptr })
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// close the socket by post a event
			// asio don't allow operate the same socket in multi thread,if you close socket
			// in one thread and another thread is calling socket's async_... function,it 
			// will crash.so we must care for operate the socket. when need close the 
			// socket ,we use the strand to post a event,make sure the socket's close 
			// operation is in the same thread.

			// First ensure that all send and recv events are not executed again
			derive.push_event(
			[&derive, ec, old_state, this_ptr = std::move(this_ptr), chain = std::forward<DeferEvent>(chain)]
			(event_queue_guard<derived_t>&& g) mutable
			{
				// All pending sending events will be cancelled when code run to here.

				// We must use the asio::post function to execute the task, otherwise :
				// when the server acceptor thread is same as this session thread,
				// when the server stop, will call sessions_.for_each -> session_ptr->stop() ->
				// derived().push_event -> sessions_.erase => this can leads to a dead lock

				asio::post(derive.io().strand(), make_allocator(derive.wallocator(),
				[&derive, ec, old_state, this_ptr = std::move(this_ptr), g = std::move(g), chain = std::move(chain)]
				() mutable
				{
					// Second ensure that this session has removed from the session map.
					derive.sessions_.erase(this_ptr,
					[&derive, ec, old_state, this_ptr, g = std::move(g), chain = std::move(chain)]
					(bool erased) mutable
					{
						set_last_error(ec);

						state_t expected = state_t::stopping;
						if (derive.state().compare_exchange_strong(expected, state_t::stopping))
						{
							if (old_state == state_t::started && erased)
								derive._fire_disconnect(const_cast<std::shared_ptr<derived_t>&>(this_ptr));
						}
						else
						{
							ASIO2_ASSERT(false);
						}

						// Third we can stop this session and close this socket now.
						asio::dispatch(derive.io().strand(), make_allocator(derive.wallocator(),
						[&derive, ec, this_ptr = std::move(this_ptr), g = std::move(g), chain = std::move(chain)]
						() mutable
						{
							// call CRTP polymorphic stop
							derive._handle_disconnect(ec, std::move(this_ptr));
						}));
					});
				}));
			});
		}

		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			detail::ignore_unused(ec, this_ptr);
		}

	protected:
	};
}

#endif // !__ASIO2_DISCONNECT_COMPONENT_HPP__
