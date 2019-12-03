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

#include <future>
#include <functional>

#include <asio2/base/selector.hpp>
#include <asio2/base/iopool.hpp>

namespace asio2::detail
{
	template<class derived_t>
	class post_cp
	{
	public:
		/**
		 * @constructor
		 */
		post_cp() : derive(static_cast<derived_t&>(*this)) {}

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
			asio::post(derive.io().strand(), std::forward<Function>(f));
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
		inline auto post(Function&& f, asio::use_future_t<Allocator>) -> std::future<std::invoke_result_t<Function>>
		{
			using return_type = std::invoke_result_t<Function>;

			std::packaged_task<return_type()> task(std::forward<Function>(f));

			std::future<return_type> future = task.get_future();

			asio::post(derive.io().strand(), [t = std::move(task)]() mutable
			{
				t();
			});

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
			// Make sure we run on the strand
			if (derive.io().strand().running_in_this_thread())
				f();
			else
				asio::post(derive.io().strand(), std::forward<Function>(f));
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
		inline auto dispatch(Function&& f, asio::use_future_t<Allocator>) -> std::future<std::invoke_result_t<Function>>
		{
			using return_type = std::invoke_result_t<Function>;

			std::packaged_task<return_type()> task(std::forward<Function>(f));

			std::future<return_type> future = task.get_future();

			// Make sure we run on the strand
			if (derive.io().strand().running_in_this_thread())
			{
				task();
			}
			else
			{
				asio::post(derive.io().strand(), [t = std::move(task)]() mutable
				{
					t();
				});
			}

			return future;
		}

	protected:
		derived_t & derive;
	};
}

#endif // !__ASIO2_POST_COMPONENT_HPP__
