/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * referenced from : http://blog.csdn.net/10km/article/details/49641691
 * 
 * note : you should't call lock_read twice in the same thread, because it may be cause dead lock. see below:
 * void thread1()
 * {
 *      lock_read();  // 1
 *      work();       // 2
 *      lock_read();  // 3
 * }
 * void thread2()
 * {
 *      lock_write(); // 4
 * }
 * 
 * code run to 1,lock_read is successed and return.
 * code run to 2,long time working.
 * code run to 4,lock_write will block,becuse 1 lock_read hold the lock.
 * code run to 3,lock_read will block,because m_write_wait_count is equal 1.
 * then dead lock is occurred.
 */

#ifndef __ASIO2_RWLOCK_HPP__
#define __ASIO2_RWLOCK_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)


#include <atomic>
#include <thread>

namespace asio2
{

	/**
	 * high performance read write lock based on std::atomic
	 */
	class rwlock
	{
	public:

		rwlock(bool is_write_first = true)
			: m_is_write_first(is_write_first)
			, m_write_wait_count(0)
			, m_lock_count(0)
		{
		}

		bool try_lock_read()
		{
			int count = m_lock_count;

			// if write first is true,need wait for util m_write_wait_count equal to zero,this means 
			// let the write thread first get the lock
			if (count == -1 || (m_is_write_first && m_write_wait_count > 0))
				return false;

			return m_lock_count.compare_exchange_weak(count, count + 1);
		}

		void lock_read()
		{
			for (unsigned int k = 0; !try_lock_read(); ++k)
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

		void unlock_read()
		{
			m_lock_count--;
		}

		bool try_lock_write()
		{
			// check m_lock_count whether equal to 0,it must be equal 0
			int count = 0;
			return m_lock_count.compare_exchange_weak(count, -1);
		}

		void lock_write()
		{
			m_write_wait_count++;
			for (unsigned int k = 0; !try_lock_write(); ++k)
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
			m_write_wait_count--;
		}

		void unlock_write()
		{
			m_lock_count.store(0);
		}

	private:
		/// no copy construct function
		rwlock(const rwlock&) = delete;

		/// no operator equal function
		rwlock& operator=(const rwlock&) = delete;

	protected:
		/// is write first or not
		bool m_is_write_first = true;

		/// wait for write thread count
		std::atomic_uint m_write_wait_count;

		/// used to indicate the lock status, -1 : write,  0 : idle,  >0 : shared read
		std::atomic_int m_lock_count;

	};

	/**
	 * auto call lock_read and unlock_read 
	 */
	class rlock_guard
	{
	public:
		explicit rlock_guard(rwlock & lock) : m_rwlock(lock)
		{
			m_rwlock.lock_read();
		}
		~rlock_guard()
		{
			m_rwlock.unlock_read();
		}
	private:
		rwlock & m_rwlock;

		rlock_guard(const rlock_guard&) = delete;
		rlock_guard& operator=(const rlock_guard&) = delete;
	};

	/**
	 * auto call lock_write and unlock_write 
	 */
	class wlock_guard
	{
	public:
		explicit wlock_guard(rwlock & lock) : m_rwlock(lock)
		{
			m_rwlock.lock_write();
		}
		~wlock_guard()
		{
			m_rwlock.unlock_write();
		}
	private:
		rwlock & m_rwlock;

		wlock_guard(const wlock_guard&) = delete;
		wlock_guard& operator=(const wlock_guard&) = delete;
	};

}

#endif // !__ASIO2_RWLOCK_HPP__
