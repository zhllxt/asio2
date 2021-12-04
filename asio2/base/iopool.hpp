/*
 * COPYRIGHT (C) 2017-2021, zhllxt
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
#include <memory>
#include <algorithm>

#include <asio2/3rd/asio.hpp>
#include <asio2/base/error.hpp>
#include <asio2/base/detail/util.hpp>

namespace asio2::detail
{
	// the executor_work_guard sfinae must       use "(std::declval<std::decay_t<T>>())"
	// the io_context          sfinae must can't use "(std::declval<std::decay_t<T>>())"

	// it's very impossible, i don't know why.

	template<class T, class R = void>
	struct is_io_context_pointer : std::false_type {};

	template<class T>
	struct is_io_context_pointer<T, std::void_t<decltype(
		std::declval<std::decay_t<T>>()->~io_context()), void>> : std::true_type {};

	template<class T, class R = void>
	struct is_io_context_object : std::false_type {};

	template<class T>
	struct is_io_context_object<T, std::void_t<decltype(
		std::declval<std::decay_t<T>>().~io_context()), void>> : std::true_type {};


	template<class T, class R = void>
	struct is_executor_work_guard_pointer : std::false_type {};

	template<class T>
	struct is_executor_work_guard_pointer<T, std::void_t<decltype(
		(std::declval<std::decay_t<T>>())->~executor_work_guard()), void>> : std::true_type {};

	template<class T, class R = void>
	struct is_executor_work_guard_object : std::false_type {};

	template<class T>
	struct is_executor_work_guard_object<T, std::void_t<decltype(
		(std::declval<std::decay_t<T>>()).~executor_work_guard()), void>> : std::true_type {};


	static_assert(is_io_context_pointer<asio::io_context*>::value);
	static_assert(is_io_context_pointer<asio::io_context*&>::value);
	static_assert(is_io_context_pointer<asio::io_context*&&>::value);
	static_assert(is_io_context_pointer<std::shared_ptr<asio::io_context>>::value);
	static_assert(is_io_context_pointer<std::shared_ptr<asio::io_context>&>::value);
	static_assert(is_io_context_pointer<std::shared_ptr<asio::io_context>const&>::value);
	static_assert(is_io_context_pointer<std::shared_ptr<asio::io_context>&&>::value);
	static_assert(is_io_context_object<asio::io_context>::value);
	static_assert(is_io_context_object<asio::io_context&>::value);
	static_assert(is_io_context_object<asio::io_context&&>::value);


	/**
	 * io_context pool
	 */
	class iopool
	{
	public:
		/**
		 * @constructor
		 * @param    : concurrency - the pool size, default is double the number of CPU cores
		 */
		explicit iopool(std::size_t concurrency = default_concurrency())
		{
			if (concurrency == 0)
			{
				concurrency = default_concurrency();
			}

			for (std::size_t i = 0; i < concurrency; ++i)
			{
				this->ios_.emplace_back(std::make_unique<asio::io_context>(1));
			}

			this->threads_.reserve(this->ios_.size());
			this->guards_.reserve(this->ios_.size());
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
			clear_last_error();

			std::lock_guard<std::mutex> guard(this->mutex_);

			if (!this->stopped_)
			{
				set_last_error(asio::error::already_started);
				return true;
			}

			if (!this->guards_.empty() || !this->threads_.empty())
			{
				set_last_error(asio::error::already_started);
				return true;
			}

			// Create a pool of threads to run all of the io_contexts. 
			for (auto & ioc : this->ios_)
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
				ioc->restart();

				this->guards_.emplace_back(ioc->get_executor());

				// start work thread
				this->threads_.emplace_back([&ioc]() mutable
				{
					ioc->run();
				});
			}

			this->stopped_ = false;

			return true;
		}

		/**
		 * @function : stop all io_context objects in the pool
		 * blocking until all posted event has completed already.
		 * After we call iog.reset(), when an asio::post(strand,...) execution ends, the count
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

				if (this->guards_.empty() && this->threads_.empty())
					return;

				if (this->running_in_threads())
					return;

				this->stopped_ = true;
			}

			// Waiting for all nested events to complete.
			// The mutex_ must be released while waiting, otherwise, the stop function may be called
			// in the communication thread and the lock will be requested, which is already held here,
			// so leading to deadlock.
			this->wait_for_io_context_stopped();

			{
				std::lock_guard<std::mutex> guard(this->mutex_);

				// call executor_work_guard reset,and then the io_context working thread will be exited.
				// In fact, the guards has called reset already, but there is no problem with repeated calls
				for (auto & iog : this->guards_)
				{
					ASIO2_ASSERT(iog.owns_work() == false);
					iog.reset();
				}

				// Wait for all threads to exit. 
				for (auto & thread : this->threads_)
				{
					thread.join();
				}

				this->guards_.clear();
				this->threads_.clear();
			}
		}

		/**
		 * @function : check whether the io_context pool is stopped
		 */
		inline bool stopped() const
		{
			return (this->stopped_);
		}

		/**
		 * @function : get an io_context to use
		 */
		inline asio::io_context& get(std::size_t index = static_cast<std::size_t>(-1))
		{
			ASIO2_ASSERT(!this->ios_.empty());

			// Use a round-robin scheme to choose the next io_context to use. 
			return *(this->ios_[index < this->ios_.size() ? index : ((++(this->next_)) % this->ios_.size())]);
		}

		/**
		 * @function : call user custom callback function for every io_context
		 * the custom callback function is like this :
		 * void on_callback(asio::io_context& ioc)
		 */
		template<class Function>
		inline void for_each(Function&& f)
		{
			std::lock_guard<std::mutex> guard(this->mutex_);

			for (auto& ioc : this->ios_)
			{
				f(*ioc);
			}
		}

		/**
		 * @function : Determine whether current code is running in the io_context pool threads.
		 */
		inline bool running_in_threads() const
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
		 * @function : Determine whether current code is running in the io_context thread by index
		 */
		inline bool running_in_thread(std::size_t index) const
		{
			ASIO2_ASSERT(index < this->threads_.size());

			if (!(index < this->threads_.size()))
				throw std::out_of_range("Invalid index.");

			return (std::this_thread::get_id() == this->threads_[index].get_id());
		}

		/**
		 * @function : get io_context pool size.
		 */
		inline std::size_t size() const
		{
			return this->ios_.size();
		}

		/**
		 * Use to ensure that all nested asio::post(...) events are fully invoked.
		 */
		inline void wait_for_io_context_stopped()
		{
			{
				std::lock_guard<std::mutex> guard(this->mutex_);

				if (this->running_in_threads())
					return;

				// first reset the acceptor io_context work guard
				if (!this->guards_.empty())
					this->guards_.front().reset();
			}

			constexpr auto max = std::chrono::milliseconds(10);
			constexpr auto min = std::chrono::milliseconds(1);

			// second wait indefinitely until the acceptor io_context is stopped
			if (!this->ios_.empty())
			{
				auto t1 = std::chrono::steady_clock::now();
				auto& ioc = this->ios_.front();
				while (!ioc->stopped())
				{
					auto t2 = std::chrono::steady_clock::now();
					auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
					std::this_thread::sleep_for(std::clamp(ms, min, max));
				}
			}

			{
				std::lock_guard<std::mutex> guard(this->mutex_);

				for (std::size_t i = 1; i < this->guards_.size(); ++i)
				{
					this->guards_[i].reset();
				}
			}

			for (std::size_t i = 1; i < this->ios_.size(); ++i)
			{
				auto t1 = std::chrono::steady_clock::now();
				auto& ioc = this->ios_[i];
				while (!ioc->stopped())
				{
					auto t2 = std::chrono::steady_clock::now();
					auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
					std::this_thread::sleep_for(std::clamp(ms, min, max));
				}
			}
		}

	protected:
		/// threads to run all of the io_context
		std::vector<std::thread>                                     threads_;

		/// The pool of io_context. 
		std::vector<std::unique_ptr<asio::io_context>>               ios_;

		/// 
		std::mutex                                                   mutex_;

		/// Flag whether the io_context pool has stopped already
		bool                                                         stopped_  = true;

		/// The next io_context to use for a connection. 
		std::size_t                                                  next_     = 0;

		// Give all the io_contexts executor_work_guard to do so that their run() functions will not 
		// exit until they are explicitly stopped. 
		std::vector<asio::executor_work_guard<asio::io_context::executor_type>> guards_;
	};

	class iopool_base
	{
	public:
		iopool_base() = default;
		virtual ~iopool_base() = default;

		virtual bool              start()                                            = 0;
		virtual void              stop ()                                            = 0;
		virtual bool              stopped() const                                    = 0;
		virtual asio::io_context& get(std::size_t index)                             = 0;
		virtual void              for_each(std::function<void(asio::io_context&)> f) = 0;
		virtual std::size_t       size() const                                       = 0;
	};

	class default_iopool : public iopool_base
	{
	public:
		explicit default_iopool(std::size_t concurrency) : impl_(concurrency)
		{
		}

		/**
		 * @destructor
		 */
		virtual ~default_iopool()
		{
			this->impl_.stop();
		}

		/**
		 * @function : run all io_context objects in the pool.
		 */
		virtual bool start() override
		{
			return this->impl_.start();
		}

		/**
		 * @function : stop all io_context objects in the pool
		 */
		virtual void stop() override
		{
			this->impl_.stop();
		}

		/**
		 * @function : check whether the io_context pool is stopped
		 */
		virtual bool stopped() const override
		{
			return this->impl_.stopped();
		}

		/**
		 * @function : get an io_context to use
		 */
		virtual asio::io_context& get(std::size_t index) override
		{
			return this->impl_.get(index);
		}

		/**
		 * @function : call user custom callback function for every io_context
		 */
		virtual void for_each(std::function<void(asio::io_context&)> f) override
		{
			this->impl_.for_each(std::move(f));
		}

		/**
		 * @function : get io_context pool size.
		 */
		virtual std::size_t size() const override
		{
			return this->impl_.size();
		}

	protected:
		iopool impl_;
	};

	/**
	 * This io_context pool is passed in by the user
	 */
	template<class IoContextPointerContainer>
	class user_iopool : public iopool_base
	{
	public:
		using user_container = typename detail::remove_cvref_t<IoContextPointerContainer>;
		using value_type     = typename user_container::value_type;
		using container      = std::vector<value_type>;

		static_assert(is_io_context_pointer<value_type>::value);

		/**
		 * @constructor
		 */
		explicit user_iopool(IoContextPointerContainer&& ios)
		{
			for (auto& ioc : ios)
			{
				this->ios_.emplace_back(ioc);
			}
		}

		/**
		 * @destructor
		 */
		virtual ~user_iopool()
		{
			this->stop();
		}

		/**
		 * @function : run all io_context objects in the pool.
		 */
		virtual bool start() override
		{
			clear_last_error();

			std::lock_guard<std::mutex> guard(this->mutex_);

			if (!this->stopped_)
			{
				set_last_error(asio::error::already_started);
				return true;
			}

			this->stopped_ = false;

			return true;
		}

		/**
		 * @function : stop all io_context objects in the pool
		 */
		virtual void stop() override
		{
			std::lock_guard<std::mutex> guard(this->mutex_);

			if (this->stopped_)
				return;

			this->stopped_ = true;
		}

		/**
		 * @function : check whether the io_context pool is stopped
		 */
		virtual bool stopped() const override
		{
			return (this->stopped_);
		}

		/**
		 * @function : get an io_context to use
		 */
		virtual asio::io_context& get(std::size_t index) override
		{
			// Use a round-robin scheme to choose the next io_context to use. 
			return *(this->ios_[index < this->ios_.size() ? index : ((++(this->next_)) % this->ios_.size())]);
		}

		/**
		 * @function : call user custom callback function for every io_context
		 */
		virtual void for_each(std::function<void(asio::io_context&)> f) override
		{
			std::lock_guard<std::mutex> guard(this->mutex_);

			for (auto& ioc : this->ios_)
			{
				f(*ioc);
			}
		}

		/**
		 * @function : get io_context pool size.
		 */
		virtual std::size_t size() const override
		{
			return this->ios_.size();
		}

	protected:
		/// The io_context pointer container
		/// the io_context pointer maybe "io_context* , std::shared_ptr<io_context> , ..."
		container                      ios_;

		/// 
		std::mutex                     mutex_;

		/// Flag whether the io_context pool has stopped already
		bool                           stopped_  = true;

		/// The next io_context to use for a connection. 
		std::size_t                    next_     = 0;
	};

	class io_t
	{
	public:
		io_t(asio::io_context* ioc) : context_(ioc), strand_(*context_) {}
		~io_t() = default;

		inline asio::io_context         & context() { return (*(this->context_)); }
		inline asio::io_context::strand & strand () { return    this->strand_   ; }

	protected:
		asio::io_context       * context_ = nullptr;
		asio::io_context::strand strand_;
	};

	class iopool_cp
	{
	public:
		template<class Integer, std::enable_if_t<
			std::is_integral_v<detail::remove_cvref_t<Integer>>
			, int> = 0>
		explicit iopool_cp(Integer concurrency)
		{
			this->iopool_ = std::make_unique<default_iopool>(concurrency);

			this->iopool_->for_each([this](asio::io_context& ioc) mutable
			{
				this->iots_.emplace_back(&ioc);
			});
		}

		template<class IoContextPointerContainer, std::enable_if_t<
			!std::is_integral_v<detail::remove_cvref_t<IoContextPointerContainer>> &&
			!is_io_context_pointer<IoContextPointerContainer>::value &&
			!is_io_context_object <IoContextPointerContainer>::value
			, int> = 0>
		explicit iopool_cp(IoContextPointerContainer&& ios)
		{
			ASIO2_ASSERT(!ios.empty() && "The io_context pointer container is empty.");

			this->iopool_ = std::make_unique<user_iopool<IoContextPointerContainer>>(
				std::forward<IoContextPointerContainer>(ios));

			this->iopool_->for_each([this](asio::io_context& ioc) mutable
			{
				this->iots_.emplace_back(&ioc);
			});
		}

		template<class IoContextPointer, std::enable_if_t<
			is_io_context_pointer<IoContextPointer>::value
			, int> = 0>
		explicit iopool_cp(IoContextPointer&& ioc)
		{
			ASIO2_ASSERT(ioc && "The io_context pointer is nullptr.");

			using container = std::vector<detail::remove_cvref_t<IoContextPointer>>;

			container ios{ std::forward<IoContextPointer>(ioc) };

			this->iopool_ = std::make_unique<user_iopool<container>>(std::move(ios));

			this->iopool_->for_each([this](asio::io_context& ioc) mutable
			{
				this->iots_.emplace_back(&ioc);
			});
		}

		template<class IoContextRefrence, std::enable_if_t<
			is_io_context_object<IoContextRefrence>::value
			, int> = 0>
		explicit iopool_cp(IoContextRefrence&& ioc)
		{
			using container = std::vector<std::add_pointer_t<detail::remove_cvref_t<IoContextRefrence>>>;

			container ios{ &ioc };

			this->iopool_ = std::make_unique<user_iopool<container>>(std::move(ios));

			this->iopool_->for_each([this](asio::io_context& ioc) mutable
			{
				this->iots_.emplace_back(&ioc);
			});
		}

		//template<class UserCustomExecutors>
		//explicit iopool_cp(UserCustomExecutors&& executors)
		//{
		//	// This function may be implemented in the future
		//}

		~iopool_cp() = default;

		inline iopool_base& iopool() { return (*(this->iopool_)); }

	protected:
		inline io_t& _get_io(std::size_t index = static_cast<std::size_t>(-1))
		{
			// Use a round-robin scheme to choose the next io_context to use. 
			return this->iots_[index < this->iots_.size() ? index : ((++(this->next_)) % this->iots_.size())];
		}

	protected:
		/// the io_context pool for socket event
		std::unique_ptr<iopool_base> iopool_;

		/// the container for io_context and strand wrapper.
		std::vector<io_t>            iots_;

		/// The next io_context to use for a connection. 
		std::size_t                  next_ = 0;
	};
}

namespace asio2
{
	using iopool = detail::iopool;
}

#endif // !__ASIO2_IOPOOL_HPP__
