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

#include <asio2/external/asio.hpp>
#include <asio2/base/iopool.hpp>
#include <asio2/base/error.hpp>

#include <asio2/base/detail/object.hpp>
#include <asio2/base/detail/allocator.hpp>

#include <asio2/util/spin_lock.hpp>

namespace asio2
{
	class async_event : public detail::object_t<async_event>
	{
	public:
		explicit async_event(detail::io_t& io)
			: event_timer_io_(io)
			, event_timer_(io.context())
		{
		}

		~async_event() noexcept
		{
		}

		template <typename WaitHandler>
		inline void async_wait(WaitHandler&& handler)
		{
			asio::dispatch(this->event_timer_io_.strand(),
			[this, handler = std::forward<WaitHandler>(handler)]() mutable
			{
				this->event_timer_io_.timers().emplace(&event_timer_);

				// Setting expiration to infinity will cause handlers to
				// wait on the timer until cancelled.
				this->event_timer_.expires_after((std::chrono::nanoseconds::max)());

				// bind is used to adapt the user provided handler to the 
				// timer's wait handler type requirement.
				// the "handler" has hold the derive_ptr already, so this lambda don't need hold it again.
				// this event_ptr (means selfptr) has holded by the map "async_events_" already, 
				// so this lambda don't need hold selfptr again.
				this->event_timer_.async_wait(asio::bind_executor(this->event_timer_io_.strand(),
				[this, handler = std::move(handler)](const error_code&) mutable
				{
					this->event_timer_io_.timers().erase(&event_timer_);

					handler();
				}));
			});
		}

		/**
		 * @function : wakeup the waiting async_event.
		 * must ensure that the io_context is not destroyed, otherwise this will cause crash.
		 * 
		 * when the event_ptr is passed into some thread, when the thread executing,
		 * and the event_ptr->notify is called, at this time, the io_context maybe
		 * destroyed already, this will crash.
		 * eg : 
		 * asio2::udp_cast udp;
		 * auto ptr = udp.post_event([](){});
		 * std::thread([ptr]() mutable
		 * {
		 * 	std::this_thread::sleep_for(std::chrono::seconds(5));
		 * 	ptr->notify();
		 * }).detach();
		 * udp.start(...); // or don't call udp.start
		 * 
		 * asio2::iopool iopool(4);
		 * iopool.start(); // or don't call iopool.start
		 * asio2::udp_cast udp(iopool.get(0));
		 * auto ptr = udp.post_event([](){});
		 * std::thread([ptr]() mutable
		 * {
		 * 	std::this_thread::sleep_for(std::chrono::seconds(5));
		 * 	ptr->notify();
		 * }).detach();
		 * udp.start(...); // or don't call udp.start
		 * iopool.stop();
		 */
		inline void notify()
		{
			// must use dispatch, otherwise if use called post_event, and then called 
			// notify() immediately, the event will can't be notifyed.

			asio::dispatch(this->event_timer_io_.strand(), [this, this_ptr = this->selfptr()]() mutable
			{
				detail::ignore_unused(this_ptr);

				error_code ec_ignore{};
				this->event_timer_.cancel(ec_ignore);
			});
		}

	protected:
		/// The io (include io_context and strand) used for the timer.
		detail::io_t                      & event_timer_io_;

		/// Used to implementing asynchronous async_event
		asio::steady_timer                  event_timer_;
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

			std::shared_ptr<async_event> event_ptr = std::make_shared<async_event>(derive.io());

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
