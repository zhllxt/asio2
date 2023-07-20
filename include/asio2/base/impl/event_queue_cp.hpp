/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_EVENT_QUEUE_COMPONENT_HPP__
#define __ASIO2_EVENT_QUEUE_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>
#include <memory>
#include <functional>
#include <string>
#include <future>
#include <queue>
#include <tuple>
#include <utility>
#include <string_view>

#include <asio2/base/iopool.hpp>

#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/function_traits.hpp>
#include <asio2/base/detail/buffer_wrap.hpp>
#include <asio2/base/detail/function.hpp>

namespace asio2::detail
{
	template <class, class>                      class event_queue_cp;
	template <class...    >                      class defer_event;

	template<class derived_t>
	class event_queue_guard
	{
		template <class, class>           friend class event_queue_cp;
		template <class...    >           friend class defer_event;

	public:
		explicit event_queue_guard() noexcept
		{
		}
		explicit event_queue_guard(std::nullptr_t) noexcept
		{
		}

	protected:
		// the valid guard can only be created by event_queue_cp
		explicit event_queue_guard(derived_t& d) noexcept
			: derive(std::addressof(d)), derive_ptr_(d.selfptr()), valid_(true)
		{
		}

	public:
		inline event_queue_guard(event_queue_guard&& o) noexcept
			: derive(o.derive), derive_ptr_(std::move(o.derive_ptr_)), valid_(o.valid_)
		{
			o.valid_ = false;
		}

		event_queue_guard(const event_queue_guard&) = delete;
		event_queue_guard& operator=(const event_queue_guard&) = delete;
		event_queue_guard& operator=(event_queue_guard&&) = delete;

		~event_queue_guard() noexcept
		{
			if (this->valid_)
			{
				derive->next_event(std::move(*this));

				ASIO2_ASSERT(this->valid_ == false);
			}
		}

		inline bool    empty() const noexcept { return (!valid_); }
		inline bool is_empty() const noexcept { return (!valid_); }

	protected:
		derived_t                    * derive = nullptr;

		// must hold the derived object, maybe empty in client
		// if didn't hold the derived object, when the callback is executed in the event queue,
		// the derived object which holded by the callback maybe destroyed by std::move(), when
		// event_queue_guard is destroyed, and will call derive.next_event; the "derive" maybe
		// invalid already.
		std::shared_ptr<derived_t>     derive_ptr_;

		// whether the guard is valid, when object is moved by std::move the guard will be invalid
		bool                           valid_ = false;
	};


	template<class Function>
	class [[maybe_unused]] defer_event<Function>
	{
		template <class...> friend class defer_event;
	public:
		template<class Fn>
		defer_event(Fn&& fn) noexcept
			: fn_(std::forward<Fn>(fn)), valid_(true)
		{
		}

		inline defer_event(defer_event&& o) noexcept
			: fn_(std::move(o.fn_)), valid_(o.valid_)
		{
			o.valid_ = false;
		};

		defer_event(const defer_event&) = delete;
		defer_event& operator=(const defer_event&) = delete;
		defer_event& operator=(defer_event&&) = delete;

		~defer_event() noexcept
		{
			if (valid_)
			{
				valid_ = false;
				(fn_)();
			}
		}

		inline bool    empty() const noexcept { return (!valid_); }
		inline bool is_empty() const noexcept { return (!valid_); }

		inline constexpr bool is_event_queue_guard_empty() const noexcept { return true; }

	protected:
		Function fn_;

		bool     valid_ = false;
	};

	template<>
	class [[maybe_unused]] defer_event<void>
	{
		template <class...> friend class defer_event;
	public:
		defer_event() noexcept {}
		defer_event(std::nullptr_t) noexcept {}

		defer_event(defer_event&&) noexcept = default;
		defer_event& operator=(defer_event&&) noexcept = default;

		defer_event(const defer_event&) = delete;
		defer_event& operator=(const defer_event&) = delete;

		inline constexpr bool    empty() const noexcept { return true; }
		inline constexpr bool is_empty() const noexcept { return true; }

		inline constexpr bool is_event_queue_guard_empty() const noexcept { return true; }
	};

	// defer event with event queue guard dummy
	template<class derived_t>
	struct defer_eqg_dummy
	{
		inline void operator()(event_queue_guard<derived_t>) {}
	};

	template<class Function, class derived_t>
	class [[maybe_unused]] defer_event<Function, derived_t, std::false_type>
	{
		template <class...> friend class defer_event;
	public:
		template<class Fn>
		defer_event(Fn&& fn, std::nullptr_t) noexcept
			: fn_(std::forward<Fn>(fn))
			, valid_(true)
		{
		}

		inline defer_event(defer_event&& o) noexcept
			: fn_(std::move(o.fn_)), valid_(o.valid_)
		{
			o.valid_ = false;
		};

		defer_event(const defer_event&) = delete;
		defer_event& operator=(const defer_event&) = delete;
		defer_event& operator=(defer_event&&) = delete;

		~defer_event() noexcept
		{
			if (valid_)
			{
				valid_ = false;
				(fn_)(event_queue_guard<derived_t>());
			}
		}

		inline bool    empty() const noexcept { return (!valid_); }
		inline bool is_empty() const noexcept { return (!valid_); }

		inline constexpr bool is_event_queue_guard_empty() const noexcept { return true; }

		inline defer_event<Function, derived_t, std::false_type> move_event() noexcept
		{
			return std::move(*this);
		}
		inline event_queue_guard<derived_t> move_guard() noexcept
		{
			return event_queue_guard<derived_t>();
		}

	protected:
		Function fn_;

		bool     valid_ = false;
	};

	template<class Function, class derived_t>
	class [[maybe_unused]] defer_event<Function, derived_t, std::true_type>
	{
		template <class...> friend class defer_event;
	public:
		template<class Fn, class D = derived_t>
		defer_event(Fn&& fn, event_queue_guard<D> guard) noexcept
			: fn_(std::forward<Fn>(fn))
			, valid_(true)
			, guard_(std::move(guard))
		{
		}

		template<class Fn, class D = derived_t>
		defer_event(defer_event<Fn, D, std::false_type> o, event_queue_guard<D> guard) noexcept
			: fn_   (std::move(o.fn_   ))
			, valid_(          o.valid_ )
			, guard_(std::move(guard   ))
		{
			o.valid_ = false;
		}

		inline defer_event(defer_event&& o) noexcept
			: fn_(std::move(o.fn_)), valid_(o.valid_), guard_(std::move(o.guard_))
		{
			o.valid_ = false;
		};

		defer_event(const defer_event&) = delete;
		defer_event& operator=(const defer_event&) = delete;
		defer_event& operator=(defer_event&&) = delete;

		~defer_event() noexcept
		{
			if (valid_)
			{
				valid_ = false;
				(fn_)(std::move(guard_));
			}

			// guard will be destroy at here, then guard's destroctor will be called
		}

		inline bool    empty() const noexcept { return (!valid_); }
		inline bool is_empty() const noexcept { return (!valid_); }

		inline bool is_event_queue_guard_empty() const noexcept { return guard_.empty(); }

		inline defer_event<Function, derived_t, std::false_type> move_event() noexcept
		{
			defer_event<Function, derived_t, std::false_type> evt(std::move(fn_), nullptr);
			
			evt.valid_ = this->valid_;

			this->valid_ = false;

			return evt;
		}
		inline event_queue_guard<derived_t> move_guard() noexcept
		{
			return std::move(guard_);
		}

	protected:
		Function fn_;

		bool     valid_ = false;

		event_queue_guard<derived_t> guard_;
	};

	template<class derived_t>
	class [[maybe_unused]] defer_event<void, derived_t>
	{
		template <class...> friend class defer_event;
	public:
		defer_event() noexcept
		{
		}

		template<class D = derived_t>
		defer_event(event_queue_guard<D> guard) noexcept
			: guard_(std::move(guard))
		{
		}

		defer_event(defer_event&& o) noexcept = default;
		defer_event(const defer_event&) = delete;
		defer_event& operator=(const defer_event&) = delete;
		defer_event& operator=(defer_event&&) = delete;

		~defer_event() noexcept
		{
			// guard will be destroy at here, then guard's destroctor will be called
		}

		inline constexpr bool    empty() const noexcept { return true; }
		inline constexpr bool is_empty() const noexcept { return true; }

		inline bool is_event_queue_guard_empty() const noexcept { return guard_.empty(); }

		inline defer_event<defer_eqg_dummy<derived_t>, derived_t, std::false_type> move_event() noexcept
		{
			defer_event<defer_eqg_dummy<derived_t>, derived_t, std::false_type> evt(
				defer_eqg_dummy<derived_t>{}, nullptr);

			evt.valid_ = false;

			return evt;
		}
		inline event_queue_guard<derived_t> move_guard() noexcept
		{
			return std::move(guard_);
		}

	protected:
		event_queue_guard<derived_t> guard_;
	};

	template<class F>
	defer_event(F)->defer_event<F>;

	defer_event(std::nullptr_t)->defer_event<void>;

	template<class F, class derived_t>
	defer_event(F, event_queue_guard<derived_t>)->defer_event<F, derived_t, std::true_type>;

	template<class F, class derived_t>
	defer_event(defer_event<F, derived_t, std::false_type>, event_queue_guard<derived_t>)->
		defer_event<F, derived_t, std::true_type>;

	// This will cause error "non-deducible template parameter 'derived_t'" on macos clion
	//template<class F, class derived_t>
	//defer_event(F, std::nullptr_t)->defer_event<F, derived_t, std::false_type>;

	// This will cause error "non-deducible template parameter 'derived_t'" on macos clion
	//template<class derived_t>
	//defer_event()->defer_event<void, derived_t>;

	template<class derived_t>
	defer_event(event_queue_guard<derived_t>)->defer_event<void, derived_t>;


	template<class derived_t, class args_t = void>
	class event_queue_cp
	{
	public:
		/**
		 * @brief constructor
		 */
		event_queue_cp() noexcept {}

		/**
		 * @brief destructor
		 */
		~event_queue_cp() = default;

		/**
		 * @brief Get pending event count in the event queue.
		 */
		inline std::size_t get_pending_event_count() const noexcept
		{
			return this->events_.size();
		}

		/**
		 * post a task to the tail of the event queue
		 * Callback signature : void()
		 */
		template<class Callback>
		inline derived_t& post_queued_event(Callback&& func)
		{
			using return_type = std::invoke_result_t<Callback>;

			derived_t& derive = static_cast<derived_t&>(*this);

			std::packaged_task<return_type()> task(std::forward<Callback>(func));

			auto fn = [p = derive.selfptr(), t = std::move(task)](event_queue_guard<derived_t> g) mutable
			{
				detail::ignore_unused(p, g);

				t();
			};

			// Make sure we run on the io_context thread
			// beacuse the callback "fn" hold the derived_ptr already,
			// so this callback for asio::dispatch don't need hold the derived_ptr again.
			asio::post(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, fn = std::move(fn)]() mutable
			{
				ASIO2_ASSERT(this->events_.size() < std::size_t(32767));

				bool empty = this->events_.empty();
				this->events_.emplace(std::move(fn));
				if (empty)
				{
					(this->events_.front())(event_queue_guard<derived_t>{static_cast<derived_t&>(*this)});
				}
			}));

			return derive;
		}

		/**
		 * post a task to the tail of the event queue
		 * Callback signature : void()
		 */
		template<class Callback, typename Allocator>
		inline auto post_queued_event(Callback&& func, asio::use_future_t<Allocator>) ->
			std::future<std::invoke_result_t<Callback>>
		{
			using return_type = std::invoke_result_t<Callback>;

			derived_t& derive = static_cast<derived_t&>(*this);

			std::packaged_task<return_type()> task(std::forward<Callback>(func));

			std::future<return_type> future = task.get_future();

			auto fn = [p = derive.selfptr(), t = std::move(task)](event_queue_guard<derived_t> g) mutable
			{
				detail::ignore_unused(p, g);

				t();
			};

			// Make sure we run on the io_context thread
			// beacuse the callback "fn" hold the derived_ptr already,
			// so this callback for asio::dispatch don't need hold the derived_ptr again.
			asio::post(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, fn = std::move(fn)]() mutable
			{
				ASIO2_ASSERT(this->events_.size() < std::size_t(32767));

				bool empty = this->events_.empty();
				this->events_.emplace(std::move(fn));
				if (empty)
				{
					(this->events_.front())(event_queue_guard<derived_t>{static_cast<derived_t&>(*this)});
				}
			}));

			return future;
		}

	protected:
		/**
		 * push a task to the tail of the event queue
		 * Callback signature : void(event_queue_guard<derived_t> g)
		 * note : the callback must hold the derived_ptr itself
		 * note : the callback must can't be hold the event_queue_guard, otherwise maybe cause deadlock.
		 */
		template<class Callback>
		inline derived_t& push_event(Callback&& func)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// Should we use post to ensure that the task must be executed in the order of push_event?
			// If we don't do that, example :
			// call push_event in thread main with task1 first, call push_event in thread 0 with task2 second,
			// beacuse the task1 is not in the thread 0, so the task1 will be enqueued by asio::dispatch, but 
			// the task2 is in the thread 0, so the task2 will be enqueued directly, In this case, 
			// task 2 is before task 1 in the queue

		#ifndef ASIO2_STRONG_EVENT_ORDER
			// manual dispatch has better performance.
			// Make sure we run on the io_context thread
			if (derive.io_->running_in_this_thread())
			{
				ASIO2_ASSERT(this->events_.size() < std::size_t(32767));

				bool empty = this->events_.empty();
				this->events_.emplace(std::forward<Callback>(func));
				if (empty)
				{
					(this->events_.front())(event_queue_guard<derived_t>{derive});
				}

				return derive;
			}
		#endif

			// beacuse the callback "func" hold the derived_ptr already,
			// so this callback for asio::dispatch don't need hold the derived_ptr again.
			asio::post(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, func = std::forward<Callback>(func)]() mutable
			{
				ASIO2_ASSERT(this->events_.size() < std::size_t(32767));

				bool empty = this->events_.empty();
				this->events_.emplace(std::move(func));
				if (empty)
				{
					(this->events_.front())(event_queue_guard<derived_t>{static_cast<derived_t&>(*this)});
				}
			}));

			return derive;
		}

		/**
		 * post a task to the tail of the event queue
		 * Callback signature : void(event_queue_guard<derived_t> g)
		 * note : the callback must hold the derived_ptr itself
		 * note : the callback must can't be hold the event_queue_guard, otherwise maybe cause deadlock.
		 */
		template<class Callback>
		inline derived_t& post_event(Callback&& func)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// Make sure we run on the io_context thread
			// beacuse the callback "func" hold the derived_ptr already,
			// so this callback for asio::dispatch don't need hold the derived_ptr again.
			asio::post(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, func = std::forward<Callback>(func)]() mutable
			{
				ASIO2_ASSERT(this->events_.size() < std::size_t(32767));

				bool empty = this->events_.empty();
				this->events_.emplace(std::move(func));
				if (empty)
				{
					(this->events_.front())(event_queue_guard<derived_t>{static_cast<derived_t&>(*this)});
				}
			}));

			return derive;
		}

		/**
		 * dispatch a task
		 * if the guard is not valid, the task will pushed to the tail of the event queue(like push_event),
		 * otherwise the task will be executed directly.
		 * Callback signature : void(event_queue_guard<derived_t> g)
		 * note : the callback must hold the derived_ptr itself
		 * note : the callback must can't be hold the event_queue_guard, otherwise maybe cause deadlock.
		 */
		template<class Callback>
		inline derived_t& disp_event(Callback&& func, event_queue_guard<derived_t> guard)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if (guard.is_empty())
			{
				derive.push_event(std::forward<Callback>(func));
			}
			else
			{
				// when some exception occured, disp_event maybe called in the "catch(){ ... }", 
				// then this maybe not in the io_context thread.

				//ASIO2_ASSERT(derive.io_->running_in_this_thread());

				// beacuse the callback "func" hold the derived_ptr already,
				// so this callback for asio::dispatch don't need hold the derived_ptr again.
				asio::dispatch(derive.io_->context(), make_allocator(derive.wallocator(),
				[func = std::forward<Callback>(func), guard = std::move(guard)]() mutable
				{
					func(std::move(guard));
				}));
			}

			return derive;
		}

		/**
		 * Removes an element from the front of the event queue.
		 * and then execute the next element of the queue.
		 */
		template<typename = void>
		inline derived_t& next_event(event_queue_guard<derived_t> g)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// manual dispatch has better performance.
			// Make sure we run on the io_context thread
			if (derive.io_->running_in_this_thread())
			{
				ASIO2_ASSERT(!g.is_empty());
				ASIO2_ASSERT(!this->events_.empty());

				if (!this->events_.empty())
				{
					this->events_.pop();

					if (!this->events_.empty())
					{
						(this->events_.front())(std::move(g));
					}
					else
					{
						// must set valid to false, otherwise when g is destroyed, it will enter
						// next_event again, this will cause a infinite loop, and cause stack overflow.
						g.valid_ = false;
					}
				}

				ASIO2_ASSERT(g.is_empty());

				return derive;
			}

			// must hold the derived_ptr, beacuse next_event is called by event_queue_guard, when
			// event_queue_guard is destroyed, the event queue and event_queue_guard maybe has't
			// hold derived object both.
			asio::post(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, p = derive.selfptr(), g = std::move(g)]() mutable
			{
				ASIO2_ASSERT(!g.is_empty());
				ASIO2_ASSERT(!this->events_.empty());

				if (!this->events_.empty())
				{
					this->events_.pop();

					if (!this->events_.empty())
					{
						(this->events_.front())(std::move(g));
					}
					else
					{
						// must set valid to false, otherwise when g is destroyed, it will enter
						// next_event again, this will cause a infinite loop, and cause stack overflow.
						g.valid_ = false;
					}
				}

				ASIO2_ASSERT(g.is_empty());
			}));

			return derive;
		}

	protected:
		std::queue<detail::function<
			void(event_queue_guard<derived_t>), detail::function_size_traits<args_t>::value>> events_;
	};
}

#endif // !__ASIO2_EVENT_QUEUE_COMPONENT_HPP__
