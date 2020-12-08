/*
 * COPYRIGHT (C) 2017-2019, zhllxt
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

#include <asio2/base/selector.hpp>
#include <asio2/base/iopool.hpp>
#include <asio2/base/error.hpp>

#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/function_traits.hpp>
#include <asio2/base/detail/buffer_wrap.hpp>

namespace asio2::detail
{
	template <class, class>                      class event_queue_cp;

	template<class derived_t, class args_t = void>
	class event_queue_guard
	{
		template <class, class>           friend class event_queue_cp;
	protected:
		event_queue_guard(derived_t & d, bool valid = true)
			: derive(d), derive_ptr_(d.selfptr()), valid_(valid)
		{
		}
	public:
		inline event_queue_guard(event_queue_guard&& o)
			: derive(o.derive), derive_ptr_(std::move(o.derive_ptr_)), valid_(o.valid_)
		{
			o.valid_ = !o.valid_;
		}
		inline event_queue_guard(const event_queue_guard& o)
			: derive(o.derive), derive_ptr_(std::move(o.derive_ptr_)), valid_(o.valid_)
		{
			const_cast<event_queue_guard&>(o).valid_ = !o.valid_;
		}
		inline void operator=(event_queue_guard&& o) = delete;
		inline void operator=(const event_queue_guard& o) = delete;

		~event_queue_guard()
		{
			if (this->valid_)
				derive.next_event();
		}

	protected:
		derived_t                    & derive;
		std::shared_ptr<derived_t>     derive_ptr_;
		bool                           valid_;
	};

	template<class derived_t, class args_t = void>
	class event_queue_cp
	{
	public:
		/**
		 * @constructor
		 */
		event_queue_cp() {}

		/**
		 * @destructor
		 */
		~event_queue_cp() = default;

	public:
		/**
		 * push a task to the tail of the event queue
		 * Callback signature : bool()
		 */
		template<class Callback>
		inline derived_t & push_event(Callback&& f)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

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

			derive.post([this, &derive, p = derive.selfptr(), f = std::forward<Callback>(f)]() mutable
			{
				bool empty = this->events_.empty();
				this->events_.emplace(std::move(f));
				if (empty)
				{
					(this->events_.front())(event_queue_guard<derived_t>{derive});
				}
			});

			return (derive);
		}

	protected:
		/**
		 * Removes an element from the front of the event queue.
		 * and then execute the next element of the queue.
		 */
		template<typename = void>
		inline derived_t & next_event()
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// Make sure we run on the strand
			if (derive.io().strand().running_in_this_thread())
			{
				ASIO2_ASSERT(!this->events_.empty());
				if (!this->events_.empty())
				{
					this->events_.pop();

					if (!this->events_.empty())
					{
						(this->events_.front())(event_queue_guard<derived_t>{derive});
					}
				}
				return (derive);
			}

			derive.post([this, &derive, p = derive.selfptr()]() mutable
			{
				ASIO2_ASSERT(!this->events_.empty());
				if (!this->events_.empty())
				{
					this->events_.pop();

					if (!this->events_.empty())
					{
						(this->events_.front())(event_queue_guard<derived_t>{derive});
					}
				}
			});

			return (derive);
		}

	protected:
		std::queue<std::function<bool(event_queue_guard<derived_t>&&)>> events_;
	};
}

#endif // !__ASIO2_EVENT_QUEUE_COMPONENT_HPP__
