/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * referenced from boost/smart_ptr/detail/spinlock_std_atomic.hpp
 */


#ifndef __ASIO2_SPIN_LOCK_HPP__
#define __ASIO2_SPIN_LOCK_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <atomic>
#include <thread>

namespace asio2
{
	class spin_lock
	{
	public:
		spin_lock() noexcept
		{
			v_.clear();
		}

		bool try_lock() noexcept
		{
			return !v_.test_and_set(std::memory_order_acquire);
		}

		void lock() noexcept
		{
			for (unsigned k = 0; !try_lock(); ++k)
			{
				if (k < 16)
				{
					std::this_thread::yield();
				}
				else if (k < 32)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(0));
				}
				else
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			}
		}

		void unlock() noexcept
		{
			v_.clear(std::memory_order_release);
		}

	public:
		std::atomic_flag v_;
	};
}

#endif // !__ASIO2_SPIN_LOCK_HPP__
