/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_THREAD_ID_COMPONENT_HPP__
#define __ASIO2_THREAD_ID_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <thread>

#include <asio2/base/error.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t = void>
	class thread_id_cp
	{
	public:
		/**
		 * @constructor
		 */
		thread_id_cp() = default;

		/**
		 * @destructor
		 */
		~thread_id_cp() = default;

		thread_id_cp(thread_id_cp&&) noexcept = default;
		thread_id_cp(thread_id_cp const&) = default;
		thread_id_cp& operator=(thread_id_cp&&) noexcept = default;
		thread_id_cp& operator=(thread_id_cp const&) = default;

	public:
		/**
		 * @function : Determine whether the object is running in the current thread.
		 */
		inline bool running_in_this_thread() const noexcept
		{
			derived_t& derive = const_cast<derived_t&>(static_cast<const derived_t&>(*this));

			return (derive.io().get_thread_id() == std::this_thread::get_id());
		}

		/**
		 * @function : return the thread id of the current object running in.
		 */
		inline std::thread::id get_thread_id() const noexcept
		{
			derived_t& derive = const_cast<derived_t&>(static_cast<const derived_t&>(*this));

			return derive.io().get_thread_id();
		}

	protected:
	};
}

#endif // !__ASIO2_THREAD_ID_COMPONENT_HPP__
