/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_ALIVE_TIME_COMPONENT_HPP__
#define __ASIO2_ALIVE_TIME_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <chrono>

namespace asio2::detail
{
	template<class derived_t, class args_t>
	class alive_time_cp
	{
	public:
		/**
		 * @constructor
		 */
		alive_time_cp() = default;

		/**
		 * @destructor
		 */
		~alive_time_cp() = default;

		alive_time_cp(alive_time_cp&&) noexcept = default;
		alive_time_cp(alive_time_cp const&) = default;
		alive_time_cp& operator=(alive_time_cp&&) noexcept = default;
		alive_time_cp& operator=(alive_time_cp const&) = default;

	public:
		/**
		 * @function : get the time when the last alive event occurred, same as get_last_alive_time
		 */
		inline std::chrono::time_point<std::chrono::system_clock> last_alive_time() const noexcept
		{
			return this->get_last_alive_time();
		}

		/**
		 * @function : get the time when the last alive event occurred
		 */
		inline std::chrono::time_point<std::chrono::system_clock> get_last_alive_time() const noexcept
		{
			return this->last_alive_time_;
		}

		/**
		 * @function : reset last alive time to system_clock::now()
		 */
		inline derived_t & update_alive_time() noexcept
		{
			this->last_alive_time_ = std::chrono::system_clock::now();
			return (static_cast<derived_t &>(*this));
		}

		/**
		 * @function : get silence duration of std::chrono::duration
		 */
		inline std::chrono::system_clock::duration get_silence_duration() const noexcept
		{
			return std::chrono::duration_cast<std::chrono::system_clock::duration>(
				std::chrono::system_clock::now() - this->last_alive_time_);
		}

	protected:
		/// last alive time 
		decltype(std::chrono::system_clock::now())  last_alive_time_ = std::chrono::system_clock::now();
	};
}

#endif // !__ASIO2_ALIVE_TIME_COMPONENT_HPP__
