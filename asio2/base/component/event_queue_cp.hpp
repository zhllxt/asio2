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
	template<class derived_t>
	class event_queue_cp
	{
	public:
		/**
		 * @constructor
		 */
		event_queue_cp() : derive(static_cast<derived_t&>(*this)) {}

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
			// Make sure we run on the strand
			if (derive.io().strand().running_in_this_thread())
			{
				bool empty = this->events_.empty();
				this->events_.emplace(std::forward<Callback>(f));
				if (empty)
				{
					(this->events_.front())();
				}
				return (static_cast<derived_t &>(*this));
			}

			asio::post(derive.io().strand(), make_allocator(derive.wallocator(),
				[this, p = derive.selfptr(), f = std::forward<Callback>(f)]() mutable
			{
				bool empty = this->events_.empty();
				this->events_.emplace(std::move(f));
				if (empty)
				{
					(this->events_.front())();
				}
			}));

			return (static_cast<derived_t &>(*this));
		}

		/**
		 * Removes an element from the front of the event queue.
		 * and then execute the next element of the queue.
		 */
		template<typename = void>
		inline derived_t & next_event()
		{
			// Make sure we run on the strand
			if (derive.io().strand().running_in_this_thread())
			{
				if (!this->events_.empty())
				{
					this->events_.pop();

					if (!this->events_.empty())
					{
						(this->events_.front())();
					}
				}
				return (static_cast<derived_t &>(*this));
			}

			asio::post(derive.io().strand(), make_allocator(derive.wallocator(),
				[this, p = derive.selfptr()]() mutable
			{
				if (!this->events_.empty())
				{
					this->events_.pop();

					if (!this->events_.empty())
					{
						(this->events_.front())();
					}
				}
			}));

			return (static_cast<derived_t &>(*this));
		}

	protected:
		derived_t                         & derive;

		std::queue<std::function<bool()>>   events_;
	};
}

#endif // !__ASIO2_EVENT_QUEUE_COMPONENT_HPP__
