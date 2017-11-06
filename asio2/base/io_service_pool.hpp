/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_IO_SERVICE_POOL_HPP__
#define __ASIO2_IO_SERVICE_POOL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <vector>
#include <memory>
#include <thread>

#include <boost/asio.hpp>

namespace asio2
{

	/**
	 * the io_service_pool interface
	 * note : must ensure all events completion handler of the same socket run in the same thread within 
	 *        io_service.run,the event completion handler is refer to async_read async_send and so on...
	 */
	class io_service_pool
	{
	public:
		
		/**
		 * @construct
		 * @param    : pool_size - the new pool size,default is equal to the cpu kernel count
		 */
		explicit io_service_pool(std::size_t pool_size = std::thread::hardware_concurrency())
		{
			if (pool_size == 0)
			{
				assert(false);
				pool_size = std::thread::hardware_concurrency();
			}

			// Give all the io_services work to do so that their run() functions will not 
			// exit until they are explicitly stopped. 
			for (std::size_t i = 0; i < pool_size; ++i)
			{
				std::shared_ptr<boost::asio::io_service> ioservice_ptr = std::make_shared<boost::asio::io_service>();
				std::shared_ptr<boost::asio::io_service::work> workptr = std::make_shared<boost::asio::io_service::work>(*ioservice_ptr);
				m_io_services.emplace_back(ioservice_ptr);
				m_works.emplace_back(workptr);
			}
		}

		/**
		 * @destruct
		 */
		virtual ~io_service_pool()
		{
		}

		/**
		 * @function : run all io_service objects in the pool. 
		 */
		void run()
		{
			std::size_t i;

			// Create a pool of threads to run all of the io_services. 
			std::vector<std::shared_ptr<std::thread>> threads;
			for (i = 0; i < m_io_services.size(); ++i)
			{
				std::shared_ptr<std::thread> thread = std::make_shared<std::thread>(
					// when bind a override function,should use static_cast to convert the function to correct function version
					std::bind(static_cast<std::size_t (boost::asio::io_service::*)()>(
						&boost::asio::io_service::run), m_io_services[i]) );
				threads.emplace_back(thread);
			}

			// Wait for all threads in the pool to exit. 
			for (i = 0; i < threads.size(); ++i)
			{
				threads[i]->join();
			}
		}

		/**
		 * @function : stop all io_service objects in the pool
		 */
		void stop()
		{
			// Explicitly stop all io_services. 
			for (std::size_t i = 0; i < m_io_services.size(); ++i)
			{
				m_io_services[i]->stop();
			}
		}

		/**
		 * @function : destroy all io_service in the pool
		 */
		void destroy()
		{
			m_works.clear();
			m_io_services.clear();
			m_next_io_service = 0;
		}

		/**
		 * @function : get an io_service to use
		 */
		std::shared_ptr<boost::asio::io_service> get_io_service_ptr()
		{
			// use lock to protected "m_next_io_service",otherwise "m_io_services[m_next_io_service]" may be out of bounds
			std::lock_guard<spin_lock> g(m_lock);
			// Use a round-robin scheme to choose the next io_service to use. 
			std::shared_ptr<boost::asio::io_service> ioservice_ptr = m_io_services[m_next_io_service];
			m_next_io_service++;
			if (m_next_io_service == m_io_services.size())
				m_next_io_service = 0;
			return ioservice_ptr;
		}

	protected:

		/// The pool of io_services. 
		std::vector<std::shared_ptr<boost::asio::io_service>> m_io_services;

		/// The work that keeps the io_services running. 
		std::vector<std::shared_ptr<boost::asio::io_service::work>> m_works;

		/// The next io_service to use for a connection. 
		std::size_t m_next_io_service = 0;

		/// spin lock for thread safe
		spin_lock m_lock;
	};

	using io_service = boost::asio::io_service;
	using io_service_pool_ptr = std::shared_ptr<io_service_pool>;
}

#endif // !__ASIO2_IO_SERVICE_POOL_HPP__
