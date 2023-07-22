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
	class thread_group;

	/**
	 * thread pool interface, this pool is multi thread safed.
	 * the tasks will be running in random thread.
	 */
	class thread_pool
	{
		friend class thread_group;

	public:
		/**
		 * @brief constructor
		 */
		explicit thread_pool(std::size_t thread_count = std::thread::hardware_concurrency())
		{
			if (thread_count < static_cast<std::size_t>(1))
				thread_count = static_cast<std::size_t>(1);

			this->workers_.reserve(thread_count);

			for (std::size_t i = 0; i < thread_count; ++i)
			{
				// emplace_back can use the parameters to construct the std::thread object automictly
				// use lambda function as the thread proc function,lambda can has no parameters list
				this->workers_.emplace_back([this]() mutable
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
		 * @brief destructor
		 */
		~thread_pool()
		{
			this->stop();
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
		 * @brief get thread count of the thread pool with no lock
		 */
		inline std::size_t thread_count() noexcept
		{
			return this->workers_.size();
		}

		/**
		 * @brief get thread count of the thread pool with lock
		 */
		inline std::size_t get_thread_count() noexcept
		{
			std::unique_lock<std::mutex> lock(this->mtx_);

			return this->workers_.size();
		}

		/**
		 * @brief get thread count of the thread pool with no lock, same as thread_count()
		 */
		inline std::size_t pool_size() noexcept
		{
			return this->workers_.size();
		}

		/**
		 * @brief get thread count of the thread pool with lock, same as get_thread_count()
		 */
		inline std::size_t get_pool_size() noexcept
		{
			// is std container.size() thread safety ?
			// @see: the size() function in file: /asio2/base/session_mgr.hpp

			std::unique_lock<std::mutex> lock(this->mtx_);

			return this->workers_.size();
		}

		/**
		 * @brief get remain task size with no lock
		 */
		inline std::size_t task_size() noexcept
		{
			return this->tasks_.size();
		}

		/**
		 * @brief get remain task size with lock
		 */
		inline std::size_t get_task_size() noexcept
		{
			std::unique_lock<std::mutex> lock(this->mtx_);

			return this->tasks_.size();
		}

		/**
		 * @brief Determine whether current code is running in the pool's threads.
		 */
		inline bool running_in_threads() noexcept
		{
			std::unique_lock<std::mutex> lock(this->mtx_);

			std::thread::id curr_tid = std::this_thread::get_id();
			for (std::thread& thread : this->workers_)
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
		 * @brief Get the thread id of the specified thread index with no lock.
		 */
		inline std::thread::id thread_id(std::size_t index) noexcept
		{
			return this->workers_[index % this->workers_.size()].get_id();
		}

		/**
		 * @brief Get the thread id of the specified thread index with lock.
		 */
		inline std::thread::id get_thread_id(std::size_t index) noexcept
		{
			std::unique_lock<std::mutex> lock(this->mtx_);
			return this->workers_[index % this->workers_.size()].get_id();
		}

	protected:
		/**
		 * @brief Stop the thread pool and block until all tasks finish executing
		 */
		void stop()
		{
			{
				std::unique_lock<std::mutex> lock(this->mtx_);
				this->stop_ = true;
			}

			this->cv_.notify_all();

			for (std::thread& worker : this->workers_)
			{
				if (worker.joinable())
					worker.join();
			}
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

	/**
	 * thread group interface, this group is multi thread safed.
	 * the task will be running in the specified thread.
	 */
	class thread_group
	{
	public:
		// Avoid conflicts with thread_pool in other namespace
		using worker_t = asio2::thread_pool;

		/**
		 * @brief constructor
		 */
		explicit thread_group(std::size_t thread_count = std::thread::hardware_concurrency())
		{
			if (thread_count < static_cast<std::size_t>(1))
				thread_count = static_cast<std::size_t>(1);

			this->workers_.reserve(thread_count);

			for (std::size_t i = 0; i < thread_count; ++i)
			{
				this->workers_.emplace_back(new worker_t(static_cast<std::size_t>(1)));
			}
		}

		/**
		 * @brief destructor
		 */
		~thread_group()
		{
			// must block until all threads exited, otherwise maybe cause crash.
			// eg: 
			// asio2::thread_group thpool;
			// thpool.post(1, [&thpool]()
			// {
			//    // here, if the thread 0 is deleted already, this function will cause crash.
			//    thpool.running_in_threads();
			// });
			for (worker_t* p : this->workers_)
			{
				p->stop();
			}

			for (worker_t* p : this->workers_)
			{
				delete p;
			}

			this->workers_.clear();
		}

		/**
		 * @brief post a function object into the thread group with specified thread index, 
		 * then return immediately, the function object will never be executed inside this 
		 * function. Instead, it will be executed asynchronously in the thread group.
		 * @param thread_index - which thread to execute the function.
		 * @param fun - global function,static function,lambda,member function,std::function.
		 * @return std::future<fun_return_type>
		 */
		template<class IntegerT, class Fun, class... Args,
			std::enable_if_t<std::is_integral_v<std::remove_cv_t<std::remove_reference_t<IntegerT>>>, int> = 0>
		auto post(IntegerT thread_index, Fun&& fun, Args&&... args) -> std::future<std::invoke_result_t<Fun, Args...>>
		{
			return this->workers_[thread_index % this->workers_.size()]->post(
				std::forward<Fun>(fun), std::forward<Args>(args)...);
		}

		/**
		 * @brief get thread count of the thread group with no lock
		 */
		inline std::size_t thread_count() noexcept
		{
			return this->workers_.size();
		}

		/**
		 * @brief get thread count of the thread group with lock
		 */
		inline std::size_t get_thread_count() noexcept
		{
			return this->workers_.size();
		}

		/**
		 * @brief get thread count of the thread group with no lock, same as thread_count()
		 */
		inline std::size_t pool_size() noexcept
		{
			return this->workers_.size();
		}

		/**
		 * @brief get thread count of the thread group with lock, same as get_thread_count()
		 */
		inline std::size_t get_pool_size() noexcept
		{
			return this->workers_.size();
		}

		/**
		 * @brief get remain task size with no lock
		 */
		inline std::size_t task_size() noexcept
		{
			std::size_t count = 0;

			for (worker_t* p : this->workers_)
			{
				count += p->task_size();
			}

			return count;
		}

		/**
		 * @brief get remain task size with lock
		 */
		inline std::size_t get_task_size() noexcept
		{
			std::size_t count = 0;

			for (worker_t* p : this->workers_)
			{
				count += p->get_task_size();
			}

			return count;
		}

		/**
		 * @brief get remain task size of the specified thread with no lock
		 */
		inline std::size_t task_size(std::size_t thread_index) noexcept
		{
			return this->workers_[thread_index % this->workers_.size()]->task_size();
		}

		/**
		 * @brief get remain task size of the specified thread with lock
		 */
		inline std::size_t get_task_size(std::size_t thread_index) noexcept
		{
			return this->workers_[thread_index % this->workers_.size()]->get_task_size();
		}

		/**
		 * @brief Determine whether current code is running in the group's threads.
		 */
		inline bool running_in_threads() noexcept
		{
			for (worker_t* p : this->workers_)
			{
				if (p->running_in_threads())
					return true;
			}

			return false;
		}

		/**
		 * @brief Determine whether current code is running in the thread by index
		 */
		inline bool running_in_thread(std::size_t index) noexcept
		{
			if (!(index < this->workers_.size()))
				return false;

			return this->workers_[index]->running_in_thread(0);
		}

		/**
		 * @brief Get the thread id of the specified thread index with no lock.
		 */
		inline std::thread::id thread_id(std::size_t index) noexcept
		{
			return this->workers_[index % this->workers_.size()]->thread_id(0);
		}

		/**
		 * @brief Get the thread id of the specified thread index with lock.
		 */
		inline std::thread::id get_thread_id(std::size_t index) noexcept
		{
			return this->workers_[index % this->workers_.size()]->get_thread_id(0);
		}

	private:
		/// no copy construct function
		thread_group(const thread_group&) = delete;

		/// no operator equal function
		thread_group& operator=(const thread_group&) = delete;

	protected:
		// 
		std::vector<worker_t*> workers_;
	};
}

#endif // !__ASIO2_THREAD_POOL_HPP__
