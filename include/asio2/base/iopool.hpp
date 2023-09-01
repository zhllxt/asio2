/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
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
#include <atomic>
#include <unordered_set>
#include <map>
#include <functional>

#include <asio2/base/error.hpp>
#include <asio2/base/define.hpp>
#include <asio2/base/log.hpp>

#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/shared_mutex.hpp>

namespace asio2::detail
{
	using io_context_work_guard = asio::executor_work_guard<asio::io_context::executor_type>;

	/* the below sfinae will cause compile error on gcc 7.3
	// unbelievable :
	// the 1 sfinae need use   std::declval<std::decay_t<T>>()
	// the 2 sfinae need use  (std::declval<std::decay_t<T>>())
	// the 3 sfinae need use ((std::declval<std::decay_t<T>>()))

	//-----------------------------------------------------------------------------------

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

	//-----------------------------------------------------------------------------------

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

	//-----------------------------------------------------------------------------------

#if defined(ASIO2_ENABLE_LOG)
	static_assert(is_io_context_pointer<asio::io_context*  >::value);
	static_assert(is_io_context_pointer<asio::io_context*& >::value);
	static_assert(is_io_context_pointer<asio::io_context*&&>::value);
	static_assert(is_io_context_pointer<std::shared_ptr<asio::io_context>  >::value);
	static_assert(is_io_context_pointer<std::shared_ptr<asio::io_context>& >::value);
	static_assert(is_io_context_pointer<std::shared_ptr<asio::io_context>&&>::value);
	static_assert(is_io_context_pointer<std::shared_ptr<asio::io_context>const&>::value);
	static_assert(is_io_context_object<asio::io_context  >::value);
	static_assert(is_io_context_object<asio::io_context& >::value);
	static_assert(is_io_context_object<asio::io_context&&>::value);
#endif
	*/

	//-----------------------------------------------------------------------------------

	class iopool;

	template<class, class> class iopool_cp;

	class io_t
	{
		friend class iopool;
		template<class, class> friend class iopool_cp;

	public:
		io_t(std::shared_ptr<asio::io_context> ioc_ptr) noexcept : context_(std::move(ioc_ptr))
		{
		}
		~io_t() noexcept
		{
		}

		inline asio::io_context                        & context() noexcept { return (*(this->context_))   ; }
		inline std::atomic<std::size_t>                & pending() noexcept { return    this->pending_     ; }
		inline std::unordered_set<asio::steady_timer*> & timers () noexcept { return    this->timers_      ; }

		inline asio::io_context                        const& context() const noexcept { return (*(this->context_))   ; }
		inline std::atomic<std::size_t>                const& pending() const noexcept { return    this->pending_     ; }
		inline std::unordered_set<asio::steady_timer*> const& timers () const noexcept { return    this->timers_      ; }

		template<class Object>
		inline void regobj(Object* p)
		{
			if (!p)
				return;

			// should hold a io_contxt guard to ensure that the unregobj must be called, otherwise 
			// the objects maybe is not empty and the unregobj maybe not be called.
			asio::dispatch(this->context(), [this, p, optr = p->derived().selfptr()]() mutable
			{
				std::size_t k = reinterpret_cast<std::size_t>(p);

				io_context_work_guard iocg(this->context_->get_executor());

				this->objects_[k] = [p, optr = std::move(optr), iocg = std::move(iocg)]() mutable
				{
					detail::ignore_unused(optr, iocg);

					p->stop();
				};
			});
		}

		template<class Object>
		inline void unregobj(Object* p)
		{
			if (!p)
				return;

			// must use post, beacuse the "for each objects_" was called in the iopool.stop,
			// then the object->stop is called in the for each, then the unregobj is called 
			// in the object->stop, if we erase the elem of the objects_ directly at here,
			// it will cause the iterator is invalid when executed at "for each objects_" .
			asio::post(this->context(), [this, p, optr = p->derived().selfptr()]() mutable
			{
				detail::ignore_unused(optr);

				this->objects_.erase(reinterpret_cast<std::size_t>(p));
			});
		}

		/**
		 * @brief
		 */
		inline void cancel()
		{
			// moust read write the timers_ in io_context thread by "post"
			// when code run to here, the io_context maybe stopped already.
			asio::post(this->context(), [this]() mutable
			{
				for (asio::steady_timer* timer : this->timers_)
				{
					// when the timer is canceled, it will erase itself from timers_.
					detail::cancel_timer(*timer);
				}

				for (auto&[ptr, fun] : this->objects_)
				{
					detail::ignore_unused(ptr);
					if (fun)
					{
						fun();
					}
				}

				this->timers_.clear();
				this->objects_.clear();
			});
		}

		/**
		 * @brief initialize the thread id to "std::this_thread::get_id()"
		 */
		inline void init_thread_id() noexcept
		{
			if (this->thread_id_ != std::this_thread::get_id())
			{
				this->thread_id_ = std::this_thread::get_id();
			}
		}

		/**
		 * @brief uninitialize the thread id to empty.
		 */
		inline void fini_thread_id() noexcept
		{
			this->thread_id_ = std::thread::id{};
		}

		/**
		 * @brief return the thread id of the current io_context running in.
		 */
		inline std::thread::id get_thread_id() const noexcept
		{
			return this->thread_id_;
		}

		/**
		 * @brief Determine whether the current io_context is running in the current thread.
		 */
		inline bool running_in_this_thread() const noexcept
		{
			return (std::this_thread::get_id() == this->thread_id_);
		}

	protected:
		// 
		std::shared_ptr<asio::io_context>        context_;

		// the strand will cause some problem when used in dll.
		// 1. when declare a strand in dll, and export it, when use the strand in exe which 
		//    exported by the dll, the strand.running_in_this_thread will false, even if it
		//    is called in the io_context thread.
		// 2. when declare a strand in dll, and export it, when use asio::bind_executor(strand
		//    in exe, it will cause deak lock.
		//    eg: async_connect(endpoint, asio::bind_executor(strand, callback)); the callback
		//        will never be called.
		//asio::io_context::strand                 strand_;

		// Use this variable to ensure async_send function was executed correctly.
		// see : send_cp.hpp "# issue x:"
		std::atomic<std::size_t>                 pending_{};

		// Use this variable to save the timers that have not been closed properly.
		// If we don't do this, the following problem will occurs:
		// user call client.stop, when the code is run to before the iopool's 
		// wait_for_io_context_stopped, and user call client.start_timer at another
		// thread, this will cause the wait_for_io_context_stopped will block forever 
		// until the timer expires.
		// e.g:
		//     {
		//         asio2::timer timer;
		//         timer.post([&]()
		//         {
		//             timer.start_timer(1, std::chrono::seconds(1), []() {});
		//         });
		//     } // the timer's destructor will be called here.
		// when the timer's destructor is called, it will call the "stop_all_timers"
		// function, the "stop_all_timers" will "post a event", this "post a event"
		// will executed before the "timer.start_timer(1,...)", so when the 
		// "timer.start_timer(1,...)" is executed, nobody has a chance to cancel it,
		// and this will cause the iopool's wait_for_io_context_stopped function
		// blocked forever.
		std::unordered_set<asio::steady_timer*>      timers_;

		// Used to save the server or client or other objects, when iopool.stop is called,
		// the objects.stop will be called automaticly.
		std::map<std::size_t, std::function<void()>> objects_;

		// the thread id of the current io_context running in.
		std::thread::id                              thread_id_{};
	};

	//-----------------------------------------------------------------------------------

	template<class T, class R = void>
	struct is_io_t_pointer : std::false_type {};

	template<class T>
	struct is_io_t_pointer<T, std::void_t<decltype(
		((std::declval<std::decay_t<T>>()))->~io_t()), void>> : std::true_type {};

	template<class T, class R = void>
	struct is_io_t_object : std::false_type {};

	template<class T>
	struct is_io_t_object<T, std::void_t<decltype(
		((std::declval<std::decay_t<T>>())).~io_t()), void>> : std::true_type {};

	//-----------------------------------------------------------------------------------

	/**
	 * io_context pool
	 */
	class iopool
	{
		template<class, class> friend class iopool_cp;

		// used fo fix the compile error under vs2017
		template<class R, class P, class F, class T>
		struct post_lambda_1
		{
			std::atomic<std::size_t>& pending;

			P p;
			F f;
			T t;

			template<class X = P, class Y = F, class Z = T>
			explicit post_lambda_1(std::atomic<std::size_t>& pd, X&& x, Y&& y, Z&& z)
				: pending(pd), p(std::forward<X>(x)), f(std::forward<Y>(y)), t(std::forward<Z>(z))
			{
			}

			template<class U = R>
			void operator()()
			{
				if constexpr (std::is_void_v<R>)
				{
					std::apply(std::move(f), std::move(t));

					p.set_value();
				}
				else
				{
					p.set_value(std::apply(std::move(f), std::move(t)));
				}

				pending--;
			}
		};

		// used fo fix the compile error under vs2017
		template<class R, class P, class F, class T>
		struct post_lambda_2
		{
			P p;
			F f;
			T t;

			template<class X = P, class Y = F, class Z = T>
			explicit post_lambda_2(X&& x, Y&& y, Z&& z)
				: p(std::forward<X>(x)), f(std::forward<Y>(y)), t(std::forward<Z>(z))
			{
			}

			template<class U = R>
			void operator()()
			{
				if constexpr (std::is_void_v<U>)
				{
					std::apply(std::move(f), std::move(t));

					p.set_value();
				}
				else
				{
					p.set_value(std::apply(std::move(f), std::move(t)));
				}
			}
		};

	public:
		/**
		 * @brief constructor
		 * @param concurrency - the pool size, default is double the number of CPU cores
		 */
		explicit iopool(std::size_t concurrency = default_concurrency()) : state_(state_t::stopped), next_(0)
		{
			if (concurrency == 0)
			{
				concurrency = default_concurrency();
			}

			for (std::size_t i = 0; i < concurrency; ++i)
			{
				this->iocs_.emplace_back(std::make_shared<asio::io_context>(1));
			}

			for (std::size_t i = 0; i < concurrency; ++i)
			{
				this->iots_.emplace_back(std::make_shared<io_t>(this->iocs_[i]));
			}

			this->threads_.reserve(this->iots_.size());
			this->guards_ .reserve(this->iots_.size());
		}

		/**
		 * @brief destructor
		 */
		~iopool()
		{
			this->stop();

			// only call object's stop function in io_context thread and hasn't call object's 
			// stop function in non io_context thread maybe cause this problem:
			// the io_context do one task, and at this time, the shared ptr object reference
			// counter is 1 in the task,
			// when the task is finished, the shared ptr object will be destroyed, when 
			// it destroyed, the iopool will be destroyed too, but at this time, the iopool
			// stop is running in the io_context thread, so the iopool and the iopool thread
			// will can not stopped, then this caused a crash.
			// so we must ensure that: we must hold a shared ptr object manual, to avoid
			// the shared ptr object destroyed in the io_context thread, so a method is:
			// we call stop in non io_context thread can solve this problem.

		#if defined(ASIO2_ENABLE_LOG)
			if (!this->threads_.empty())
			{
				ASIO2_LOG_FATAL(
					"fatal error: the object is destroyed in the io_context thread. {}",
					this->threads_.size());
			}
		#endif

			// You should call stop function manually by youself to avoid this problem.
			// eg:
			// asio2::tcp_client client;
			// ...
			// client.stop();
			ASIO2_ASSERT(!this->running_in_threads());
			ASIO2_ASSERT(this->threads_.empty());
		}

		/**
		 * @brief run all io_context objects in the pool.
		 */
		bool start()
		{
			clear_last_error();

			// use read lock to check the state, to avoid deadlock.
			{
				asio2::shared_locker guard(this->mutex_);

				if (this->state_ != state_t::stopped)
				{
					set_last_error(asio::error::already_started);
					return true;
				}

				if (!this->guards_.empty() || !this->threads_.empty())
				{
					set_last_error(asio::error::already_started);
					return true;
				}
			}

			// then must use write lock again yet.
			asio2::unique_locker guard(this->mutex_);

			if (this->state_ != state_t::stopped)
			{
				set_last_error(asio::error::already_started);
				return true;
			}

			if (!this->guards_.empty() || !this->threads_.empty())
			{
				set_last_error(asio::error::already_started);
				return true;
			}

			this->state_ = state_t::starting;

			std::vector<std::promise<void>> promises(this->iots_.size());

			// Create a pool of threads to run all of the io_contexts. 
			for (std::size_t i = 0; i < this->iots_.size(); ++i)
			{
				auto& iot = this->iots_[i];
				std::promise<void>& promise = promises[i];

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
				iot->context().restart();

				this->guards_.emplace_back(iot->context().get_executor());

				// start work thread
				this->threads_.emplace_back([this, &iot, &promise]() mutable
				{
					detail::ignore_unused(this);

					iot->thread_id_ = std::this_thread::get_id();

					// after the thread id is seted already, we set the promise
					promise.set_value();

					// should we catch the exception ? 
					// If an exception occurs here, what should we do ?
					// We should handle exceptions in other business functions to ensure that
					// exceptions will not be triggered here.

					// You can define ASIO_NO_EXCEPTIONS in the /asio2/config.hpp to disable the
					// exception. so when the exception occurs, you can check the stack trace.
				#if !defined(ASIO_NO_EXCEPTIONS) && !defined(BOOST_ASIO_NO_EXCEPTIONS)
					try
					{
				#endif
						iot->context().run();
				#if !defined(ASIO_NO_EXCEPTIONS) && !defined(BOOST_ASIO_NO_EXCEPTIONS)
					}
					catch (system_error const& e)
					{
						std::ignore = e;

						ASIO2_LOG_ERROR("fatal exception in io_context::run:1: {}", e.what());

						ASIO2_ASSERT(false);
					}
					catch (std::exception const& e)
					{
						std::ignore = e;

						ASIO2_LOG_ERROR("fatal exception in io_context::run:2: {}", e.what());

						ASIO2_ASSERT(false);
					}
					catch (...)
					{
						ASIO2_LOG_ERROR("fatal exception in io_context::run:3");

						ASIO2_ASSERT(false);
					}
				#endif

					// memory leaks occur when SSL is used in multithreading
					// https://github.com/chriskohlhoff/asio/issues/368
				#if defined(ASIO2_ENABLE_SSL) || defined(ASIO2_USE_SSL)
					OPENSSL_thread_stop();
				#endif
				});
			}

			for (std::size_t i = 0; i < this->iots_.size(); ++i)
			{
				promises[i].get_future().wait();
			}

		#if defined(_DEBUG) || defined(DEBUG)
			for (std::size_t i = 0; i < this->iots_.size(); ++i)
			{
				ASIO2_ASSERT(this->iots_[i]->get_thread_id() == this->threads_[i].get_id());
			}
		#endif

			this->state_ = state_t::started;

			return true;
		}

		/**
		 * @brief stop all io_context objects in the pool
		 * blocking until all posted event has completed already.
		 * After we call iog.reset(), when an asio::post(io_context,...) execution ends, the count
		 * of the io_context will be checked. If the count equals 0, the io_context will be closed. Then 
		 * the subsequent call of asio:: post(io_context,...) will fail, and the post event will not
		 * be executed. When our program exits, it will nest call asio:: post (io_context...) to post
		 * many events, so when an asio::post(io_context,...) inside someone asio::post(io_context,...)
		 * has not yet been executed, the io_context may have been closed, which will result in the
		 * nested asio::post(io_context,...) never being executed.
		 */
		void stop()
		{
			// split read and write to avoid deadlock caused by iopool.post([&iopool]() {iopool.stop(); });
			{
				asio2::shared_locker guard(this->mutex_);

				if (this->state_ != state_t::started)
					return;

				if (this->guards_.empty() && this->threads_.empty())
					return;

				if (this->running_in_threads_impl())
					return this->cancel_impl();
			}

			{
				asio2::unique_locker guard(this->mutex_);

				if (this->state_ != state_t::started)
					return;

				this->state_ = state_t::stopping;
			}

			// Waiting for all nested events to complete.
			// The mutex_ must be released while waiting, otherwise, the stop function may be called
			// in the communication thread and the lock will be requested, which is already held here,
			// so leading to deadlock.
			this->wait_for_io_context_stopped();

			{
				asio2::unique_locker guard(this->mutex_);

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

				this->guards_ .clear();
				this->threads_.clear();

			#if defined(_DEBUG) || defined(DEBUG)
				for (std::size_t i = 0; i < this->iots_.size(); ++i)
				{
					ASIO2_ASSERT(this->iots_[i]->objects_.empty());
				}
			#endif

				this->state_ = state_t::stopped;
			}
		}

		/**
		 * @brief check whether the io_context pool is started
		 */
		inline bool started() const noexcept
		{
			asio2::shared_locker guard(this->mutex_);

			return (this->state_ == state_t::started);
		}

		/**
		 * @brief check whether the io_context pool is stopped
		 */
		inline bool stopped() const noexcept
		{
			asio2::shared_locker guard(this->mutex_);

			return (this->state_ == state_t::stopped);
		}

		/**
		 * @brief get an io_t to use
		 */
		inline std::shared_ptr<io_t> get(std::size_t index = static_cast<std::size_t>(-1)) noexcept
		{
			asio2::shared_locker guard(this->mutex_);

			ASIO2_ASSERT(!this->iots_.empty());

			return this->iots_[this->next_impl(index)];
		}

		/**
		 * @brief get an io_context to use
		 */
		inline asio::io_context& get_context(std::size_t index = static_cast<std::size_t>(-1)) noexcept
		{
			asio2::shared_locker guard(this->mutex_);

			ASIO2_ASSERT(!this->iots_.empty());

			return this->iots_[this->next_impl(index)]->context();
		}

		/**
		 * @brief get an io_context shared_ptr to use
		 */
		inline std::shared_ptr<asio::io_context> get_context_ptr(std::size_t index = std::size_t(-1)) noexcept
		{
			asio2::shared_locker guard(this->mutex_);

			ASIO2_ASSERT(!this->iocs_.empty());

			return this->iocs_[this->next_impl(index)];
		}

		/**
		 * @brief Determine whether current code is running in the io_context pool threads.
		 */
		inline bool running_in_threads() const noexcept
		{
			asio2::shared_locker guard(this->mutex_);

			return this->running_in_threads_impl();
		}

		/**
		 * @brief Determine whether current code is running in the io_context thread by index
		 */
		inline bool running_in_thread(std::size_t index) const noexcept
		{
			asio2::shared_locker guard(this->mutex_);

			ASIO2_ASSERT(index < this->threads_.size());

			if (!(index < this->threads_.size()))
				return false;

			return (std::this_thread::get_id() == this->threads_[index].get_id());
		}

		/**
		 * @brief get io_context pool size.
		 */
		inline std::size_t size() const noexcept
		{
			asio2::shared_locker guard(this->mutex_);

			return this->iots_.size();
		}

		/**
		 * @brief Get the thread id of the specified thread index.
		 */
		inline std::thread::id get_thread_id(std::size_t index) const noexcept
		{
			asio2::shared_locker guard(this->mutex_);

			return this->threads_[index % this->threads_.size()].get_id();
		}

		/**
		 * @brief Get the thread native handle of the specified thread index.
		 * @note after test, on Windows:
		 * this will be failed:
		 * SetThreadPriority((HANDLE)thread.native_handle(), THREAD_PRIORITY_HIGHEST);
		 * this will be successed:
		 * SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
		 */
		inline std::thread::native_handle_type get_thread_handle(std::size_t index) noexcept
		{
			asio2::shared_locker guard(this->mutex_);

			return this->threads_[index % this->threads_.size()].native_handle();
		}

		/**
		 * Use to ensure that all nested asio::post(...) events are fully invoked.
		 */
		inline void wait_for_io_context_stopped() ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			// split read and write to avoid deadlock caused by iopool.post([&iopool]() {iopool.stop(); });
			{
				//asio2::shared_locker guard(this->mutex_);

				if (this->running_in_threads_impl())
					return this->cancel_impl();

				// wiat fo all pending events completed.
				for (auto& iot : this->iots_)
				{
					while (iot->pending() > std::size_t(0))
						std::this_thread::sleep_for(std::chrono::milliseconds(0));
				}
			}

			{
				asio2::unique_locker guard(this->mutex_);

				// first reset the acceptor io_context work guard
				if (!this->guards_.empty())
					this->guards_.front().reset();
			}

			constexpr auto max = std::chrono::milliseconds(10);
			constexpr auto min = std::chrono::milliseconds(1);

			{
				// don't need lock, maybe cause deadlock in client start iopool
				//asio2::shared_locker guard(this->mutex_);

				// second wait indefinitely until the acceptor io_context is stopped
				for (std::size_t i = 0; i < std::size_t(1) && i < this->iocs_.size(); ++i)
				{
					auto t1 = std::chrono::steady_clock::now();
					auto& ioc = this->iocs_[i];
					auto& iot = this->iots_[i];
					while (!ioc->stopped())
					{
						// the timer may not be canceled successed when using visual
						// studio break point for debugging, so cancel it at each loop

						// must cancel all iots, otherwise maybe cause deaklock like below:
						// the client_ptr->bind_recv has hold the session_ptr, and the session_ptr
						// is in the indexed 1 iot ( not indexed 0 iot ), so if call iot->cancel,
						// the cancel function of indexed 1 iot wont be called, so the stop function
						// of client_ptr won't be called too, so the session_ptr which holded by the
						// client_ptr will can't be destroyed, so the server's acceptor io will 
						// can't be stopped(this means the indexed 0 io can't be stopped).

						//server.bind_accept([](std::shared_ptr<asio2::tcp_session>& session_ptr)
						//{
						//	std::shared_ptr<asio2::tcp_client> client_ptr = std::make_shared<asio2::tcp_client>(
						//		512, 1024, session_ptr->io());
						//
						//	client_ptr->bind_recv([session_ptr](std::string_view data) mutable
						//	{
						//	});
						//
						//	client_ptr->async_start("127.0.0.1", 8888);
						//});

						this->cancel_impl();

						auto t2 = std::chrono::steady_clock::now();
						auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
						std::this_thread::sleep_for(std::clamp(ms, min, max));
					}
					iot->thread_id_ = std::thread::id{};
					ASIO2_ASSERT(iot->timers().empty());
					ASIO2_ASSERT(iot->objects_.empty());
				}
			}

			{
				asio2::unique_locker guard(this->mutex_);

				for (std::size_t i = 1; i < this->guards_.size(); ++i)
				{
					this->guards_[i].reset();
				}
			}

			{
				// don't need lock, maybe cause deadlock in client start iopool
				//asio2::shared_locker guard(this->mutex_);

				for (std::size_t i = 1; i < this->iocs_.size(); ++i)
				{
					auto t1 = std::chrono::steady_clock::now();
					auto& ioc = this->iocs_[i];
					auto& iot = this->iots_[i];
					while (!ioc->stopped())
					{
						// the timer may not be canceled successed when using visual
						// studio break point for debugging, so cancel it at each loop
						this->cancel_impl();

						auto t2 = std::chrono::steady_clock::now();
						auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
						std::this_thread::sleep_for(std::clamp(ms, min, max));
					}
					iot->thread_id_ = std::thread::id{};
					ASIO2_ASSERT(iot->timers().empty());
					ASIO2_ASSERT(iot->objects_.empty());
				}
			}
		}

		/**
		 * 
		 */
		inline void cancel()
		{
			asio2::shared_locker guard(this->mutex_);

			return this->cancel_impl();
		}

		/**
		 * @brief destroy the content of all member variables, this is used for solve the memory leaks.
		 * After this function is called, this class object cannot be used again.
		 */
		inline void destroy() noexcept
		{
			asio2::unique_locker guard(this->mutex_);

			this->threads_.clear();
			this->iocs_.clear();
			this->iots_.clear();
			this->guards_.clear();

		#if defined(_DEBUG) || defined(DEBUG)
			this->derive_pointer_ = []() {};
		#endif
		}

		/**
		 * @brief
		 */
		inline std::size_t next(std::size_t index) noexcept
		{
			asio2::shared_locker guard(this->mutex_);

			return this->next_impl(index);
		}

		/**
		 * The wait_for() function blocks until the specified duration has elapsed.
		 *
		 * @param rel_time - The duration for which the call may block.
		 */
		template <typename Rep, typename Period>
		void wait_for(const std::chrono::duration<Rep, Period>& rel_time)
		{
			if (this->running_in_threads())
			{
				set_last_error(asio::error::operation_not_supported);
				return;
			}

			clear_last_error();

			io_t* iot = nullptr;

			{
				asio2::shared_locker guard(this->mutex_);
				iot = this->iots_[0].get();
			}

			asio::steady_timer timer(iot->context());
			timer.expires_after(rel_time);
			timer.wait(get_last_error());
		}

		/**
		 * The wait_until() function blocks until the specified time has been reached.
		 *
		 * @param abs_time - The time point until which the call may block.
		 */
		template <typename Clock, typename Duration>
		void wait_until(const std::chrono::time_point<Clock, Duration>& abs_time)
		{
			if (this->running_in_threads())
			{
				set_last_error(asio::error::operation_not_supported);
				return;
			}

			clear_last_error();

			io_t* iot = nullptr;

			{
				asio2::shared_locker guard(this->mutex_);
				iot = this->iots_[0].get();
			}

			asio::steady_timer timer(iot->context());
			timer.expires_at(abs_time);
			timer.wait(get_last_error());
		}

		/**
		 * The wait_signal() function blocks util some signal delivered.
		 * 
		 * @return The delivered signal number. Maybe invalid value when some exception occured.
		 */
		template <class... Ints>
		int wait_signal(Ints... signal_number)
		{
			if (this->running_in_threads())
			{
				set_last_error(asio::error::operation_not_supported);
				return 0;
			}

			clear_last_error();

			io_t* iot = nullptr;

			{
				asio2::shared_locker guard(this->mutex_);
				iot = this->iots_[0].get();
			}

			// note: The variable name signals will conflict with the macro signals of qt
			asio::signal_set signalset(iot->context());

			(signalset.add(signal_number), ...);

			std::promise<int> promise;
			std::future<int> future = promise.get_future();

			signalset.async_wait([&](const error_code& /*ec*/, int signo)
			{
				promise.set_value(signo);
			});

			return future.get();
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
			asio2::shared_locker guard(this->mutex_);

			using return_type = std::invoke_result_t<Fun, Args...>;

			std::size_t index = 0, num = (std::numeric_limits<std::size_t>::max)();

			for (std::size_t i = 0, n = this->iots_.size(); i < n; ++i)
			{
				std::size_t pending = this->iots_[i]->pending().load();

				if (pending == 0)
				{
					index = i;
					break;
				}

				if (pending < num)
				{
					num = pending;
					index = i;
				}
			}

			std::atomic<std::size_t>& pending = this->iots_[index]->pending();

			pending++;

			std::promise<return_type> promise;
			std::future<return_type> future = promise.get_future();

			using lambda_t = post_lambda_1<
				return_type,
				std::promise<return_type>,
				detail::remove_cvref_t<Fun>,
				std::tuple<detail::remove_cvref_t<Args>...>>;

			asio::post(*(this->iocs_[index]), lambda_t
			{
				pending,
				std::move(promise),
				std::forward<Fun>(fun),
				std::tuple(std::forward<Args>(args) ...)
			});

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
			asio2::shared_locker guard(this->mutex_);

			using return_type = std::invoke_result_t<Fun, Args...>;

			std::promise<return_type> promise;
			std::future<return_type> future = promise.get_future();

			using lambda_t = post_lambda_2<
				return_type,
				std::promise<return_type>,
				detail::remove_cvref_t<Fun>,
				std::tuple<detail::remove_cvref_t<Args>...>>;

			asio::post(*(this->iocs_[thread_index % this->iocs_.size()]), lambda_t
			{
				std::move(promise),
				std::forward<Fun>(fun),
				std::tuple(std::forward<Args>(args) ...)
			});

			return future;
		}

	protected:
		inline bool running_in_threads_impl() const noexcept ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			std::thread::id curr_tid = std::this_thread::get_id();

			for (auto& thread : this->threads_)
			{
				if (curr_tid == thread.get_id())
					return true;
			}

			return false;
		}

		inline void cancel_impl() ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			for (std::size_t i = 0; i < this->iocs_.size(); ++i)
			{
				auto& ioc = this->iocs_[i];
				auto& iot = this->iots_[i];
				if (!ioc->stopped())
				{
					iot->cancel();
				}
			}
		}

		inline std::size_t next_impl(std::size_t index) noexcept ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			// Use a round-robin scheme to choose the next io_context to use. 
			return (index == static_cast<std::size_t>(-1) ?
				((++(this->next_)) % this->iots_.size()) : (index % this->iots_.size()));
		}

	protected:
		/// 
		mutable asio2::shared_mutexer                                mutex_;

		/// threads to run all of the io_context
		std::vector<std::thread>                                     threads_ ASIO2_GUARDED_BY(mutex_);

		/// The pool of io_context. 
		std::vector<std::shared_ptr<asio::io_context>>               iocs_    ASIO2_GUARDED_BY(mutex_);

		/// The pool of io_context. 
		std::vector<std::shared_ptr<io_t>>                           iots_    ASIO2_GUARDED_BY(mutex_);

		/// Flag whether the io_context pool has stopped already
		detail::state_t                                              state_   ASIO2_GUARDED_BY(mutex_);

		/// The next io_context to use for a connection. 
		std::size_t                                                  next_;

		// Give all the io_contexts executor_work_guard to do so that their run() functions will not 
		// exit until they are explicitly stopped. 
		std::vector<io_context_work_guard>                           guards_ ASIO2_GUARDED_BY(mutex_);

		// for debug, to see the derived object details.
	#if defined(_DEBUG) || defined(DEBUG)
		std::function<void()>                                        derive_pointer_;
	#endif
	};

	class iopool_base
	{
	public:
		iopool_base() = default;
		virtual ~iopool_base() {}

		virtual bool                        start  ()                           = 0;
		virtual void                        stop   ()                           = 0;
		virtual bool                        started()                  noexcept = 0;
		virtual bool                        stopped()                  noexcept = 0;
		virtual void                        destroy()                  noexcept = 0;
		virtual std::shared_ptr<io_t>       get    (std::size_t index) noexcept = 0;
		virtual std::size_t                 size   ()                  noexcept = 0;
		virtual bool             running_in_threads()                  noexcept = 0;
	};

	class default_iopool : public iopool_base
	{
		template<class, class> friend class iopool_cp;

	public:
		explicit default_iopool(std::size_t concurrency) : impl_(concurrency)
		{
		}

		/**
		 * @brief destructor
		 */
		virtual ~default_iopool()
		{
			this->impl_.stop();
		}

		/**
		 * @brief run all io_context objects in the pool.
		 */
		virtual bool start() override
		{
			return this->impl_.start();
		}

		/**
		 * @brief stop all io_context objects in the pool
		 */
		virtual void stop() override
		{
			this->impl_.stop();
		}

		/**
		 * @brief check whether the io_context pool is started
		 */
		virtual bool started() noexcept override
		{
			return this->impl_.started();
		}

		/**
		 * @brief check whether the io_context pool is stopped
		 */
		virtual bool stopped() noexcept override
		{
			return this->impl_.stopped();
		}

		/**
		 * @brief destroy the content of all member variables, this is used for solve the memory leaks.
		 * After this function is called, this class object cannot be used again.
		 */
		virtual void destroy() noexcept override
		{
			return this->impl_.destroy();
		}

		/**
		 * @brief get an io_t to use
		 */
		virtual std::shared_ptr<io_t> get(std::size_t index) noexcept override
		{
			return this->impl_.get(index);
		}

		/**
		 * @brief get io_context pool size.
		 */
		virtual std::size_t size() noexcept override
		{
			return this->impl_.size();
		}

		/**
		 * @brief Determine whether current code is running in the io_context pool threads.
		 */
		virtual bool running_in_threads() noexcept override
		{
			return this->impl_.running_in_threads();
		}

	protected:
		detail::iopool impl_;
	};

	/**
	 * This io_context pool is passed in by the user
	 */
	class user_iopool : public iopool_base
	{
		template<class, class> friend class iopool_cp;

	public:
		/**
		 * @brief constructor
		 */
		explicit user_iopool(std::vector<std::shared_ptr<io_t>> iots) : iots_(std::move(iots)), stopped_(true), next_(0)
		{
		}

		/**
		 * @brief destructor
		 */
		virtual ~user_iopool()
		{
			this->stop();
		}

		/**
		 * @brief run all io_context objects in the pool.
		 */
		virtual bool start() override
		{
			clear_last_error();

			asio2::unique_locker guard(this->mutex_);

			if (!this->stopped_)
			{
				set_last_error(asio::error::already_started);
				return true;
			}

			this->stopped_ = false;

			return true;
		}

		/**
		 * @brief stop all io_context objects in the pool
		 */
		virtual void stop() override
		{
			{
				asio2::shared_locker guard(this->mutex_);

				if (this->stopped_)
					return;

				// wiat fo all pending events completed.
				for (auto& iot : this->iots_)
				{
					while (iot->pending() > std::size_t(0))
						std::this_thread::sleep_for(std::chrono::milliseconds(0));
				}
			}

			{
				asio2::unique_locker guard(this->mutex_);

				if (this->stopped_)
					return;

				this->stopped_ = true;
			}
		}

		/**
		 * @brief check whether the io_context pool is started
		 */
		virtual bool started() noexcept override
		{
			asio2::shared_locker guard(this->mutex_);

			return (!this->stopped_);
		}

		/**
		 * @brief check whether the io_context pool is stopped
		 */
		virtual bool stopped() noexcept override
		{
			asio2::shared_locker guard(this->mutex_);

			return (this->stopped_);
		}

		/**
		 * @brief destroy the content of all member variables, this is used for solve the memory leaks.
		 * After this function is called, this class object cannot be used again.
		 */
		virtual void destroy() noexcept override
		{
			asio2::unique_locker guard(this->mutex_);

			this->iots_.clear();
		}

		/**
		 * @brief get an io_t to use
		 */
		virtual std::shared_ptr<io_t> get(std::size_t index) noexcept override
		{
			asio2::shared_locker guard(this->mutex_);

			return this->iots_[this->next_impl(index)];
		}

		/**
		 * @brief get io_context pool size.
		 */
		virtual std::size_t size() noexcept override
		{
			asio2::shared_locker guard(this->mutex_);

			return this->iots_.size();
		}

		/**
		 * @brief
		 */
		inline std::size_t next(std::size_t index) noexcept
		{
			asio2::shared_locker guard(this->mutex_);

			return this->next_impl(index);
		}

		/**
		 * @brief Determine whether current code is running in the io_context pool threads.
		 */
		virtual bool running_in_threads() noexcept override
		{
			asio2::shared_locker guard(this->mutex_);

			return this->running_in_threads_impl();
		}

	protected:
		inline bool running_in_threads_impl() noexcept ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			std::thread::id curr_tid = std::this_thread::get_id();

			for (auto& iot : this->iots_)
			{
				if (curr_tid == iot->get_thread_id())
					return true;
			}

			return false;
		}

		inline std::size_t next_impl(std::size_t index) noexcept ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			// Use a round-robin scheme to choose the next io_context to use. 
			// Use this->iots_.size() instead of this->size() to avoid call virtual function.
			return (index == static_cast<std::size_t>(-1) ?
				((++(this->next_)) % this->iots_.size()) : (index % this->iots_.size()));
		}

	protected:
		/// 
		mutable asio2::shared_mutexer            mutex_;

		/// The pool of io_t. 
		std::vector<std::shared_ptr<io_t>>       iots_    ASIO2_GUARDED_BY(mutex_);

		/// Flag whether the io_context pool has stopped already
		bool                                     stopped_ ASIO2_GUARDED_BY(mutex_);

		/// The next io_context to use for a connection. 
		std::size_t                              next_;
	};

	template<class derived_t, class args_t = void>
	class iopool_cp
	{
	public:
		template<class T>
		explicit iopool_cp(T&& v) : next_(0)
		{
			using type = typename detail::remove_cvref_t<T>;

			if /**/ constexpr (std::is_integral_v<type>)
			{
				this->iopool_ = std::make_unique<default_iopool>(v);

			#if defined(_DEBUG) || defined(DEBUG)
				derived_t& derive = static_cast<derived_t&>(*this);
				static_cast<default_iopool*>(this->iopool_.get())->impl_.derive_pointer_ =
					[&derive]() { detail::ignore_unused(derive); };
			#endif
			}
			else
			{
				this->iopool_ = std::make_unique<user_iopool>(this->to_iots(std::forward<T>(v)));
			}

			for (std::size_t i = 0, size = iopool_->size(); i < size; ++i)
			{
				iots_.emplace_back(iopool_->get(i));
			}
		}

		~iopool_cp() = default;

		/**
		 * The wait_stop() function blocks until the stop() function has been called.
		 */
		void wait_stop()
		{
			if (this->iopool().running_in_threads())
			{
				set_last_error(asio::error::operation_not_supported);
				return;
			}

			clear_last_error();

			derived_t& derive = static_cast<derived_t&>(*this);

			std::promise<error_code> promise;
			std::future<error_code> future = promise.get_future();

			std::shared_ptr<io_t>& iot = this->iots_[0];

			// We must use asio::post to ensure the wait_stop_timer_ is read write in the 
			// same thread.
			asio::post(iot->context(), [this, iot, this_ptr = derive.selfptr(), promise = std::move(promise)]
			() mutable
			{
				if (this->wait_stop_timer_)
				{
					iot->timers().erase(this->wait_stop_timer_.get());

					detail::cancel_timer(*(this->wait_stop_timer_));
				}

				this->wait_stop_timer_ = std::make_unique<asio::steady_timer>(iot->context());

				iot->timers().emplace(this->wait_stop_timer_.get());

				this->wait_stop_timer_->expires_after((std::chrono::nanoseconds::max)());
				this->wait_stop_timer_->async_wait(
				[this_ptr = std::move(this_ptr), promise = std::move(promise)]
				(const error_code&) mutable
				{
					detail::ignore_unused(this_ptr);

					promise.set_value(error_code{});
				});
			});

			set_last_error(future.get());
		}

		/**
		 * The wait_for() function blocks until the specified duration has elapsed.
		 *
		 * @param rel_time - The duration for which the call may block.
		 */
		template <typename Rep, typename Period>
		void wait_for(const std::chrono::duration<Rep, Period>& rel_time)
		{
			if (this->iopool().running_in_threads())
			{
				set_last_error(asio::error::operation_not_supported);
				return;
			}

			clear_last_error();

			asio::steady_timer timer(this->iots_[0]->context());
			timer.expires_after(rel_time);
			timer.wait(get_last_error());
		}

		/**
		 * The wait_until() function blocks until the specified time has been reached.
		 *
		 * @param abs_time - The time point until which the call may block.
		 */
		template <typename Clock, typename Duration>
		void wait_until(const std::chrono::time_point<Clock, Duration>& abs_time)
		{
			if (this->iopool().running_in_threads())
			{
				set_last_error(asio::error::operation_not_supported);
				return;
			}

			clear_last_error();

			asio::steady_timer timer(this->iots_[0]->context());
			timer.expires_at(abs_time);
			timer.wait(get_last_error());
		}

		/**
		 * The wait_signal() function blocks util some signal delivered.
		 * 
		 * @return The delivered signal number. Maybe invalid value when some exception occured.
		 */
		template <class... Ints>
		int wait_signal(Ints... signal_number)
		{
			if (this->iopool().running_in_threads())
			{
				set_last_error(asio::error::operation_not_supported);
				return 0;
			}

			clear_last_error();

			// note: The variable name signals will conflict with the macro signals of qt
			asio::signal_set signalset(this->iots_[0]->context());

			(signalset.add(signal_number), ...);

			std::promise<int> promise;
			std::future<int> future = promise.get_future();

			signalset.async_wait([&](const error_code& /*ec*/, int signo)
			{
				promise.set_value(signo);
			});

			return future.get();
		}

		/**
		 * Get the iopool_base interface reference.
		 */
		inline iopool_base& iopool() noexcept { return (*(this->iopool_)); }

		/**
		 * Get the iopool_base interface reference.
		 */
		inline iopool_base const& iopool() const noexcept { return (*(this->iopool_)); }

	protected:
		inline std::shared_ptr<io_t> _get_io(std::size_t index = static_cast<std::size_t>(-1)) noexcept
		{
			ASIO2_ASSERT(!iots_.empty());
			std::size_t n = (index == static_cast<std::size_t>(-1) ?
				((++(this->next_)) % this->iots_.size()) : (index % this->iots_.size()));
			return this->iots_[n];
		}

		inline bool is_iopool_started() const noexcept
		{
			return this->iopool_->started();
		}

		inline bool is_iopool_stopped() const noexcept
		{
			return this->iopool_->stopped();
		}

		inline bool start_iopool()
		{
			bool ret = this->iopool_->start();

			// if the io_context is customed that passed by the user, then when the server
			// accepted a new session, then the session's fire init will be called, but at
			// this time, the session io_t's thread id is not inited, if use call the thread
			// id function in the fire init callback, it will be failed, so we do all ios
			// init thread at here first.
			if (ret)
			{
				for (std::shared_ptr<io_t>& iot : this->iots_)
				{
					asio::dispatch(iot->context(), [iot]() mutable
					{
						iot->init_thread_id();
					});
				}
			}

			return ret;
		}

		inline void stop_iopool()
		{
			if (this->is_iopool_stopped())
				return;

			derived_t& derive = static_cast<derived_t&>(*this);

			std::shared_ptr<io_t>& iot = this->iots_[0];

			// if the server's or client's iopool is user_iopool, and when the server.stop 
			// or client.stop is called, we need notify the timer to cancel for the function
			// wait_stop, otherwise the wait_stop function will blocked forever.
			// We must use asio::post to ensure the wait_stop_timer_ is read write in the 
			// same thread.
			asio::post(iot->context(), [this, iot, this_ptr = derive.selfptr()]() mutable
			{
				detail::ignore_unused(this_ptr);

				if (this->wait_stop_timer_)
				{
					iot->timers().erase(this->wait_stop_timer_.get());

					detail::cancel_timer(*(this->wait_stop_timer_));
				}
			});

			this->iopool_->stop();
		}

		/**
		 * @brief destroy the content of all member variables, this is used for solve the memory leaks.
		 * After this function is called, this class object cannot be used again.
		 */
		void destroy_iopool() noexcept
		{
			this->iots_.clear();
			this->wait_stop_timer_.reset();

			this->iopool_->destroy();

			// cant reset the iopool pointer to nullptr, beacuse the derived class destructor will call
			// the iopool.is_stopped() functions.
			//this->iopool_.reset();
		}

	protected:
		template<class T>
		std::vector<std::shared_ptr<io_t>> to_iots(T&& v)
		{
			using type = typename detail::remove_cvref_t<T>;

			std::vector<std::shared_ptr<io_t>> iots{};

			if constexpr (std::is_same_v<type, detail::iopool>)
			{
				ASIO2_ASSERT(v.size() && "The iopool is empty.");

				for (std::size_t i = 0; i < v.size(); ++i)
				{
					iots.emplace_back(v.get(i));
				}
			}
			else if constexpr (std::is_same_v<type, std::shared_ptr<asio::io_context>>)
			{
				ASIO2_ASSERT(v && "The io_context pointer is nullptr.");

				iots.emplace_back(std::make_shared<io_t>(std::forward<T>(v)));
			}
			else if constexpr (std::is_same_v<type, asio::io_context*>)
			{
				ASIO2_ASSERT(v && "The io_context pointer is nullptr.");

				std::shared_ptr<asio::io_context> iop(v, [](asio::io_context*) {});

				iots.emplace_back(std::make_shared<io_t>(std::move(iop)));
			}
			else if constexpr (std::is_same_v<type, asio::io_context>)
			{
				static_assert(std::is_reference_v<std::remove_cv_t<T>>);

				std::shared_ptr<asio::io_context> iop(std::addressof(v), [](asio::io_context*) {});

				iots.emplace_back(std::make_shared<io_t>(std::move(iop)));
			}
			else if constexpr (std::is_same_v<type, std::shared_ptr<io_t>>)
			{
				ASIO2_ASSERT(v && "The io_t pointer is nullptr.");

				iots.emplace_back(std::forward<T>(v));
			}
			else if constexpr (std::is_same_v<type, io_t*>)
			{
				ASIO2_ASSERT(v && "The io_t pointer is nullptr.");

				iots.emplace_back(std::shared_ptr<io_t>(v, [](io_t*) {}));
			}
			else if constexpr (std::is_same_v<type, io_t>)
			{
				static_assert(std::is_reference_v<std::remove_cv_t<T>>);

				iots.emplace_back(std::shared_ptr<io_t>(std::addressof(v), [](io_t*) {}));
			}
			else
			{
				// std::vector<...> std::list<...>

				ASIO2_ASSERT(!v.empty() && "The container is empty.");

				for (auto& e : v)
				{
					std::vector<std::shared_ptr<io_t>> tmps = this->to_iots(e);

					for (std::shared_ptr<io_t>& iot : tmps)
					{
						iots.emplace_back(std::move(iot));
					}
				}
			}

			return iots;
		}

	protected:
		/// the io_context pool for socket event
		std::unique_ptr<iopool_base>                       iopool_;

		/// Use a copy to avoid calling the virtual function "iopool_base::get"
		std::vector<std::shared_ptr<io_t>>                 iots_;

		/// The next io_context to use for a connection. 
		std::size_t                                        next_;

		/// the timer used for wait_stop function.
		std::unique_ptr<asio::steady_timer>                wait_stop_timer_;
	};
}

namespace asio2
{
	using io_t   = detail::io_t;
	using iopool = detail::iopool;
}

#endif // !__ASIO2_IOPOOL_HPP__
