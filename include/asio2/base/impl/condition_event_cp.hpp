/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_CONDITION_EVENT_COMPONENT_HPP__
#define __ASIO2_CONDITION_EVENT_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstddef>
#include <map>
#include <limits>
#include <memory>
#include <type_traits>
#include <chrono>
#include <mutex>

#include <asio2/base/iopool.hpp>

#include <asio2/base/detail/object.hpp>
#include <asio2/base/detail/allocator.hpp>

#include <asio2/util/spin_lock.hpp>

namespace asio2::detail
{
	template<class, class> class condition_event_cp;
	template<class, class> class rdc_call_cp_impl;
}

namespace asio2
{
	class [[deprecated("Replace async_event with condition_event")]] async_event : public detail::object_t<async_event>
	{
	};

	class condition_event : public detail::object_t<condition_event>
	{
		template<class, class> friend class asio2::detail::condition_event_cp;
		template<class, class> friend class asio2::detail::rdc_call_cp_impl;

	public:
		explicit condition_event(std::shared_ptr<detail::io_t>& iot)
			: event_timer_io_(iot)
		{
		}

		~condition_event() noexcept
		{
		}

	protected:
		template <typename WaitHandler>
		inline void async_wait(WaitHandler&& handler)
		{
			// when code run to here, it means that the io_context is started already,
			// then we set the flag to true, if the io_context is not started, we can't
			// set the flag to true, otherwise maybe cause crash.
			// eg : 
			// asio2::udp_cast udp;
			// auto ptr = udp.post_condition_event([](){});
			// std::thread([ptr]() mutable
			// {
			// 	std::this_thread::sleep_for(std::chrono::seconds(5));
			// 	ptr->notify();
			// }).detach();
			// // udp.start(...); // udp.start is not called,
			// then the udp is destroyed, after 5 seconds, the code will run to ptr->notify,
			// then it cause crash...

			// when code run to here, the io_context must be not destroy.

			std::shared_ptr<detail::io_t> io_ptr = this->event_timer_io_.lock();
			if (!io_ptr)
				return;

			this->event_timer_ = std::make_unique<asio::steady_timer>(io_ptr->context());

			io_ptr->timers().emplace(this->event_timer_.get());

			// Setting expiration to infinity will cause handlers to
			// wait on the timer until cancelled.
			this->event_timer_->expires_after((std::chrono::nanoseconds::max)());

			// bind is used to adapt the user provided handler to the 
			// timer's wait handler type requirement.
			// the "handler" has hold the derive_ptr already, so this lambda don't need hold it again.
			// this event_ptr (means self ptr) has holded by the map "condition_events_" already, 
			// so this lambda don't need hold self ptr again.
			this->event_timer_->async_wait(
			[this, handler = std::forward<WaitHandler>(handler)](const error_code& ec) mutable
			{
				ASIO2_ASSERT((!ec) || ec == asio::error::operation_aborted);

				detail::ignore_unused(ec);

				std::shared_ptr<detail::io_t> io_ptr = this->event_timer_io_.lock();
				if (!io_ptr)
					return;

				io_ptr->timers().erase(this->event_timer_.get());

				handler();
			});
		}

	public:
		/**
		 * @brief wakeup the waiting condition event.
		 */
		inline void notify()
		{
			// must use dispatch, otherwise if use called post_condition_event, and then called 
			// notify() immediately, the event will can't be notifyed.

			std::shared_ptr<detail::io_t> io_ptr = this->event_timer_io_.lock();
			if (!io_ptr)
				return;

			asio::dispatch(io_ptr->context(), [this, this_ptr = this->selfptr()]() mutable
			{
				detail::ignore_unused(this_ptr);

				detail::cancel_timer(*(this->event_timer_));
			});
		}

	protected:
		/// The io used for the timer. use weak_ptr, otherwise will cause circular reference.
		std::weak_ptr<detail::io_t>         event_timer_io_;

		/// Used to implementing asynchronous condition event
		std::unique_ptr<asio::steady_timer> event_timer_;
	};
}

namespace asio2::detail
{
	template<class derived_t, class args_t = void>
	class condition_event_cp
	{
	public:
		/**
		 * @brief constructor
		 */
		condition_event_cp() = default;

		/**
		 * @brief destructor
		 */
		~condition_event_cp() = default;

	public:
		/**
		 * @brief Post a asynchronous condition event to execution util the event is notifyed by user.
		 * Before you call event_ptr->notify(); the event will not execute forever.
		 * The function signature of the handler must be : void handler();
		 */
		template<typename Function>
		inline std::shared_ptr<condition_event> post_condition_event(Function&& fn)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			std::shared_ptr<condition_event> event_ptr = std::make_shared<condition_event>(derive.io_);

			asio::dispatch(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, this_ptr = derive.selfptr(), event_ptr, fn = std::forward<Function>(fn)]() mutable
			{
				condition_event* evt = event_ptr.get();

				this->condition_events_.emplace(evt, std::move(event_ptr));

				evt->async_wait([this, this_ptr = std::move(this_ptr), key = evt, fn = std::move(fn)]() mutable
				{
					fn();

					this->condition_events_.erase(key);
				});
			}));

			return event_ptr;
		}

		/**
		 * @brief Notify all condition events to execute.
		 */
		inline derived_t& notify_all_condition_events()
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// Make sure we run on the io_context thread
			asio::dispatch(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, this_ptr = derive.selfptr()]() mutable
			{
				for (auto&[key, event_ptr] : this->condition_events_)
				{
					detail::ignore_unused(this_ptr, key);

					event_ptr->notify();
				}
			}));

			return derive;
		}

	protected:
		/// Used to exit the condition event when component is ready to stop.
		/// if user don't notify the event to execute, the io_context will
		/// block forever, so we need notify the condition event when component
		/// is ready to stop.
		std::map<condition_event*, std::shared_ptr<condition_event>> condition_events_;
	};
}

#endif // !__ASIO2_CONDITION_EVENT_COMPONENT_HPP__
