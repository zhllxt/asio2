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

#include <cassert>
#include <ctime>
#include <cstring>

#include <memory>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>
#include <fstream>
#include <string>

#include <asio2/util/def.hpp>
#include <asio2/util/helper.hpp>

// when use logger in multi module(eg:exe and dll),should #define CROSS_MODULE 1
#if !defined(CROSS_MODULE)
#	define CROSS_MODULE 0
#endif

/*
 # don't use "FILE fwrite ..." to handle the file,because if you declare a logger object,when call "fopen" in exe module,
   and call "fwite" in dll module,it will crash.use std::ofstream can avoid this problem.
 
 # why use "CROSS_MODULE" and create the file in a thread ? when call std::ofstream::open in exe module,and close file in
   dll module,it will crash.we should ensure that which module create the object,the object must destroy by the same module.
   so we create a threa,when need recreate the file,we post a notify event to the thread,and the thread will create the 
   file in the module which create the file.
 
 # why don't use boost::log ? beacuse if you want use boost::log on multi module(eg:exe and dll),the boost::log must be 
   compilered by shared link(a dll),and when the exe is compilered with MT/MTd,there will be a lot of compilation problems.
*/

namespace asio2
{

#if defined(_MSC_VER)
#	pragma warning(push) 
#	pragma warning(disable:4996)
#endif

	/**
	 * a simple logger
	 */
	class logger
	{
	public:
		// We define our own severity levels
		enum severity_level
		{
			trace,
			debug,
			info,
			warn,
			error,
			fatal,
			report
		};

		enum dest
		{	// constants for file opening options
			dest_mask = 0xff
		};

		static constexpr dest console = (dest)0x01;
		static constexpr dest file    = (dest)0x02;

		explicit logger(
			const char *   filename  = "",
			severity_level level     = trace,
			unsigned int   dest      = console | file,
			std::size_t    roll_size = 1024 * 1024 * 1024
		)
			: m_level(level), m_dest(dest), m_roll_size(roll_size)
		{
			if (filename)
				m_filename = filename;
			if (m_roll_size < 1024)
				m_roll_size = 1024;
			if (m_level < trace || m_level > report)
				m_level = debug;

#if (CROSS_MODULE > 0)
			_thread.swap(std::thread([this]()
			{
				while (true)
				{
					std::unique_lock<std::mutex> lock(_mtx);
					_cv.wait(lock);

					if (_is_stop)
						return;

					if (_mkflag)
					{
						mkfile();

						_mkflag = false;
					}
				}
			}));
#endif

			mkfile();
		}

		virtual ~logger()
		{
#if (CROSS_MODULE > 0)
			{
				std::unique_lock<std::mutex> lock(_mtx);
				_is_stop = true;
			}

			_cv.notify_all();

			if (_thread.joinable())
				_thread.join();
#endif

			std::lock_guard<std::mutex> g(m_lock);
			if (m_file.is_open())
			{
				m_file.flush();
				m_file.close();
			}
		}

		static logger & get() { static logger log; return log; }

		const char * level2severity(severity_level level)
		{
			switch (level)
			{
			case trace:  return "TRACE";
			case debug:  return "DEBUG";
			case info:   return "INFOR";
			case warn:   return "WARNG";
			case error:  return "ERROR";
			case fatal:  return "FATAL";
			case report: return "REPOT";
			}
			return "";
		}

		/**
		 * set the log ouput level,if you has't call this function,the defaul level is trace.
		 */
		void set_level(severity_level level)
		{
			m_level = level;
			if (m_level < trace || m_level > report)
				m_level = debug;
		}

		severity_level get_level()
		{
			return m_level;
		}

		void set_dest(unsigned int dest)
		{
			m_dest = dest;
		}

		unsigned int get_dest()
		{
			return m_dest;
		}

		/**
		 * convert the level value to string
		 */
		const char * level2string(severity_level level)
		{
			switch (level)
			{
			case trace:  return "trace";
			case debug:  return "debug";
			case info:   return "info";
			case warn:   return "warn";
			case error:  return "error";
			case fatal:  return "fatal";
			case report: return "report";
			}
			return "";
		}

		void log_trace(const char * format, ...)
		{
			if (format && *format)
			{
				va_list args;
				va_start(args, format);

				write(logger::severity_level::trace, format, args);

				va_end(args);
			}
		}

		void log_debug(const char * format, ...)
		{
			if (format && *format)
			{
				va_list args;
				va_start(args, format);

				write(logger::severity_level::debug, format, args);

				va_end(args);
			}
		}

		void log_info(const char * format, ...)
		{
			if (format && *format)
			{
				va_list args;
				va_start(args, format);

				write(logger::severity_level::info, format, args);

				va_end(args);
			}
		}

		void log_warn(const char * format, ...)
		{
			if (format && *format)
			{
				va_list args;
				va_start(args, format);

				write(logger::severity_level::warn, format, args);

				va_end(args);
			}
		}

		void log_error(const char * format, ...)
		{
			if (format && *format)
			{
				va_list args;
				va_start(args, format);

				write(logger::severity_level::error, format, args);

				va_end(args);
			}
		}

		void log_fatal(const char * format, ...)
		{
			if (format && *format)
			{
				va_list args;
				va_start(args, format);

				write(logger::severity_level::fatal, format, args);

				va_end(args);
			}
		}

		void log_report(const char * format, ...)
		{
			if (format && *format)
			{
				va_list args;
				va_start(args, format);

				write(logger::severity_level::report, format, args);

				va_end(args);
			}
		}

		/**
		 * write log
		 */
		logger & write(severity_level level, const char * format, ...)
		{
			if (level >= m_level && format && *format)
			{
				va_list args;
				va_start(args, format);

				write(level, format, args);

				va_end(args);
			}
			return (*this);
		}

		/**
		 * write log
		 */
		logger & write(severity_level level, const char * format, va_list args)
		{
			if (level >= m_level && format && *format && m_dest != 0)
			{
				static const size_t head_len = std::strlen("[") + std::strlen(level2severity(level)) + std::strlen("]") + std::strlen(" [YYYY-mm-dd HH:MM:SS] ") + std::strlen(LN);

				va_list args_copy;
				va_copy(args_copy, args);
				int body_len = std::vsnprintf(nullptr, 0, format, args_copy);

				if (body_len > 0)
				{
					size_t msg_len = head_len + body_len;
					char * buf = new char[msg_len + 1]; // + 1 : terminating null character

					time_t t;
					time(&t);                 /* Get time in seconds */
					struct tm * tm = localtime(&t);  /* Convert time to struct */
					char tmbuf[20] = { 0 };
					strftime(tmbuf, 20, "%Y-%m-%d %H:%M:%S", tm);

					size_t offset = 0, size = 0;
					const char * data = nullptr;

					data = "[";
					size = std::strlen(data);
					std::memcpy((void *)(buf + offset), (const char *)data, size);
					offset += size;

					data = level2severity(level);
					size = std::strlen(data);
					std::memcpy((void *)(buf + offset), (const char *)data, size);
					offset += size;

					data = "] [";
					size = std::strlen(data);
					std::memcpy((void *)(buf + offset), (const char *)data, size);
					offset += size;

					data = tmbuf;
					size = std::strlen(data);
					std::memcpy((void *)(buf + offset), (const char *)data, size);
					offset += size;

					data = "] ";
					size = std::strlen(data);
					std::memcpy((void *)(buf + offset), (const char *)data, size);
					offset += size;

					va_copy(args_copy, args);
					std::vsprintf((char*)(buf + offset), format, args_copy);
					offset += body_len;

					data = LN;
					size = std::strlen(data);
					std::memcpy((void *)(buf + offset), (const char *)data, size);
					offset += size;

					std::lock_guard<std::mutex> g(m_lock);
					// print log into the console window
					if ((m_dest & dest::dest_mask) & console)
					{
						std::printf("%.*s", static_cast<int>(msg_len), buf);
					}
					// save log into the file
					if ((m_dest & dest::dest_mask) & file)
					{
						if (m_file.is_open())
						{
							m_file.write(buf, msg_len);
							m_size += msg_len;

							// if file size is too large,close this file,and create a new file.
							if (m_size > m_roll_size)
							{
#if (CROSS_MODULE > 0)
								_mkflag = true;

								while (_mkflag)
								{
									std::this_thread::sleep_for(std::chrono::milliseconds(1));

									_cv.notify_one();
								}
#endif
								mkfile();
							}
						}
					}
					SAFE_DELETE(buf);
				}
			}
			return (*this);
		}

		logger & flush()
		{
			std::lock_guard<std::mutex> g(m_lock);
			if (m_file.is_open())
			{
				m_file.flush();
			}

			return (*this);
		}

	protected:
		logger & mkfile()
		{
			if (m_filename.empty())
			{
				m_filename = get_current_directory();

				m_filename += "log";
				m_filename += SLASH_STRING;

				create_directory(m_filename);

				time_t t;
				time(&t);                 /* Get time in seconds */
				struct tm * tm = localtime(&t);  /* Convert time to struct */
				char tmbuf[20] = { 0 };
				strftime(tmbuf, 20, "%Y-%m-%d %H.%M.%S", tm);

				m_filename += tmbuf;
				m_filename += ".log";
			}
			else
			{
				// Compatible with three file name parameters : 
				// abc.log
				// D:\log\abc.log
				// /usr/log/abc.log
				auto slash = m_filename.find_last_of("\\/");

				// abc.log
				if (slash == std::string::npos)
				{
					std::string name = m_filename;

					m_filename = get_current_directory();

					m_filename += "log";
					m_filename += SLASH_STRING;

					create_directory(m_filename);

					m_filename += name;
				}
				// D:\log\abc.log // /usr/log/abc.log
				else
				{
					create_directory(m_filename.substr(0, slash));
				}
			}

			if (m_file.is_open())
			{
				m_file.flush();
				m_file.close();
			}

			m_size = 0;

			m_file.open(m_filename, std::ofstream::binary | std::ofstream::out | std::ofstream::app);

			return (*this);
		}

	private:
		/// no copy construct function
		logger(const logger&) = delete;

		/// no operator equal function
		logger& operator=(const logger&) = delete;

	protected:
		severity_level m_level = severity_level::trace;

		unsigned int   m_dest = console | file;

		std::size_t    m_roll_size = 1024 * 1024 * 1024;

		std::size_t    m_size = 0;

		std::string    m_filename;

		std::ofstream  m_file;

		std::mutex     m_lock;

#if (CROSS_MODULE > 0)
		std::thread    _thread;
		std::mutex     _mtx;
		std::condition_variable _cv;
		volatile bool _is_stop = false;
		volatile bool _mkflag  = false;
#endif

	};

#if defined(_MSC_VER)
#	pragma warning(pop) 
#endif

}

#endif // !__ASIO2_LOGGER_HPP__
