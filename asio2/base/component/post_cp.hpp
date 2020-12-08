/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_POST_COMPONENT_HPP__
#define __ASIO2_POST_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <set>
#include <future>
#include <functional>

#include <asio2/base/selector.hpp>
#include <asio2/base/iopool.hpp>
#include <asio2/base/detail/allocator.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t = void>
	class post_cp
	{
	public:
		/**
		 * @constructor
		 */
		post_cp() {}

		/**
		 * @destructor
		 */
		~post_cp() = default;

	public:
		/**
		 * @function : Submits a completion token or function object for execution.
		 * This function submits an object for execution using the object's associated
		 * executor. The function object is queued for execution, and is never called
		 * from the current thread prior to returning from <tt>post()</tt>.
		 */
		template<typename Function>
		inline derived_t & post(Function&& f)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			asio::post(derive.io().strand(), make_allocator(
				derive.wallocator(), std::forward<Function>(f)));
			return (derive);
		}

		/**
		 * @function : Submits a completion token or function object for execution after specified delay time.
		 * This function submits an object for execution using the object's associated
		 * executor. The function object is queued for execution, and is never called
		 * from the current thread prior to returning from <tt>post()</tt>.
		 */
		template<typename Function, typename Rep, typename Period>
		inline derived_t & post(Function&& f, std::chrono::duration<Rep, Period> delay)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			std::unique_ptr<asio::steady_timer> timer = std::make_unique<
				asio::steady_timer>(derive.io().context());

			asio::dispatch(derive.io().strand(), make_allocator(derive.wallocator(),
				[this, self = derive.selfptr(), key = timer.get()]() mutable
			{
				this->timed_tasks_.emplace(key);
			}));

			timer->expires_after(delay);
			timer->async_wait(asio::bind_executor(derive.io().strand(),
				make_allocator(derive.wallocator(), [this, self = derive.selfptr(),
					timer = std::move(timer), f = std::forward<Function>(f)](const error_code& ec) mutable
			{
				f();
				this->timed_tasks_.erase(timer.get());
			})));
			return (derive);
		}

		/**
		 * @function : Submits a completion token or function object for execution.
		 * This function submits an object for execution using the object's associated
		 * executor. The function object is queued for execution, and is never called
		 * from the current thread prior to returning from <tt>post()</tt>.
		 * note : Never call future's waiting function(eg:wait,get) in a communication(eg:on_recv)
		 * thread, it will cause dead lock;
		 */
		template<typename Function, typename Allocator>
		inline auto post(Function&& f, asio::use_future_t<Allocator>) ->
			std::future<std::invoke_result_t<Function>>
		{
			using return_type = std::invoke_result_t<Function>;

			derived_t& derive = static_cast<derived_t&>(*this);

			std::packaged_task<return_type()> task(std::forward<Function>(f));

			std::future<return_type> future = task.get_future();

			asio::post(derive.io().strand(), make_allocator(derive.wallocator(),
				[t = std::move(task)]() mutable
			{
				t();
			}));

			return future;
		}

		/**
		 * @function : Submits a completion token or function object for execution after specified delay time.
		 * This function submits an object for execution using the object's associated
		 * executor. The function object is queued for execution, and is never called
		 * from the current thread prior to returning from <tt>post()</tt>.
		 * note : Never call future's waiting function(eg:wait,get) in a communication(eg:on_recv)
		 * thread, it will cause dead lock;
		 */
		template<typename Function, typename Rep, typename Period, typename Allocator>
		inline auto post(Function&& f, std::chrono::duration<Rep, Period> delay, asio::use_future_t<Allocator>)
			-> std::future<std::invoke_result_t<Function>>
		{
			using return_type = std::invoke_result_t<Function>;

			derived_t& derive = static_cast<derived_t&>(*this);

			std::packaged_task<return_type()> task(std::forward<Function>(f));

			std::future<return_type> future = task.get_future();

			std::unique_ptr<asio::steady_timer> timer = std::make_unique<
				asio::steady_timer>(derive.io().context());

			asio::dispatch(derive.io().strand(), make_allocator(derive.wallocator(),
				[this, self = derive.selfptr(), key = timer.get()]() mutable
			{
				this->timed_tasks_.emplace(key);
			}));

			timer->expires_after(delay);
			timer->async_wait(asio::bind_executor(derive.io().strand(),
				make_allocator(derive.wallocator(), [this, self = derive.selfptr(),
					timer = std::move(timer), t = std::move(task)](const error_code& ec) mutable
			{
				t();
				this->timed_tasks_.erase(timer.get());
			})));

			return future;
		}

		/**
		 * @function : Submits a completion token or function object for execution.
		 * This function submits an object for execution using the object's associated
		 * executor. The function object is queued for execution. if current thread is 
		 * the strand's thread, the function will be executed immediately, otherwise 
		 * the task will be asynchronously post to strand to execute.
		 */
		template<typename Function>
		inline derived_t & dispatch(Function&& f)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			asio::dispatch(derive.io().strand(), make_allocator(
				derive.wallocator(), std::forward<Function>(f)));
			return (derive);
		}

		/**
		 * @function : Submits a completion token or function object for execution.
		 * This function submits an object for execution using the object's associated
		 * executor. The function object is queued for execution. if current thread is
		 * the strand's thread, the function will be executed immediately, otherwise
		 * the task will be asynchronously post to strand to execute.
		 * note : Never call future's waiting function(eg:wait,get) in a communication(eg:on_recv)
		 * thread, it will cause dead lock;
		 */
		template<typename Function, typename Allocator>
		inline auto dispatch(Function&& f, asio::use_future_t<Allocator>) ->
			std::future<std::invoke_result_t<Function>>
		{
			using return_type = std::invoke_result_t<Function>;

			derived_t& derive = static_cast<derived_t&>(*this);

			std::packaged_task<return_type()> task(std::forward<Function>(f));

			std::future<return_type> future = task.get_future();

			// Make sure we run on the strand
			if (derive.io().strand().running_in_this_thread())
			{
				task();
			}
			else
			{
				asio::post(derive.io().strand(), make_allocator(derive.wallocator(),
					[t = std::move(task)]() mutable
				{
					t();
				}));
			}

			return future;
		}

		/**
		 * @function : Stop all timed tasks which you posted with a delay duration.
		 */
		inline derived_t& stop_all_timed_tasks()
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// Make sure we run on the strand
			if (!derive.io().strand().running_in_this_thread())
			{
				asio::post(derive.io().strand(), make_allocator(derive.wallocator(),
					[this, this_ptr = derive.selfptr()]() mutable
				{
					this->stop_all_timed_tasks();
				}));
				return (derive);
			}

			for (asio::steady_timer* timer : this->timed_tasks_)
			{
				timer->cancel(ec_ignore);
			}

			return (derive);
		}

	protected:
		/// Used to exit the timer tasks when component is ready to stop.
		std::set<asio::steady_timer*> timed_tasks_;
	};
}

#endif // !__ASIO2_POST_COMPONENT_HPP__
