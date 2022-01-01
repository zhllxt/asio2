/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_ASYNC_EVENT_COMPONENT_HPP__
#define __ASIO2_ASYNC_EVENT_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstddef>
#include <map>
#include <limits>
#include <memory>
#include <type_traits>
#include <chrono>

#include <asio2/3rd/asio.hpp>
#include <asio2/base/iopool.hpp>
#include <asio2/base/error.hpp>

#include <asio2/base/detail/object.hpp>
#include <asio2/base/detail/allocator.hpp>

namespace asio2
{
	class async_event : public detail::object_t<async_event>
	{
	public:
		explicit async_event(detail::io_t& io, std::shared_ptr<void> derive_ptr)
			: event_timer_io_(io)
			, derive_ptr_(std::move(derive_ptr))
		{
			event_timer_ = std::make_shared<asio::steady_timer>(io.context());
		}

		~async_event()
		{
		}

		template <typename WaitHandler>
		inline void async_wait(WaitHandler&& handler)
		{
			detail::io_t& io = this->event_timer_io_;
			std::shared_ptr<asio::steady_timer> timer = this->event_timer_;

			asio::dispatch(io.strand(),
			[&io, handler = std::forward<WaitHandler>(handler), timer = std::move(timer)]
			() mutable
			{
				io.timers().emplace(timer.get());

				// Setting expiration to infinity will cause handlers to
				// wait on the timer until cancelled.
				timer->expires_after((std::chrono::nanoseconds::max)());

				// bind is used to adapt the user provided handler to the 
				// timer's wait handler type requirement.
				// the "handler" has hold the session_ptr already, so this lambda don't need hold it again.
				timer->async_wait(asio::bind_executor(io.strand(),
				[&io, timer, handler = std::move(handler)](const error_code&) mutable
				{
					io.timers().erase(timer.get());
					handler();
				}));
			});
		}

		template <typename WaitHandler, typename Allocator>
		inline void async_wait(WaitHandler&& handler, Allocator& allocator)
		{
			detail::io_t& io = this->event_timer_io_;
			std::shared_ptr<asio::steady_timer> timer = this->event_timer_;

			asio::dispatch(io.strand(),
			[&io, handler = std::forward<WaitHandler>(handler), allocator, timer = std::move(timer)]
			() mutable
			{
				io.timers().emplace(timer.get());

				// Setting expiration to infinity will cause handlers to
				// wait on the timer until cancelled.
				timer->expires_after((std::chrono::nanoseconds::max)());

				// bind is used to adapt the user provided handler to the 
				// timer's wait handler type requirement.
				// the "handler" has hold the session_ptr already, so this lambda don't need hold it again.
				timer->async_wait(asio::bind_executor(io.strand(),
					detail::make_allocator(allocator,
				[&io, timer, handler = std::move(handler)](const error_code&) mutable
				{
					io.timers().erase(timer.get());
					handler();
				})));
			});
		}

		/**
		 * @function : wakeup the waiting async_event.
		 */
		inline void notify()
		{
			// why use "std::shared_ptr<asio::steady_timer>", why don't use asio::steady_timer?
			// if user passed the "event_ptr" to another thread after post_event, and has called
			// event_ptr->notify before "another thread", when run to "another thread", user call
			// event_ptr->notify and the thread is exited immediately, and this will cause the
			// event_ptr destroyed immediately, when code run to the inner of asio::dispatch, when 
			// call the timer.cancel, it will crashed, beacuse the timer is destroyed already.
			std::shared_ptr<asio::steady_timer> timer = this->event_timer_;

			// must use dispatch, otherwise if use called post_event, and then called 
			// notify() immediately, the event will can't be notifyed.
			asio::dispatch(this->event_timer_io_.strand(), [timer = std::move(timer)]() mutable
			{
				error_code ec_ignore{};
				timer->cancel(ec_ignore);
			});
		}

	protected:
		/// The io (include io_context and strand) used for the timer.
		detail::io_t                      & event_timer_io_;

		/// Used to implementing asynchronous async_event
		std::shared_ptr<asio::steady_timer> event_timer_;

		/// hold the derive shared_ptr, otherwise when user call notify in another
		/// thread after a period of time, the event_timer_io_ maybe destroyed already.
		std::shared_ptr<void>               derive_ptr_;
	};
}

namespace asio2::detail
{
	template<class derived_t, class args_t = void>
	class async_event_cp
	{
	public:
		/**
		 * @constructor
		 */
		async_event_cp() = default;

		/**
		 * @destructor
		 */
		~async_event_cp() = default;

	public:
		/**
		 * @function : Post a asynchronous event to execution util the event is notifyed by user.
		 * Before you call event_ptr->notify(); the event will not execute forever.
		 * The function signature of the handler must be : void handler();
		 */
		template<typename Function>
		inline std::shared_ptr<async_event> post_event(Function&& f)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// note : need test.
			// has't check where the server or client is stopped, if the server is stopping, but the 
			// iopool's wait_for_io_context_stopped() has't compelete and just at sleep, then user
			// call post_event but don't call notify, it maybe cause the wait_for_io_context_stopped()
			// can't compelete forever, and the server.stop or client.stop never compeleted.

			std::shared_ptr<async_event> event_ptr = std::make_shared<async_event>(
				derive.io(), derive.selfptr());

			asio::dispatch(derive.io().strand(), make_allocator(derive.wallocator(),
			[this, this_ptr = derive.selfptr(), event_ptr, f = std::forward<Function>(f)]() mutable
			{
				async_event* evt = event_ptr.get();

				this->async_events_.emplace(evt, std::move(event_ptr));

				evt->async_wait([this, this_ptr = std::move(this_ptr), key = evt, f = std::move(f)]() mutable
				{
					f();

					this->async_events_.erase(key);
				});
			}));

			return event_ptr;
		}

		/**
		 * @function : Notify all async_events to execute.
		 */
		inline derived_t& notify_all_events()
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// Make sure we run on the strand
			if (!derive.io().strand().running_in_this_thread())
			{
				asio::post(derive.io().strand(), make_allocator(derive.wallocator(),
				[this, this_ptr = derive.selfptr()]() mutable
				{
					this->notify_all_events();
				}));
				return (derive);
			}

			for (auto&[key, event_ptr] : this->async_events_)
			{
				detail::ignore_unused(key);
				event_ptr->notify();
			}

			return (derive);
		}

	protected:
		/// Used to exit the async_event when component is ready to stop.
		/// if user don't notify the event to execute, the io_context will
		/// block forever, so we need notify the async_event when component
		/// is ready to stop.
		std::map<async_event*, std::shared_ptr<async_event>> async_events_;
	};
}

#endif // !__ASIO2_ASYNC_EVENT_COMPONENT_HPP__
