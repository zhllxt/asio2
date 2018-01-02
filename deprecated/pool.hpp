/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * Author   : zhllxt
 * QQ       : 37792738
 * Email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_POOL_HPP__
#define __ASIO2_POOL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if !defined(NDEBUG) && !defined(DEBUG) && !defined(_DEBUG)
#	define NDEBUG
#endif


#include <cassert>
#include <memory>
#include <atomic>

#include <asio2/util/spin_lock.hpp>

#include <boost/pool/pool.hpp>


namespace asio2
{

	/**
	 * thread safe pool based on boost pool
	 * you must construct this object by "new" mode, can't construct this object on the stack.because it used shared_from_this.
	 */
	template<typename T>
	class pool : public boost::pool<>, public std::enable_shared_from_this<asio2::pool<T>>
	{
	public:

		/**
		 * @construct
		 */
		explicit pool(
			const std::size_t nrequested_size
		) : boost::pool<>(nrequested_size)
		{
		}

		/**
		 * @destruct
		 */
		virtual ~pool() noexcept
		{
		}

		/**
		 * @function : get a buf for use from pool, return a shared_ptr object contain the buf,
		 *             when the shared_ptr invalid,will enter the custom deleter,and put the buf
		 *             into the pool again for next use.
		 * @param : requested_size - have no effect,just for placeholders
		 */
		std::shared_ptr<T> get(std::size_t requested_size = 0)
		{
			std::lock_guard<spin_lock> g(m_lock);
			// [it's very important] why pass the shared_ptr<this> to the lumdba function ? why not pass "this" pointer to the lumdba function ?
			// because the shared_ptr custom deleter has used "this" object,when the shared_ptr is destructed,it will call the custom deleter,
			// but at this time the "this" object may be destructed already before the shared_ptr destructed,so pass a shared_ptr<this> to the
			// custom deleter,can make sure the "this" obejct is destructed after the the shared_ptr destructed.
			auto this_ptr = this->shared_from_this();
			// note : must use this->shared_from_this() ,if you use shared_from_this() ,it will occur [-fpermissive] error when compile on gcc
			auto deleter = [this_ptr](T * p)
			{
				std::lock_guard<spin_lock> g(this_ptr->m_lock);
				assert(this_ptr->is_from((void*)p));
				this_ptr->free((void*)p);
			};
			return std::shared_ptr<T>((T*)malloc(), deleter);
		}

		/**
		 * @function : get the allocated memory size
		 */
		std::size_t get_pool_size()
		{
			return this->alloc_size();
		}

		/**
		 * @function : release the buffer pool malloced memory 
		 */
		void destroy()
		{
			purge_memory();
		}

	private:
		/// no copy construct function
		pool(const pool&) = delete;

		/// no operator equal function
		pool& operator=(const pool&) = delete;

	protected:

		/// lock for thread safe
		spin_lock m_lock;

	};

}

#endif // !__ASIO2_POOL_HPP__
