/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
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
		 * @brief constructor
		 */
		explicit thread_pool(std::size_t thread_count = std::thread::hardware_concurrency())
		{
			if (thread_count < 1)
				thread_count = 1;

			this->indexed_tasks_ = std::vector<std::queue<std::packaged_task<void()>>>{ thread_count };

			this->workers_.reserve(thread_count);

			for (std::size_t i = 0; i < thread_count; ++i)
			{
				// emplace_back can use the parameters to construct the std::thread object automictly
				// use lambda function as the thread proc function,lambda can has no parameters list
				this->workers_.emplace_back([this, i]() mutable
				{
					std::queue<std::packaged_task<void()>>& indexed_tasks = this->indexed_tasks_[i];

					for (;;)
					{
						std::packaged_task<void()> task;

						{
							std::unique_lock<std::mutex> lock(this->mtx_);
							this->cv_.wait(lock, [this, &indexed_tasks]
								{ return (this->stop_ || !this->tasks_.empty() || !indexed_tasks.empty()); });

							if (this->stop_ && this->tasks_.empty() && indexed_tasks.empty())
								return;

							if (indexed_tasks.empty())
							{
								task = std::move(this->tasks_.front());
								this->tasks_.pop();
							}
							else
							{
								task = std::move(indexed_tasks.front());
								indexed_tasks.pop();
							}
						}

						task();
					}
				});
			}
		}

		/**
		 * @brief destructor
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
		 * @brief post a function object into the thread pool, then return immediately,
		 * the function object will never be executed inside this function. Instead, it will
		 * be executed asynchronously in the thread pool.
		 * @param fun - global function,static function,lambda,member function,std::function.
		 * @return std::future<fun_return_type>
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
		 * @brief post a function object into the thread pool with specified thread index, 
		 * then return immediately, the function object will never be executed inside this 
		 * function. Instead, it will be executed asynchronously in the thread pool.
		 * @param thread_index - which thread to execute the function.
		 * @param fun - global function,static function,lambda,member function,std::function.
		 * @return std::future<fun_return_type>
		 */
		template<class IntegerT, class Fun, class... Args,
			std::enable_if_t<std::is_integral_v<std::remove_cv_t<std::remove_reference_t<IntegerT>>>, int> = 0>
		auto post(IntegerT thread_index, Fun&& fun, Args&&... args) -> std::future<std::invoke_result_t<Fun, Args...>>
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

				this->indexed_tasks_[thread_index % this->indexed_tasks_.size()].emplace(std::move(task));
			}

			this->cv_.notify_one();

			return future;
		}

		/**
		 * @brief get thread count of the thread pool, same as get_pool_size()
		 */
		inline std::size_t pool_size() noexcept
		{
			return this->get_pool_size();
		}

		/**
		 * @brief get thread count of the thread pool
		 */
		inline std::size_t get_pool_size() noexcept
		{
			// is std container.size() thread safety ?
			// @see: the size() function in file: /asio2/base/session_mgr.hpp

			std::unique_lock<std::mutex> lock(this->mtx_);

			return this->workers_.size();
		}

		/**
		 * @brief get remain task size, same as get_task_size()
		 */
		inline std::size_t task_size() noexcept
		{
			return this->get_task_size();
		}

		/**
		 * @brief get remain task size
		 */
		inline std::size_t get_task_size() noexcept
		{
			std::unique_lock<std::mutex> lock(this->mtx_);

			std::size_t count = this->tasks_.size();

			for (std::queue<std::packaged_task<void()>>& queue : this->indexed_tasks_)
			{
				count += queue.size();
			}

			return count;
		}

		/**
		 * @brief Determine whether current code is running in the pool's threads.
		 */
		inline bool running_in_threads() noexcept
		{
			std::unique_lock<std::mutex> lock(this->mtx_);

			std::thread::id curr_tid = std::this_thread::get_id();
			for (auto & thread : this->workers_)
			{
				if (curr_tid == thread.get_id())
					return true;
			}

			return false;
		}

		/**
		 * @brief Determine whether current code is running in the thread by index
		 */
		inline bool running_in_thread(std::size_t index) noexcept
		{
			std::unique_lock<std::mutex> lock(this->mtx_);

			if (!(index < this->workers_.size()))
				return false;

			return (std::this_thread::get_id() == this->workers_[index].get_id());
		}

		/**
		 * @brief Get the thread id of the specified thread index.
		 */
		inline std::thread::id get_thread_id(std::size_t index) noexcept
		{
			std::unique_lock<std::mutex> lock(this->mtx_);
			return this->workers_[index % this->workers_.size()].get_id();
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

		// the task queue with thread index can be specified
		std::vector<std::queue<std::packaged_task<void()>>> indexed_tasks_;

		// synchronization
		std::mutex mtx_;
		std::condition_variable cv_;

		// flag indicate the pool is stoped
		bool stop_ = false;
	};
}

#endif // !__ASIO2_THREAD_POOL_HPP__
