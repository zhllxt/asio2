/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * The cost of dynamic (virtual calls) vs. static (CRTP) dispatch in C++
 * https://eli.thegreenplace.net/2013/12/05/the-cost-of-dynamic-virtual-calls-vs-static-crtp-dispatch-in-c/
 */

#ifndef __ASIO2_OBJECT_HPP__
#define __ASIO2_OBJECT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>

#include <asio2/base/error.hpp>

namespace asio2
{
	class object
	{
	public:
	};
}

namespace asio2::detail
{
	/**
	 * the lowest based class used fo CRTP
	 * see : CRTP and multilevel inheritance 
	 * https://stackoverflow.com/questions/18174441/crtp-and-multilevel-inheritance
	 */
	template<class derived_t, bool enable_shared_from_this = true>
	class object_t : public asio2::object, public std::enable_shared_from_this<derived_t>
	{
	protected:
		/**
		 * @brief constructor
		 */
		object_t() = default;

		/**
		 * @brief destructor
		 */
		~object_t() = default;

	protected:
		/**
		 * @brief obtain derived class object through CRTP mechanism
		 */
		inline const derived_t & derived() const noexcept
		{
			return static_cast<const derived_t &>(*this);
		}

		/**
		 * @brief obtain derived class object through CRTP mechanism
		 */
		inline derived_t & derived() noexcept
		{
			return static_cast<derived_t &>(*this);
		}

		/**
		 * @brief if the "derived_t" is created like a "shared_ptr", it will return
		 *             a not empty shared_ptr<derived_t>, othwise it will return a empty
		 *             shared_ptr<derived_t>.
		 */
		inline std::shared_ptr<derived_t> selfptr() noexcept
		{
			// if the "derived_t" (maybe server,client,session...) is created like a
			// "shared_ptr", then here will return a not empty shared_ptr, otherwise
			// here will return a empty shared_ptr.
			// e.g : when the "derived_t" is udp_cast, and user has called post_condition_event,
			// and hold the "event_ptr" into another thread, and when the udp_cast is 
			// soppted and destroyed, and user called the "event_ptr->notify()" 
			// in the "another thread", if the udp_cast is created like a "shared_ptr",
			// then the event_ptr's member variable "derive_ptr_" will hold the shared_ptr
			// of udp_cast, this has no problem. but if the udp_cast is created as a 
			// local variable not a "shared_ptr", then the event_ptr's member variable
			// "derive_ptr_" will hold a empty shared_ptr, then it will crash when 
			// user called the "event_ptr->notify()" in the "another thread", beacuse
			// at this time, the "event_timer_io_" maybe destroyed already.
			return this->derived().weak_from_this().lock();
		}
	};

	template<class derived_t>
	class object_t<derived_t, false> : public asio2::object
	{
	protected:
		/**
		 * @brief constructor
		 */
		object_t() = default;

		/**
		 * @brief destructor
		 */
		~object_t() = default;

	protected:
		/**
		 * @brief obtain derived class object through CRTP mechanism
		 */
		inline const derived_t & derived() const noexcept
		{
			return static_cast<const derived_t &>(*this);
		}

		/**
		 * @brief obtain derived class object through CRTP mechanism
		 */
		inline derived_t & derived() noexcept
		{
			return static_cast<derived_t &>(*this);
		}

		/**
		 * @brief always return a empty shared_ptr<derived_t>.
		 */
		inline std::shared_ptr<derived_t> selfptr() noexcept
		{
			return std::shared_ptr<derived_t>{};
		}
	};
}

#endif // !__ASIO2_OBJECT_HPP__
