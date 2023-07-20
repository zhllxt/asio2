/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_POST_COMPONENT_HPP__
#define __ASIO2_POST_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <set>
#include <future>
#include <functional>

#include <asio2/base/iopool.hpp>
#include <asio2/base/detail/allocator.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t = void>
	class post_cp
	{
	public:
		/**
		 * @brief constructor
		 */
		post_cp() {}

		/**
		 * @brief destructor
		 */
		~post_cp() = default;

	public:
		/**
		 * @brief Submits a completion token or function object for execution.
		 * This function submits an object for execution using the object's associated
		 * executor. The function object is queued for execution, and is never called
		 * from the current thread prior to returning from <tt>post()</tt>.
		 */
		template<typename Function>
		inline derived_t & post(Function&& fn)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// if use call post, but the user callback "fn" has't hold the session_ptr,
			// it maybe cause crash, so we need hold the session_ptr again at here.
			// if the session_ptr is already destroyed, the selfptr() will cause crash.
			asio::post(derive.io_->context(), make_allocator(derive.wallocator(),
			[p = derive.selfptr(), fn = std::forward<Function>(fn)]() mutable
			{
				detail::ignore_unused(p);

				fn();
			}));

			return derive;
		}

		/**
		 * @brief Submits a completion token or function object for execution after specified delay time.
		 * This function submits an object for execution using the object's associated
		 * executor. The function object is queued for execution, and is never called
		 * from the current thread prior to returning from <tt>post()</tt>.
		 */
		template<typename Function, typename Rep, typename Period>
		inline derived_t & post(Function&& fn, std::chrono::duration<Rep, Period> delay)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// note : need test.
			// has't check where the server or client is stopped, if the server is stopping, but the 
			// iopool's wait_for_io_context_stopped() has't compelete and just at sleep, then user
			// call post but don't call stop_all_timed_tasks, it maybe cause the
			// wait_for_io_context_stopped() can't compelete forever,and the server.stop or client.stop
			// never compeleted.

			if (delay > std::chrono::duration_cast<
				std::chrono::duration<Rep, Period>>((asio::steady_timer::duration::max)()))
				delay = std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(
					(asio::steady_timer::duration::max)());

			asio::post(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, &derive, p = derive.selfptr(), fn = std::forward<Function>(fn), delay]() mutable
			{
				std::unique_ptr<asio::steady_timer> timer = std::make_unique<
					asio::steady_timer>(derive.io_->context());

				this->timed_tasks_.emplace(timer.get());

				derive.io_->timers().emplace(timer.get());

				timer->expires_after(delay);
				timer->async_wait(
				[this, &derive, p = std::move(p), timer = std::move(timer), fn = std::move(fn)]
				(const error_code& ec) mutable
				{
					ASIO2_ASSERT((!ec) || ec == asio::error::operation_aborted);

					derive.io_->timers().erase(timer.get());
					detail::ignore_unused(p);
					set_last_error(ec);
				#if defined(ASIO2_ENABLE_TIMER_CALLBACK_WHEN_ERROR)
					fn();
				#else
					if (!ec)
					{
						fn();
					}
				#endif
					this->timed_tasks_.erase(timer.get());
				});
			}));

			return derive;
		}

		/**
		 * @brief Submits a completion token or function object for execution.
		 * This function submits an object for execution using the object's associated
		 * executor. The function object is queued for execution, and is never called
		 * from the current thread prior to returning from <tt>post()</tt>.
		 * note : Never call future's waiting function(eg:wait,get) in a communication(eg:on_recv)
		 * thread, it will cause dead lock;
		 */
		template<typename Function, typename Allocator>
		inline auto post(Function&& fn, asio::use_future_t<Allocator>) ->
			std::future<std::invoke_result_t<Function>>
		{
			using return_type = std::invoke_result_t<Function>;

			derived_t& derive = static_cast<derived_t&>(*this);

			std::packaged_task<return_type()> task(std::forward<Function>(fn));

			std::future<return_type> future = task.get_future();

			asio::post(derive.io_->context(), make_allocator(derive.wallocator(),
			[p = derive.selfptr(), t = std::move(task)]() mutable
			{
				detail::ignore_unused(p);

				t();
			}));

			return future;
		}

		/**
		 * @brief Submits a completion token or function object for execution after specified delay time.
		 * This function submits an object for execution using the object's associated
		 * executor. The function object is queued for execution, and is never called
		 * from the current thread prior to returning from <tt>post()</tt>.
		 * note : Never call future's waiting function(eg:wait,get) in a communication(eg:on_recv)
		 * thread, it will cause dead lock;
		 */
		template<typename Function, typename Rep, typename Period, typename Allocator>
		inline auto post(Function&& fn, std::chrono::duration<Rep, Period> delay, asio::use_future_t<Allocator>)
			-> std::future<std::invoke_result_t<Function>>
		{
			using return_type = std::invoke_result_t<Function>;

			derived_t& derive = static_cast<derived_t&>(*this);

			if (delay > std::chrono::duration_cast<
				std::chrono::duration<Rep, Period>>((asio::steady_timer::duration::max)()))
				delay = std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(
					(asio::steady_timer::duration::max)());

			std::packaged_task<return_type()> task(std::forward<Function>(fn));

			std::future<return_type> future = task.get_future();

			asio::post(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, &derive, p = derive.selfptr(), t = std::move(task), delay]() mutable
			{
				std::unique_ptr<asio::steady_timer> timer = std::make_unique<
					asio::steady_timer>(derive.io_->context());

				this->timed_tasks_.emplace(timer.get());

				derive.io_->timers().emplace(timer.get());

				timer->expires_after(delay);
				timer->async_wait(
				[this, &derive, p = std::move(p), timer = std::move(timer), t = std::move(t)]
				(const error_code& ec) mutable
				{
					ASIO2_ASSERT((!ec) || ec == asio::error::operation_aborted);

					derive.io_->timers().erase(timer.get());
					detail::ignore_unused(p);
					set_last_error(ec);
				#if defined(ASIO2_ENABLE_TIMER_CALLBACK_WHEN_ERROR)
					t();
				#else
					if (!ec)
					{
						t();
					}
				#endif
					this->timed_tasks_.erase(timer.get());
				});
			}));

			return future;
		}

		/**
		 * @brief Submits a completion token or function object for execution.
		 * This function submits an object for execution using the object's associated
		 * executor. The function object is queued for execution. if current thread is 
		 * the io_context's thread, the function will be executed immediately, otherwise 
		 * the task will be asynchronously post to io_context to execute.
		 */
		template<typename Function>
		inline derived_t & dispatch(Function&& fn)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			asio::dispatch(derive.io_->context(), make_allocator(derive.wallocator(),
			[p = derive.selfptr(), fn = std::forward<Function>(fn)]() mutable
			{
				detail::ignore_unused(p);

				fn();
			}));

			return derive;
		}

		/**
		 * @brief Submits a completion token or function object for execution.
		 * This function submits an object for execution using the object's associated
		 * executor. The function object is queued for execution. if current thread is
		 * the io_context's thread, the function will be executed immediately, otherwise
		 * the task will be asynchronously post to io_context to execute.
		 * note : Never call future's waiting function(eg:wait,get) in a communication(eg:on_recv)
		 * thread, it will cause dead lock;
		 */
		template<typename Function, typename Allocator>
		inline auto dispatch(Function&& fn, asio::use_future_t<Allocator>) ->
			std::future<std::invoke_result_t<Function>>
		{
			using return_type = std::invoke_result_t<Function>;

			derived_t& derive = static_cast<derived_t&>(*this);

			std::packaged_task<return_type()> task(std::forward<Function>(fn));

			std::future<return_type> future = task.get_future();

			// Make sure we run on the io_context thread
			asio::dispatch(derive.io_->context(), make_allocator(derive.wallocator(),
			[p = derive.selfptr(), t = std::move(task)]() mutable
			{
				detail::ignore_unused(p);

				t();
			}));

			return future;
		}

		/**
		 * @brief Stop all timed events which you posted with a delay duration.
		 */
		inline derived_t& stop_all_timed_events()
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			asio::post(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, p = derive.selfptr()]() mutable
			{
				detail::ignore_unused(p);

				for (asio::steady_timer* timer : this->timed_tasks_)
				{
					detail::cancel_timer(*timer);
				}
			}));

			return derive;
		}

		/**
		 * @brief Stop all timed tasks which you posted with a delay duration.
		 * This function is the same as stop_all_timed_events.
		 */
		inline derived_t& stop_all_timed_tasks()
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return derive.stop_all_timed_events();
		}

	protected:
		/**
		 * @brief Stop all timed events which you posted with a delay duration.
		 * Use dispatch instead of post, this function is used for inner.
		 */
		inline derived_t& _dispatch_stop_all_timed_events()
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			asio::dispatch(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, p = derive.selfptr()]() mutable
			{
				detail::ignore_unused(p);

				for (asio::steady_timer* timer : this->timed_tasks_)
				{
					detail::cancel_timer(*timer);
				}
			}));

			return derive;
		}

	protected:
		/// Used to exit the timer tasks when component is ready to stop.
		std::set<asio::steady_timer*> timed_tasks_;
	};
}

#endif // !__ASIO2_POST_COMPONENT_HPP__
