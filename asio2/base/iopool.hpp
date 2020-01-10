/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_IOPOOL_HPP__
#define __ASIO2_IOPOOL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <type_traits>

#include <asio2/base/selector.hpp>

namespace asio2::detail
{
	class io_t
	{
	public:
		io_t() : context_(1), strand_(context_) {}
		~io_t() = default;

		inline asio::io_context & context() { return this->context_; }
		inline asio::io_context::strand &  strand() { return this->strand_; }

	protected:
		asio::io_context context_;
		asio::io_context::strand strand_;
	};

	/**
	 * io_context pool
	 */
	class iopool
	{
	public:
		/**
		 * @constructor
		 * @param    : concurrency - the pool size,default is double the number of CPU cores
		 */
		explicit iopool(std::size_t concurrency = std::thread::hardware_concurrency() * 2)
			: ios_(concurrency == 0 ? std::thread::hardware_concurrency() * 2 : concurrency)
		{
			this->threads_.reserve(this->ios_.size());
			this->works_.reserve(this->ios_.size());
		}

		/**
		 * @destructor
		 */
		~iopool()
		{
			this->stop();
		}

		/**
		 * @function : run all io_context objects in the pool.
		 */
		bool start()
		{
			std::lock_guard<std::mutex> guard(this->mutex_);

			if (!this->stopped_)
				return false;

			if (!this->works_.empty() || !this->threads_.empty())
				return false;

			// Create a pool of threads to run all of the io_contexts. 
			for (auto & io : this->ios_)
			{
				/// Restart the io_context in preparation for a subsequent run() invocation.
				/**
				 * This function must be called prior to any second or later set of
				 * invocations of the run(), run_one(), poll() or poll_one() functions when a
				 * previous invocation of these functions returned due to the io_context
				 * being stopped or running out of work. After a call to restart(), the
				 * io_context object's stopped() function will return @c false.
				 *
				 * This function must not be called while there are any unfinished calls to
				 * the run(), run_one(), poll() or poll_one() functions.
				 */
				io.context().restart();

				this->works_.emplace_back(io.context().get_executor());

				// start work thread
				this->threads_.emplace_back([&io]()
				{
					io.context().run();
				});
			}

			this->stopped_ = false;

			return true;
		}

		/**
		 * @function : stop all io_context objects in the pool
		 * blocking until all posted event has completed already.
		 * After we call work.reset(), when an asio::post(strand,...) execution ends, the count
		 * of the strand will be checked. If the count equals 0, the strand will be closed. Then 
		 * the subsequent call of asio:: post(strand,...) will fail, and the post event will not
		 * be executed. When our program exits, it will nest call asio:: post (strand...) to post
		 * many events, so when an asio::post(strand,...) inside someone asio::post(strand,...)
		 * has not yet been executed, the strand may have been closed, which will result in the
		 * nested asio::post(strand,...) never being executed.
		 */
		void stop()
		{
			{
				std::lock_guard<std::mutex> guard(this->mutex_);

				if (this->stopped_)
					return;

				if (this->works_.empty() && this->threads_.empty())
					return;

				if (this->running_in_iopool_threads())
					return;

				this->stopped_ = true;
			}

			// Waiting for all nested events to complete.
			// The mutex_ must be released while waiting, otherwise, the stop function may be called
			// in the communication thread and the lock will be requested, which is already held here,
			// so leading to deadlock.
			this->wait_iothreads();

			{
				std::lock_guard<std::mutex> guard(this->mutex_);

				// call work reset,and then the io_context working thread will be exited.
				for (auto & work : this->works_)
				{
					work.reset();
				}

				// Wait for all threads to exit. 
				for (auto & thread : this->threads_)
				{
					thread.join();
				}

				this->works_.clear();
				this->threads_.clear();
			}
		}

		/**
		 * @function : check whether the iopool is started
		 */
		inline bool is_started() const
		{
			return (!this->stopped_);
		}

		/**
		 * @function : check whether the iopool is stopped
		 */
		inline bool is_stopped() const
		{
			return (this->stopped_);
		}

		/**
		 * @function : get an io_context to use
		 */
		inline io_t & get(std::size_t index = static_cast<std::size_t>(-1))
		{
			// Use a round-robin scheme to choose the next io_context to use. 
			return this->ios_[index < this->ios_.size() ? index : ((++(this->next_)) % this->ios_.size())];
		}

		/**
		 * @function : Determine whether current code is running in the iopool threads.
		 */
		inline bool running_in_iopool_threads()
		{
			std::thread::id curr_tid = std::this_thread::get_id();
			for (auto & thread : this->threads_)
			{
				if (curr_tid == thread.get_id())
					return true;
			}
			return false;
		}

		/**
		 * Use to ensure that all nested asio::post(...) events are fully invoked.
		 */
		void wait_iothreads()
		{
			{
				std::lock_guard<std::mutex> guard(this->mutex_);

				if (this->running_in_iopool_threads())
					return;

				// first reset the acceptor io_context work guard
				if (!this->works_.empty())
					this->works_.front().reset();
			}

			constexpr auto max = std::chrono::milliseconds(10);
			constexpr auto min = std::chrono::milliseconds(1);

			// second wait indefinitely until the acceptor io_context is stopped
			if (!this->ios_.empty())
			{
				auto t1 = std::chrono::steady_clock::now();
				asio::io_context & ioc = this->ios_.front().context();
				while (!ioc.stopped())
				{
					auto t2 = std::chrono::steady_clock::now();
					std::this_thread::sleep_for(
						(std::max<std::chrono::steady_clock::duration>)(
						(std::min<std::chrono::steady_clock::duration>)(t2 - t1, max), min));
				}
			}

			{
				std::lock_guard<std::mutex> guard(this->mutex_);

				for (std::size_t i = 1; i < this->works_.size(); ++i)
				{
					this->works_[i].reset();
				}
			}

			for (std::size_t i = 1; i < this->ios_.size(); ++i)
			{
				auto t1 = std::chrono::steady_clock::now();
				asio::io_context & ioc = this->ios_[i].context();
				while (!ioc.stopped())
				{
					auto t2 = std::chrono::steady_clock::now();
					std::this_thread::sleep_for(
						(std::max<std::chrono::steady_clock::duration>)(
						(std::min<std::chrono::steady_clock::duration>)(t2 - t1, max), min));
				}
			}
		}

	protected:
		/// threads to run all of the io_context
		std::vector<std::thread>       threads_;

		/// The pool of io_context. 
		std::vector<io_t>              ios_;

		/// 
		std::mutex                     mutex_;

		/// Flag whether the io_context pool has stopped already
		bool                           stopped_  = true;

		/// The next io_context to use for a connection. 
		std::size_t                    next_     = 0;

		// Give all the io_contexts work to do so that their run() functions will not 
		// exit until they are explicitly stopped. 
		std::vector<asio::executor_work_guard<asio::io_context::executor_type>> works_;
	};

	class iopool_cp
	{
	public:
		iopool_cp(std::size_t concurrency) : iopool_(concurrency) {}
		~iopool_cp() = default;

	protected:
		/// the io_context pool for socket event
		iopool iopool_;
	};
}

#endif // !__ASIO2_IOPOOL_HPP__
