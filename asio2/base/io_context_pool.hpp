/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_IO_CONTEXT_POOL_HPP__
#define __ASIO2_IO_CONTEXT_POOL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if !defined(NDEBUG) && !defined(DEBUG) && !defined(_DEBUG)
#	define NDEBUG
#endif

#include <cassert>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>

#include <boost/asio.hpp>

namespace asio2
{

	/**
	 * the io_context_pool interface
	 * note : must ensure all events completion handler of the same socket run in the same thread within 
	 *        io_context::run,the event completion handler is mean to async_read async_send and so on...
	 */
	class io_context_pool
	{
	public:
		typedef boost::asio::executor_work_guard<boost::asio::io_context::executor_type> io_context_work;

		/**
		 * @construct
		 * @param    : pool_size - the new pool size,default is equal to the cpu kernel count
		 */
		explicit io_context_pool(std::size_t pool_size = std::thread::hardware_concurrency())
		{
			if (pool_size == 0)
			{
				pool_size = std::thread::hardware_concurrency();
			}

			for (std::size_t i = 0; i < pool_size; ++i)
			{
				m_io_contexts.emplace_back(std::make_shared<boost::asio::io_context>());
			}
		}

		/**
		 * @destruct
		 */
		virtual ~io_context_pool()
		{
		}

		/**
		 * @function : run all io_context objects in the pool.
		 */
		void run()
		{
			assert(m_works.size() == 0 && m_threads.size() == 0);
			// Create a pool of threads to run all of the io_contexts. 
			for (auto & io_context_ptr : m_io_contexts)
			{
				// Give all the io_contexts work to do so that their run() functions will not 
				// exit until they are explicitly stopped. 
				m_works.emplace_back(std::make_shared<io_context_work>(io_context_ptr->get_executor()));

				// start work thread
				m_threads.emplace_back(
					// when bind a override function,should use static_cast to convert the function to correct function version
					std::bind(static_cast<std::size_t(boost::asio::io_context::*)()>(
						&boost::asio::io_context::run), io_context_ptr));
			}
		}

		/**
		 * @function : stop all io_context objects in the pool
		 */
		void stop()
		{
			// call work reset,and then the io_context working thread will be exited.
			for (auto & work_ptr : m_works)
			{
				work_ptr->reset();
			}
			m_works.clear();
			// Wait for all threads to exit. 
			for (auto & thread : m_threads)
			{
				assert(thread.get_id() != std::this_thread::get_id());
				if (thread.joinable())
					thread.join();
			}
			m_threads.clear();
			// Reset the io_context in preparation for a subsequent run() invocation.
			for (auto & io_context_ptr : m_io_contexts)
			{
				io_context_ptr->restart();
			}
		}

		/**
		 * @function : get an io_context to use
		 */
		std::shared_ptr<boost::asio::io_context> get_io_context_ptr()
		{
			// Use a round-robin scheme to choose the next io_context to use. 
			return m_io_contexts[(m_next_io_context++) % m_io_contexts.size()];
		}

	protected:
		/// threads to run all of the io_contexts
		std::vector<std::thread> m_threads;

		/// The pool of io_contexts. 
		std::vector<std::shared_ptr<boost::asio::io_context>> m_io_contexts;

		/// The work that keeps the io_contexts running. 
		std::vector<std::shared_ptr<io_context_work>> m_works;

		/// The next io_context to use for a connection. 
		std::size_t m_next_io_context = 0;

	};

}

#endif // !__ASIO2_IO_CONTEXT_POOL_HPP__
