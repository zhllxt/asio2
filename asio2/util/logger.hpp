/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_LOGGER_HPP__
#define __ASIO2_LOGGER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if !defined(NDEBUG) && !defined(DEBUG) && !defined(_DEBUG)
#	define NDEBUG
#endif

#include <cstdio>
#include <cassert>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <queue>
#include <condition_variable>
#include <functional>
#include <future>

#include <asio2/util/helper.hpp>

namespace asio2
{

#if defined(_MSC_VER)
#	pragma warning(push) 
#	pragma warning(disable:4996)
#endif

	/**
	 * the logger interface
	 */
	class logger
	{
	public:

		/**
		 * @construct
		 */
		explicit logger(std::string filename = std::string())
		{
			if (filename.empty())
			{
				filename = get_current_directory();

				time_t aclock;
				time(&aclock);                 /* Get time in seconds */
				struct tm * newtime = localtime(&aclock);  /* Convert time to struct */
				char buf[16] = { 0 };
				strftime(buf, 128, "%Y-%m-%d", newtime);

				filename += buf;
				filename += ".log";
			}
			else
			{
				// Compatible with three file name parameters : 
				// abc.log
				// D:/log/abc.log
				// /usr/log/abc.log
				size_t pos = filename.find_last_of('/');
				if (pos == std::string::npos)
					pos = filename.find_last_of('\\');

				std::string dir;
				if (pos != std::string::npos)
					dir = filename.substr(0, pos);

				if (pos == std::string::npos || dir.empty() || -1 == access(dir.c_str(), 0))
				{
					dir = get_current_directory();
					filename.insert(0, dir);
				}
			}

			m_file = std::fopen(filename.c_str(), "a+");

			assert(m_file != nullptr);
		}

		/**
		 * @destruct
		 */
		virtual ~logger()
		{
			if (m_file)
			{
				std::fflush(m_file);
				std::fclose(m_file);

				m_file = nullptr;
			}
		}

		static logger & get() { static logger log; return log; }

		logger & write(const char * format, ...)
		{
			if (m_file)
			{
				va_list args;
				va_start(args, format);

				std::string s = asio2::format(format, args);

				va_end(args);

				std::lock_guard<std::mutex> g(m_lock);
				std::fwrite((const void *)s.c_str(), s.length(), 1, m_file);
			}

			return (*this);
		}

		logger & flush()
		{
			if (m_file)
			{
				std::fflush(m_file);
			}

			return (*this);
		}

	private:
		/// no copy construct function
		logger(const logger&) = delete;

		/// no operator equal function
		logger& operator=(const logger&) = delete;

	protected:

		FILE * m_file = nullptr;

		std::mutex m_lock;

	};

#if defined(_MSC_VER)
#	pragma warning(pop) 
#endif

}

#endif // !__ASIO2_LOGGER_HPP__
