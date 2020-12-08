/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_CONNECT_TIME_COMPONENT_HPP__
#define __ASIO2_CONNECT_TIME_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <chrono>

namespace asio2::detail
{
	template<class derived_t, class args_t = void>
	class connect_time_cp
	{
	public:
		/**
		 * @constructor
		 */
		connect_time_cp() {}

		/**
		 * @destructor
		 */
		~connect_time_cp() = default;

	public:
		/**
		 * @function : get build connection time
		 */
		inline std::chrono::time_point<std::chrono::system_clock> connect_time() const
		{
			return this->connect_time_;
		}

		/**
		 * @function : reset build connection time to system_clock::now()
		 */
		inline derived_t & reset_connect_time()
		{
			this->connect_time_ = std::chrono::system_clock::now();
			return (static_cast<derived_t &>(*this));
		}

		/**
		 * @function : get connection duration of std::chrono::duration
		 */
		inline std::chrono::system_clock::duration connect_duration() const
		{
			return std::chrono::duration_cast<std::chrono::system_clock::duration>(
				std::chrono::system_clock::now() - this->connect_time_);
		}

	protected:
		/// build connection time
		decltype(std::chrono::system_clock::now()) connect_time_ = std::chrono::system_clock::now();
	};
}

#endif // !__ASIO2_CONNECT_TIME_COMPONENT_HPP__
