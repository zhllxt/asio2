/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_OBJECT_POOL_HPP__
#define __ASIO2_OBJECT_POOL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if !defined(NDEBUG) && !defined(DEBUG) && !defined(_DEBUG)
#	define NDEBUG
#endif

#include <cassert>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <queue>

#include <asio2/util/spin_lock.hpp>

namespace asio2
{

	/**
	 * the object pool interface , this pool is multi thread safed.
	 */
	template<class _obj>
	class object_pool
	{
	public:

		/**
		 * @construct
		 */
		object_pool(const std::size_t next_size = 64) : m_next_size(next_size)
		{
		}

		/**
		 * @destruct
		 */
		virtual ~object_pool()
		{
			destroy();
		}

		/**
		 * @function : get a object for use,you dont need release the object into the pool after you use completed,
		 *             it will automic put the object into the pool after you use completed.
		 */
		template<typename... Args>
		std::shared_ptr<_obj> get(Args&&... args)
		{
			std::lock_guard<spin_lock> g(m_lock);

			while (true)
			{
				if (m_pool.size() == 0)
				{
					for (std::size_t i = 0; i < m_next_size; i++)
					{
						auto p = new _obj(std::forward<Args>(args)...);
						assert(p);
						if (p)
						{
							m_pool_size++;
							m_pool.push(p);
						}
					}
				}

				if (m_pool.size() > 0)
				{
					auto p = m_pool.front();
					m_pool.pop();

					// instead use shared_ptr of "this",we use "this" directly,so we must ensure that when "this" 
					// object is destroyed,all the allocated object must be released already,otherwise when the 
					// shared_ptr of object is disappeared,it will call the custom deleter,and referenced "this",
					// but this object is destroyed already,it is crash.

					auto deleter = [this](_obj * p)
					{
						std::lock_guard<spin_lock> g(m_lock);
						m_pool.push(p);

						if (m_pool_size == m_pool.size())
						{
							// notify the wait thread,now all object is released completed
							std::unique_lock<std::mutex> lock(m_mtx);
							m_cv.notify_one();
						}
					};

					return std::shared_ptr<_obj>(p, deleter);
				}
			}
			// actually, it will never run to here.
			return nullptr;
		}

		/**
		 * @function : get the count of allocated object already
		 */
		std::size_t get_pool_size()
		{
			return m_pool_size;
		}

		/**
		 * @function : destroy the pool,this function may be blocking,If some object is still in use,we must wait
		 *             util all object is not in use,then we can destroy all objects.
		 */
		void destroy()
		{
			{
				// Wait until all object released completed
				std::unique_lock<std::mutex> lock(m_mtx);
				if (m_pool_size > m_pool.size())
				{
					m_cv.wait(lock);
				}
			}

			if (m_pool.size() > 0)
			{
				std::lock_guard<spin_lock> g(m_lock);
				while (m_pool.size() > 0)
				{
					m_pool_size--;
					delete m_pool.front();
					m_pool.pop();
				}
			}
		}

	private:
		/// no copy construct function
		object_pool(const object_pool&) = delete;

		/// no operator equal function
		object_pool& operator=(const object_pool&) = delete;

	protected:

		std::queue<_obj *> m_pool;

		std::size_t m_next_size = 64;

		std::size_t m_pool_size = 0;

		spin_lock m_lock;

		/// use condition_variable to wait for all object released completed
		std::mutex m_mtx;
		std::condition_variable m_cv;
	};

}

#endif // !__ASIO2_OBJECT_POOL_HPP__
