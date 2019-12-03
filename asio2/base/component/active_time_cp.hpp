/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_ACTIVE_TIME_COMPONENT_HPP__
#define __ASIO2_ACTIVE_TIME_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <chrono>

namespace asio2::detail
{
	template<class derived_t>
	class active_time_cp
	{
	public:
		/**
		 * @constructor
		 */
		active_time_cp() {}

		/**
		 * @destructor
		 */
		~active_time_cp() = default;

	public:
		/**
		 * @function : get the time when the last active event occurred
		 */
		inline std::chrono::time_point<std::chrono::system_clock> active_time() const
		{
			return this->active_time_;
		}

		/**
		 * @function : reset last active time to system_clock::now()
		 */
		inline derived_t & reset_active_time()
		{
			this->active_time_ = std::chrono::system_clock::now();
			return (static_cast<derived_t &>(*this));
		}

		/**
		 * @function : get silence duration of std::chrono::duration
		 */
		inline std::chrono::milliseconds silence_duration() const
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now() - this->active_time_);
		}

	protected:
		/// last active time 
		decltype(std::chrono::system_clock::now())  active_time_ = std::chrono::system_clock::now();
	};
}

#endif // !__ASIO2_ACTIVE_TIME_COMPONENT_HPP__
