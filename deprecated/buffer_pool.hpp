/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * Author   : zhllxt
 * QQ       : 37792738
 * Email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_BUFFER_POOL_HPP__
#define __ASIO2_BUFFER_POOL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if !defined(NDEBUG) && !defined(DEBUG) && !defined(_DEBUG)
#	define NDEBUG
#endif


#include <cassert>
#include <memory>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

#include <asio2/util/spin_lock.hpp>
#include <asio2/util/buffer.hpp>

#include <boost/pool/pool.hpp>

namespace asio2
{

	/**
	 * thread safe buffer_pool based on boost pool
	 */
	template<typename T>
	class buffer_pool : public boost::pool<>
	{
	public:

		/**
		 * @construct
		 */
		explicit buffer_pool(const std::size_t requested_size = 1024) : boost::pool<>(requested_size, 64)
		{
		}

		/**
		 * @destruct
		 */
		virtual ~buffer_pool()
		{
			destroy();
		}

		/**
		 * @function : get a buf for use from buffer_pool, return a shared_ptr object contain the buf,
		 *             when the shared_ptr invalid,will enter the custom deleter,and put the buf
		 *             into the buffer_pool again for next use.
		 * @param : requested_size - have no effect,just for placeholders
		 */
		std::shared_ptr<buffer<T>> get(std::size_t requested_size)
		{
			std::lock_guard<spin_lock> g(m_lock);

			// [it's very important] why pass the shared_ptr<this> to the lumdba function ? why not pass "this" pointer to the lumdba function ?
			// because the shared_ptr custom deleter has used "this" object,when the shared_ptr is destructed,it will call the custom deleter,
			// but at this time the "this" object may be destructed already before the shared_ptr destructed,so pass a shared_ptr<this> to the
			// custom deleter,can make sure the "this" obejct is destructed after the the shared_ptr destructed.

			// but if we wait until all shared_ptr object is disappeared in the destructor,we can use this instean this_ptr.

			//auto this_ptr = this->shared_from_this();
			// note : must use this->shared_from_this() ,if you use shared_from_this() ,it will occur [-fpermissive] error when compile on gcc

			auto deleter = [this](buffer<T> * p)
			{
				std::lock_guard<spin_lock> g(m_lock);

				m_pool.emplace(p);

				if (m_pool_size == m_pool.size())
				{
					// notify the wait thread,now all object is released completed
					std::unique_lock<std::mutex> lock(m_mtx);
					m_cv.notify_one();
				}
			};

			while (true)
			{
				if (m_pool.size() == 0)
				{
					auto deleter = [this](T * p)
					{
						assert(is_from((void*)p));
						free((void*)p);
					};

					buffer<T> * p = new buffer<T>(std::shared_ptr<T>((T*)this->malloc(), deleter), 0, this->get_requested_size());
					if (p)
					{
						m_pool_size++;
						m_pool.push(p);
					}
				}

				if (m_pool.size() > 0)
				{
					buffer<T> * p = m_pool.front();
					m_pool.pop();

					p->resize(0);
					p->reoffset(0);
					p->recapacity(this->get_requested_size());

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
		buffer_pool(const buffer_pool&) = delete;

		/// no operator equal function
		buffer_pool& operator=(const buffer_pool&) = delete;

	protected:

		std::queue<buffer<T>*> m_pool;

		std::size_t m_pool_size = 0;

		/// lock for thread safe
		spin_lock m_lock;

		/// use condition_variable to wait for all object released completed
		std::mutex m_mtx;
		std::condition_variable m_cv;

	};

}

#endif // !__ASIO2_BUFFER_POOL_HPP__
