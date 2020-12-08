/*
 * COPYRIGHT (C) 2017-2019, zhllxt
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

#include <asio2/base/selector.hpp>
#include <asio2/base/iopool.hpp>
#include <asio2/base/error.hpp>

#include <asio2/base/detail/object.hpp>
#include <asio2/base/detail/allocator.hpp>

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

		template <typename WaitHandler>
		void async_wait(WaitHandler&& handler)
		{
			// Setting expiration to infinity will cause handlers to
			// wait on the timer until cancelled.
			event_timer_.expires_after((std::chrono::nanoseconds::max)());

			// bind is used to adapt the user provided handler to the 
			// timer's wait handler type requirement.
			event_timer_.async_wait(asio::bind_executor(event_timer_io_.strand(),
				[handler = std::forward<WaitHandler>(handler),
				this_ptr = this->derived().selfptr()](const error_code&) mutable
			{
				handler();
			}));
		}

		template <typename WaitHandler, typename Allocator>
		void async_wait(WaitHandler&& handler, Allocator& allocator)
		{
			// Setting expiration to infinity will cause handlers to
			// wait on the timer until cancelled.
			event_timer_.expires_after((std::chrono::nanoseconds::max)());

			// bind is used to adapt the user provided handler to the 
			// timer's wait handler type requirement.
			event_timer_.async_wait(asio::bind_executor(event_timer_io_.strand(),
				detail::make_allocator(allocator, [handler = std::forward<WaitHandler>(handler),
					this_ptr = this->derived().selfptr()](const error_code&) mutable
			{
				handler();
			})));
		}

		/**
		 * @function : wakeup the waiting async_event.
		 * @return : The number of asynchronous events that were waked.
		 */
		inline std::size_t notify()
		{
			return event_timer_.cancel(detail::ec_ignore);
		}

	private:
		/// The io (include io_context and strand) used for the timer.
		detail::io_t     & event_timer_io_;

		/// Used to implementing asynchronous async_event
		asio::steady_timer event_timer_;
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
				[this, self = derive.selfptr(), event_ptr]() mutable
			{
				this->async_events_.emplace(event_ptr.get(), std::move(event_ptr));
			}));

			event_ptr->async_wait([this, self = derive.selfptr(), key = event_ptr.get(),
				f = std::forward<Function>(f)]() mutable
			{
				f();

				this->async_events_.erase(key);

			}, derive.wallocator());

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
				asio2::detail::ignore_unused(key);
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
