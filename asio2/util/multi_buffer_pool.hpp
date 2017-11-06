/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_MULTI_POOL_HPP__
#define __ASIO2_MULTI_POOL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if !defined(NDEBUG) && !defined(DEBUG) && !defined(_DEBUG)
#	define NDEBUG
#endif

#include <cassert>
#include <memory>
#include <atomic>
#include <unordered_map>
#include <condition_variable>
#include <mutex>
#include <queue>

#include <asio2/util/spin_lock.hpp>
#include <asio2/util/buffer.hpp>

namespace asio2
{

	/**
	 * thread safe multi length buffer pool
	 */
	template<typename T>
	class multi_buffer_pool
	{
	public:

		/**
		 * @construct
		 * @param : nnext_size - have no effect,just for placeholders
		 */
		explicit multi_buffer_pool(const std::size_t nnext_size = 0)
		{
		}

		/**
		 * @destruct
		 */
		virtual ~multi_buffer_pool()
		{
			destroy();
		}

		/**
		 * @function : get a buf for use from buffer_pool, return a shared_ptr object contain the buf,
		 *             when the shared_ptr invalid,will enter the custom deleter,and put the buf
		 *             into the buffer_pool again for next use.
		 */
		std::shared_ptr<buffer<T>> get(std::size_t requested_size)
		{
			while (true)
			{
				std::lock_guard<spin_lock> g(m_lock);
				if (m_pool.find(requested_size) == m_pool.end())
				{
					buffer<T> * p = new buffer<T>(requested_size);
					if (p)
					{
						m_pool_size++;
						m_pool.emplace(requested_size, p);
					}
				}

				auto iterator = m_pool.find(requested_size);
				if (iterator != m_pool.end())
				{
					auto deleter = [this, requested_size](buffer<T> * p)
					{
						std::lock_guard<spin_lock> g(m_lock);

						m_pool.emplace(requested_size, p);

						if (m_pool_size == m_pool.size())
						{
							// notify the wait thread,now all object is released completed
							std::unique_lock<std::mutex> lock(m_mtx);
							m_cv.notify_one();
						}
					};

					buffer<T> * p = iterator->second;
					m_pool.erase(iterator);

					p->resize(0);
					p->reoffset(0);
					p->recapacity(requested_size);

					return std::shared_ptr<buffer<T>>(p, deleter);
				}
			}

			// actually, it will never run to here.
			throw std::runtime_error("unknown error");
		}

		/**
		 * @function : get the count of allocated object already
		 */
		std::size_t get_pool_size()
		{
			return m_pool_size;
		}

		/**
		 * @function : destroy the buffer_pool,this function may be blocking,If some object is still in use,we must wait
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
				for (auto & pair : m_pool)
				{
					m_pool_size--;
					delete pair.second;
				}
				m_pool.clear();
			}
		}

	private:
		/// no copy construct function
		multi_buffer_pool(const multi_buffer_pool&) = delete;

		/// no operator equal function
		multi_buffer_pool& operator=(const multi_buffer_pool&) = delete;

	protected:

		/// unordered_multimap pool,key : requested size, value : buffer<T> *
		std::unordered_multimap<std::size_t, buffer<T>*> m_pool;

		std::size_t m_pool_size = 0;

		/// lock for thread safe
		spin_lock m_lock;

		/// use condition_variable to wait for all object released completed
		std::mutex m_mtx;
		std::condition_variable m_cv;

	};

	/// define send buffer pool type
	using pool_s = multi_buffer_pool<uint8_t>;

}

#endif // !__ASIO2_MULTI_POOL_HPP__
