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

//#include <boost/pool/pool.hpp>


namespace asio2
{

	///**
	// * no thread safed and no locked buffer pool based on boost::pool,just used for thread_local
	// */
	//template<class T>
	//class pool : public boost::pool<>, public std::enable_shared_from_this<pool<T>>
	//{
	//public:
	//	/**
	//	 * @construct
	//	 */
	//	pool(std::size_t requested_size) : boost::pool<>(requested_size)
	//	{
	//	}

	//	/**
	//	 * @destruct
	//	 */
	//	virtual ~pool() noexcept
	//	{
	//	}

	//	/**
	//	 * @function : get a buf for use from pool, return a shared_ptr object contain the buf,
	//	 *             when the shared_ptr invalid,will enter the custom deleter,and put the buf
	//	 *             into the pool again for next use.
	//	 */
	//	std::shared_ptr<T> malloc()
	//	{
	//		// use "this->shared_from_this()", don't use "shared_from_this()", otherwise will cause [-fpermissive] error when compiler with gcc.
	//		auto zhis = this->shared_from_this();
	//		auto deleter = [this, zhis](void * buffer)
	//		{
	//			assert(is_from(buffer));
	//			free(buffer);
	//		};

	//		return std::move(std::shared_ptr<T>(reinterpret_cast<T *>(boost::pool<>::malloc()), std::move(deleter)));
	//	}

	//	/**
	//	 * @function : get the allocated memory size
	//	 */
	//	inline std::size_t alloc_size()
	//	{
	//		return boost::pool<>::alloc_size();
	//	}

	//	/**
	//	 * @function : get the requested chunk size
	//	 */
	//	inline std::size_t get_requested_size()
	//	{
	//		return boost::pool<>::get_requested_size();
	//	}

	//private:
	//	/// no copy construct function
	//	pool(const pool&) = delete;

	//	/// no operator equal function
	//	pool& operator=(const pool&) = delete;

	//};

	// use anonymous namespace to resolve global function redefinition problem 
	namespace
	{

		//thread_local static std::shared_ptr<boost::pool<>> _recv_buffer_pool;

		//thread_local static std::shared_ptr<boost::pool<>> _send_buffer_pool;

		std::shared_ptr<uint8_t> malloc_recv_buffer(std::size_t requested_size)
		{
			//if (!_recv_buffer_pool)
			//	_recv_buffer_pool = std::make_shared<boost::pool<>>(requested_size);

			//if (requested_size > _recv_buffer_pool->get_requested_size())
			return std::shared_ptr<uint8_t>(new uint8_t[requested_size], std::default_delete<uint8_t[]>());

			//std::shared_ptr<boost::pool<>> & _pool = _recv_buffer_pool;
			//auto deleter = [_pool](void * buffer)
			//{
			//	assert(_pool->is_from(buffer));
			//	_pool->free(buffer);
			//};

			//return std::shared_ptr<uint8_t>(reinterpret_cast<uint8_t *>(_recv_buffer_pool->malloc()), deleter);
		}

		std::shared_ptr<uint8_t> malloc_send_buffer(std::size_t requested_size)
		{
			//if (!_send_buffer_pool)
			//	_send_buffer_pool = std::make_shared<boost::pool<>>(requested_size);

			//if (requested_size > _send_buffer_pool->get_requested_size())
			return std::shared_ptr<uint8_t>(new uint8_t[requested_size], std::default_delete<uint8_t[]>());

			//std::shared_ptr<boost::pool<>> & _pool = _send_buffer_pool;
			//auto deleter = [_pool](void * buffer)
			//{
			//	assert(_pool->is_from(buffer));
			//	_pool->free(buffer);
			//};

			//return std::shared_ptr<uint8_t>(reinterpret_cast<uint8_t *>(_send_buffer_pool->malloc()), deleter);
		}

	}

}

#endif // !__ASIO2_POOL_HPP__
