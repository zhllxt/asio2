/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
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

#include <asio2/3rd/asio.hpp>
#include <asio2/base/iopool.hpp>
#include <asio2/base/error.hpp>

#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/function_traits.hpp>
#include <asio2/base/detail/buffer_wrap.hpp>
#include <asio2/base/detail/function.hpp>

namespace asio2::detail
{
	struct defer_event_dummy
	{
		inline void operator()() noexcept {}
	};

	template<class Function = defer_event_dummy>
	class defer_event
	{
	public:
		template<class Fn>
		defer_event(Fn&& fn) noexcept : f(std::forward<Fn>(fn)), valid_(true) {}

		defer_event(std::nullptr_t) noexcept : f(defer_event_dummy{}), valid_(false) {}

		inline defer_event(defer_event&& o) noexcept : f(std::move(o.f)), valid_(o.valid_)
		{
			o.valid_ = false;
		};
		inline void operator=(defer_event&& o) noexcept
		{
			ASIO2_ASSERT(false);

			f = std::move(o.f);
			valid_ = o.valid_;
			o.valid_ = false;
		};

		inline defer_event(const defer_event& o) noexcept : f(const_cast<Function&&>(o.f)), valid_(o.valid_)
		{
			ASIO2_ASSERT(false);

			const_cast<defer_event&>(o).valid_ = false;
		};
		inline void operator=(const defer_event& o) noexcept
		{
			ASIO2_ASSERT(false);

			f = const_cast<Function&&>(o.f);
			valid_ = o.valid_;
			const_cast<defer_event&>(o).valid_ = false;
		}

		~defer_event() noexcept
		{
			if (valid_)
				f();
		}

		inline bool    empty() const noexcept { return !valid_; }
		inline bool is_empty() const noexcept { return !valid_; }

	protected:
		Function f;

		bool valid_;
	};

	template<class F>
	defer_event(F)->defer_event<F>;

	defer_event(std::nullptr_t)->defer_event<defer_event_dummy>;


	template <class, class>                      class event_queue_cp;

	template<class derived_t, class args_t = void>
	class event_queue_guard
	{
		template <class, class>           friend class event_queue_cp;

	protected:
		event_queue_guard(derived_t& d) noexcept : derive(d), derive_ptr_(d.selfptr()) {}

	public:
		inline event_queue_guard(event_queue_guard&& o) noexcept
			: derive(o.derive), derive_ptr_(std::move(o.derive_ptr_)), valid_(o.valid_)
		{
			ASIO2_ASSERT(o.valid_ == true);

			o.valid_ = false;
		}
		inline event_queue_guard(const event_queue_guard& o) = delete;
		inline void operator=(event_queue_guard&& o) = delete;
		inline void operator=(const event_queue_guard& o) = delete;

		~event_queue_guard() noexcept
		{
			if (this->valid_)
				derive.next_event(std::move(*this));
		}

		inline bool    valid() const noexcept { return valid_; }
		inline bool is_valid() const noexcept { return valid_; }

	protected:
		derived_t                    & derive;

		// must hold the derived object, maybe empty in client
		// if didn't hold the derived object, when the callback is executed in the event queue,
		// the derived object which holded by the callback maybe destroyed by std::move(), when
		// event_queue_guard is destroyed, and will call derive.next_event; the "derive" maybe
		// invalid already.
		std::shared_ptr<derived_t>     derive_ptr_;

		// whether the guard is valid, when object is moved by std::move the guard will be invalid
		bool                           valid_ = true;
	};

	template<class derived_t, class args_t = void>
	class event_queue_cp
	{
	public:
		/**
		 * @constructor
		 */
		event_queue_cp() noexcept {}

		/**
		 * @destructor
		 */
		~event_queue_cp() = default;

	protected:
		/**
		 * push a task to the tail of the event queue
		 * Callback signature : void(event_queue_guard<derived_t>&& g)
		 * note : the callback must hold the derived_ptr itself
		 */
		template<class Callback>
		inline derived_t & push_event(Callback&& f)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// Should we use post to ensure that the task must be executed in the order of push_event?
			// If we don't do that, example :
			// call push_event in thread main with task1 first, call push_event in thread 0 with task2 second,
			// beacuse the task1 is not in the thread 0, so the task1 will be enqueued by asio::post, but 
			// the task2 is in the thread 0, so the task2 will be enqueued directly, In this case, 
			// task 2 is before task 1 in the queue

			// Make sure we run on the strand
			if (derive.io().strand().running_in_this_thread())
			{
				bool empty = this->events_.empty();
				this->events_.emplace(std::forward<Callback>(f));
				if (empty)
				{
					(this->events_.front())(event_queue_guard<derived_t>{derive});
				}
				return (derive);
			}

			// beacuse the callback "f" hold the derived_ptr already,
			// so this callback for asio::post don't need hold the derived_ptr again.
			asio::post(derive.io().strand(), make_allocator(derive.wallocator(),
			[this, &derive, f = std::forward<Callback>(f)]() mutable
			{
				ASIO2_ASSERT(this->events_.size() < std::size_t(32767));

				bool empty = this->events_.empty();
				this->events_.emplace(std::move(f));
				if (empty)
				{
					(this->events_.front())(event_queue_guard<derived_t>{derive});
				}
			}));

			return (derive);
		}

		/**
		 * Removes an element from the front of the event queue.
		 * and then execute the next element of the queue.
		 */
		template<typename = void>
		inline derived_t & next_event(event_queue_guard<derived_t>&& g)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// Make sure we run on the strand
			if (derive.io().strand().running_in_this_thread())
			{
				ASIO2_ASSERT(g.valid());

				if (!this->events_.empty())
				{
					this->events_.pop();

					if (!this->events_.empty())
					{
						// force move, if use std::move(g), the "g" maybe passed as a refrence and not moved
						(this->events_.front())(event_queue_guard<derived_t>{std::move(g)});

						ASIO2_ASSERT(!g.valid());
					}
				}

				return (derive);
			}

			// must hold the derived_ptr, beacuse next_event is called by event_queue_guard, when
			// event_queue_guard is destroyed, the event queue and event_queue_guard maybe has't
			// hold derived object both.
			asio::post(derive.io().strand(), make_allocator(derive.wallocator(),
			[this, p = derive.selfptr(), g = std::move(g)]() mutable
			{
				ASIO2_ASSERT(g.valid());

				if (!this->events_.empty())
				{
					this->events_.pop();

					if (!this->events_.empty())
					{
						// force move, if use std::move(g), the "g" maybe passed as a refrence and not moved
						(this->events_.front())(event_queue_guard<derived_t>{std::move(g)});

						ASIO2_ASSERT(!g.valid());
					}
				}
			}));

			return (derive);
		}

	protected:
		std::queue<detail::function<void(event_queue_guard<derived_t>&&)>> events_;
	};
}

#endif // !__ASIO2_EVENT_QUEUE_COMPONENT_HPP__
