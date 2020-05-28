/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_TIMER_HPP__
#define __ASIO2_TIMER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>
#include <memory>
#include <chrono>
#include <functional>
#include <atomic>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <type_traits>

#include <asio2/base/selector.hpp>
#include <asio2/base/iopool.hpp>
#include <asio2/base/error.hpp>

#include <asio2/base/detail/object.hpp>
#include <asio2/base/detail/allocator.hpp>
#include <asio2/base/detail/util.hpp>

#include <asio2/base/component/user_timer_cp.hpp>
#include <asio2/base/component/post_cp.hpp>

namespace asio2::detail
{
	template<class derived_t>
	class timer_impl_t
		: public object_t<derived_t>
		, public iopool_cp
		, public user_timer_cp<derived_t, false>
		, public post_cp<derived_t>
	{
		template <class, bool>         friend class user_timer_cp;
		template <class>               friend class post_cp;

	public:
		using self = timer_impl_t<derived_t>;
		using super = object_t<derived_t>;

		/**
		 * @constructor
		 */
		timer_impl_t()
			: object_t<derived_t>()
			, iopool_cp(1)
			, user_timer_cp<derived_t, false>(iopool_.get(0))
			, io_(iopool_.get(0))
		{
			this->iopool_.start(); // start the io_context pool
		}

		/**
		 * @destructor
		 */
		~timer_impl_t()
		{
			this->stop_all_timers();
			this->iopool_.stop(); // stop the io_context pool
		}

	public:
		/**
		 * @function : get the io object refrence
		 */
		inline io_t & io() { return this->io_; }

	protected:
		/**
		 * @function : get the recv/read allocator object refrence
		 */
		inline auto & rallocator() { return this->wallocator_; }
		/**
		 * @function : get the send/write allocator object refrence
		 */
		inline auto & wallocator() { return this->wallocator_; }

		inline std::shared_ptr<derived_t>   selfptr()  { return std::shared_ptr<derived_t>{}; }

	protected:
		/// The io (include io_context and strand) used to handle the accept event.
		io_t                                          & io_;

		/// The memory to use for handler-based custom memory allocation. used fo send/write.
		handler_memory<size_op<>, std::true_type>       wallocator_;
	};
}

namespace asio2
{
	class timer : public detail::timer_impl_t<timer>
	{
	public:
		using detail::timer_impl_t<timer>::timer_impl_t;
	};
}

#endif // !__ASIO2_TIMER_HPP__
