/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 *
 * refenced from : https://github.com/progschj/ThreadPool
 * see c++ 17 version : https://github.com/jhasse/ThreadPool
 * 
 * 
 * note :
 * 
 * 1 : when declare an global thread_pool object in dll,when enter the 
 *     constructor to create std::thread,it will blocking forever.
 * 2 : when declare an global thread_pool object in dll and when dll is
 *     released,the code will run into thread_pool destructor,then call
 *     notify_all in the destructor, but the notify_all calling will
 *     blocking forever.
 * 
 * one resolve method is add a start and stop function,and move the
 * notify_all into the stop inner,and tell user call the start and 
 * stop function manual.
 *  
 * but in order to keep the interface simple,we don't add stop function,
 * you can use "new" "delete" way to avoid above problems,you can delete
 * thread_pool pointer object before exit.
 * 
 * std::thread cause deadlock in DLLMain : 
 * The constructor for the std::thread cannot return until the new thread
 * starts executing the thread procedure. When a new thread is created, 
 * before the thread procedure is invoked, the entry point of each loaded
 * DLL is invoked for DLL_THREAD_ATTACH. To do this, the new thread must
 * acquire the loader lock. Unfortunately, your existing thread already
 * holds the loader lock.
 * 
 */

#ifndef __ASIO2_THREAD_POOL_HPP__
#define __ASIO2_THREAD_POOL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdlib>
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

namespace asio2
{
	/**
	 * thread pool interface, this pool is multi thread safed.
	 */
	class thread_pool
	{
	public:
		/**
		 * @constructor
		 */
		explicit thread_pool(std::size_t thread_count = std::thread::hardware_concurrency())
		{
			if (thread_count < 1)
				thread_count = 1;

			this->workers_.reserve(thread_count);

			for (std::size_t i = 0; i < thread_count; ++i)
			{
				// emplace_back can use the parameters to construct the std::thread object automictly
				// use lambda function as the thread proc function,lambda can has no parameters list
				this->workers_.emplace_back([this]
				{
					for (;;)
					{
						std::packaged_task<void()> task;

						{
							std::unique_lock<std::mutex> lock(this->mtx_);
							this->cv_.wait(lock, [this] { return (this->stop_ || !this->tasks_.empty()); });

							if (this->stop_ && this->tasks_.empty())
								return;

							task = std::move(this->tasks_.front());
							this->tasks_.pop();
						}

						task();
					}
				});
			}
		}

		/**
		 * @destructor
		 */
		~thread_pool()
		{
			{
				std::unique_lock<std::mutex> lock(this->mtx_);
				this->stop_ = true;
			}

			this->cv_.notify_all();

			for (auto & worker : this->workers_)
			{
				if (worker.joinable())
					worker.join();
			}
		}

		/**
		 * @function : post a function object into the thread pool, then return immediately,
		 * the function object will never be executed inside this function. Instead, it will
		 * be executed asynchronously in the thread pool.
		 * @param : fun - global function,static function,lambda,member function,std::function
		 * @return : std::future<...>
		 */
		template<class Fun, class... Args>
		auto post(Fun&& fun, Args&&... args) -> std::future<std::invoke_result_t<Fun, Args...>>
		{
			using return_type = std::invoke_result_t<Fun, Args...>;

			std::packaged_task<return_type()> task(
				std::bind(std::forward<Fun>(fun), std::forward<Args>(args)...));

			std::future<return_type> future = task.get_future();

			{
				std::unique_lock<std::mutex> lock(this->mtx_);

				// don't allow post after stopping the pool
				if (this->stop_)
					throw std::runtime_error("post a task into thread pool but the pool is stopped");

				this->tasks_.emplace(std::move(task));
			}

			this->cv_.notify_one();

			return future;
		}

		/**
		 * @function : get thread count of the thread pool
		 */
		inline std::size_t pool_size()
		{
			return this->workers_.size();
		}

		/**
		 * @function : get remain task size
		 */
		inline std::size_t task_size()
		{
			return this->tasks_.size();
		}

	private:
		/// no copy construct function
		thread_pool(const thread_pool&) = delete;

		/// no operator equal function
		thread_pool& operator=(const thread_pool&) = delete;

	protected:
		// need to keep track of threads so we can join them
		std::vector<std::thread> workers_;

		// the task queue
		std::queue<std::packaged_task<void()>> tasks_;

		// synchronization
		std::mutex mtx_;
		std::condition_variable cv_;

		// flag indicate the pool is stoped
		bool stop_ = false;
	};
}

#endif // !__ASIO2_THREAD_POOL_HPP__
