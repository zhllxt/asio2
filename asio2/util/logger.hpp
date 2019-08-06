/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_LOGGER_HPP__
#define __ASIO2_LOGGER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <ctime>
#include <cstring>
#include <cstdarg>

#include <memory>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

/*
 * mutex,thread:
 * Linux platform needs to add -lpthread option in link libraries
 * filesystem:
 * Linux platform needs to add -lstdc++fs option in link libraries, If it still doesn't work,
 * try adding stdc++fs to Library Dependencies.
 * try search file for libstdc++fs.a
 */

// when use logger in multi module(eg:exe and dll),should #define ASIO2_LOGGER_MULTI_MODULE
// #define ASIO2_LOGGER_MULTI_MODULE

/*
 # don't use "FILE fwrite ..." to handle the file,because if you declare a logger object,when call "fopen" in exe module,
   and call "fwite" in dll module,it will crash.use std::ofstream can avoid this problem.
 
 # why use "ASIO2_LOGGER_MULTI_MODULE" and create the file in a thread ? when call std::ofstream::open in exe module,and close file in
   dll module,it will crash.we should ensure that which module create the object,the object must destroy by the same module.
   so we create a thread,when need recreate the file,we post a notify event to the thread,and the thread will create the 
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
		enum class severity_level : std::int8_t
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
			const std::string & filename = std::string(),
			severity_level level = severity_level::trace,
			unsigned int   dest = console | file,
			std::size_t    roll_size = 1024 * 1024 * 1024
		)
			: filename_(filename), level_(level), dest_(dest), roll_size_(roll_size)
		{
			if (this->roll_size_ < 1024)
				this->roll_size_ = 1024;
			if (this->level_ < severity_level::trace || this->level_ > severity_level::report)
				this->level_ = severity_level::debug;

#if defined(__unix__) || defined(__linux__)
			this->endl_ = { '\n' };
			this->preferred_ = { '/' };
#elif defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_) || defined(WIN32)
			this->endl_ = { '\r','\n' };
			this->preferred_ = { '\\' };
#	endif

#if defined(ASIO2_LOGGER_MULTI_MODULE)
			std::thread t([this]()
			{
				while (true)
				{
					std::unique_lock<std::mutex> lock(this->mtx_);
					this->cv_.wait(lock);

					if (this->is_stop_)
						return;

					if (this->mkflag_)
					{
						this->mkfile();

						this->mkflag_ = false;
					}
				}
			});
			this->thread_.swap(t);
#endif

			this->mkfile();
		}

		~logger()
		{
#if defined(ASIO2_LOGGER_MULTI_MODULE)
			{
				std::unique_lock<std::mutex> lock(this->mtx_);
				this->is_stop_ = true;
			}

			this->cv_.notify_all();

			if (this->thread_.joinable())
				this->thread_.join();
#endif

			std::lock_guard<std::mutex> g(this->lock_);
			if (this->file_.is_open())
			{
				this->file_.flush();
				this->file_.close();
			}
		}

		static logger & get() { static logger log; return log; }

		inline const char * level2severity(severity_level level)
		{
			switch (level)
			{
			case severity_level::trace:  return "TRACE";
			case severity_level::debug:  return "DEBUG";
			case severity_level::info:   return "INFOR";
			case severity_level::warn:   return "WARNG";
			case severity_level::error:  return "ERROR";
			case severity_level::fatal:  return "FATAL";
			case severity_level::report: return "REPOT";
			}
			return "";
		}

		/**
		 * set the log ouput level,if you has't call this function,the defaul level is trace.
		 */
		inline logger & level(severity_level level)
		{
			this->level_ = level;
			if (this->level_ < severity_level::trace || this->level_ > severity_level::report)
				this->level_ = severity_level::debug;
			return (*this);
		}

		inline severity_level level()
		{
			return this->level_;
		}

		inline logger & dest(unsigned int dest)
		{
			this->dest_ = dest;
			return (*this);
		}

		inline unsigned int dest()
		{
			return this->dest_;
		}

		inline logger & target(const std::function<void(const std::string & text)> & target)
		{
			this->target_ = target;
			return (*this);
		}

		inline std::function<void(const std::string & text)> & target()
		{
			return this->target_;
		}

		/**
		 * convert the level value to string
		 */
		inline const char * level2string(severity_level level)
		{
			switch (level)
			{
			case severity_level::trace:  return "trace";
			case severity_level::debug:  return "debug";
			case severity_level::info:   return "info";
			case severity_level::warn:   return "warn";
			case severity_level::error:  return "error";
			case severity_level::fatal:  return "fatal";
			case severity_level::report: return "report";
			}
			return "";
		}

		inline void log_trace(const char * format, ...)
		{
			if (logger::severity_level::trace >= this->level_ && format && *format)
			{
				va_list args;
				va_start(args, format);

				this->logv(logger::severity_level::trace, format, args);

				va_end(args);
			}
		}

		inline void log_debug(const char * format, ...)
		{
			if (logger::severity_level::debug >= this->level_ && format && *format)
			{
				va_list args;
				va_start(args, format);

				this->logv(logger::severity_level::debug, format, args);

				va_end(args);
			}
		}

		inline void log_info(const char * format, ...)
		{
			if (logger::severity_level::info >= this->level_ && format && *format)
			{
				va_list args;
				va_start(args, format);

				this->logv(logger::severity_level::info, format, args);

				va_end(args);
			}
		}

		inline void log_warn(const char * format, ...)
		{
			if (logger::severity_level::warn >= this->level_ && format && *format)
			{
				va_list args;
				va_start(args, format);

				this->logv(logger::severity_level::warn, format, args);

				va_end(args);
			}
		}

		inline void log_error(const char * format, ...)
		{
			if (logger::severity_level::error >= this->level_ && format && *format)
			{
				va_list args;
				va_start(args, format);

				this->logv(logger::severity_level::error, format, args);

				va_end(args);
			}
		}

		inline void log_fatal(const char * format, ...)
		{
			if (logger::severity_level::fatal >= this->level_ && format && *format)
			{
				va_list args;
				va_start(args, format);

				this->logv(logger::severity_level::fatal, format, args);

				va_end(args);
			}
		}

		inline void log_report(const char * format, ...)
		{
			if (logger::severity_level::report >= this->level_ && format && *format)
			{
				va_list args;
				va_start(args, format);

				this->logv(logger::severity_level::report, format, args);

				va_end(args);
			}
		}

		inline void log(severity_level level, const char * format, ...)
		{
			if (level >= this->level_ && format && *format)
			{
				va_list args;
				va_start(args, format);

				this->logv(level, format, args);

				va_end(args);
			}
		}

		inline void logv(severity_level level, const char * format, va_list args)
		{
			if (level >= this->level_ && format && *format && this->dest_ != 0)
			{
				va_list args_copy;
				va_copy(args_copy, args);

				std::string s;
				int len = std::vsnprintf(nullptr, 0, format, args_copy);
				if (len > 0)
				{
					s.resize(len);

					va_copy(args_copy, args);
					std::vsprintf((char*)s.data(), format, args_copy);

					this->log(level, s);
				}
			}
		}

		void log(severity_level level, const std::string & s)
		{
			if (level >= this->level_)
			{
				static const std::size_t head_len = std::strlen("[") + std::strlen(level2severity(level)) +
					std::strlen("]") + std::strlen(" [YYYY-mm-dd HH:MM:SS] ") + this->endl_.size();

				std::string content;
				
				content.reserve(head_len + s.size());

				time_t t;
				time(&t);                 /* Get time in seconds */
				struct tm tm = *localtime(&t);  /* Convert time to struct */
				char tmbuf[20] = { 0 };
				strftime(tmbuf, 20, "%Y-%m-%d %H:%M:%S", &tm);

				content += "[";
				content += level2severity(level);
				content += "] [";
				content += tmbuf;
				content += "] ";
				content += s;
				content += this->endl_;

				std::lock_guard<std::mutex> g(this->lock_);
				// call user defined target function
				if (this->target_)
				{
					this->target_(content);
				}
				// print log into the console window
				if ((this->dest_ & dest::dest_mask) & console)
				{
					// don't use std::cout and printf together.
					//std::cout << content << std::endl;
					std::printf("%.*s", static_cast<int>(content.size()), content.data());
				}
				// save log into the file
				if ((this->dest_ & dest::dest_mask) & file)
				{
					if (this->file_.is_open())
					{
						this->file_.write(content.data(), content.size());
						this->size_ += content.size();

						// if file size is too large,close this file,and create a new file.
						if (this->size_ > this->roll_size_)
						{
#if defined(ASIO2_LOGGER_MULTI_MODULE)
							this->mkflag_ = true;

							while (this->mkflag_)
							{
								std::this_thread::sleep_for(std::chrono::milliseconds(1));

								this->cv_.notify_one();
							}
#endif
							this->mkfile();
						}
					}
				}
			}
		}

		inline void flush()
		{
			std::lock_guard<std::mutex> g(this->lock_);
			if (this->file_.is_open())
			{
				this->file_.flush();
			}
		}

	protected:
		logger & mkfile()
		{
			if (this->filename_.empty())
			{
				this->filename_ = std::filesystem::current_path().string();

				if (this->filename_.back() != '\\' && this->filename_.back() != '/')
					this->filename_ += this->preferred_;

				this->filename_ += "log";
				this->filename_ += this->preferred_;

				std::filesystem::create_directories(std::filesystem::path(this->filename_));

				time_t t;
				time(&t);                 /* Get time in seconds */
				struct tm tm = *localtime(&t);  /* Convert time to struct */
				char tmbuf[20] = { 0 };
				strftime(tmbuf, 20, "%Y-%m-%d %H.%M.%S", &tm);

				this->filename_ += tmbuf;
				this->filename_ += ".log";
			}
			else
			{
				// Compatible with three file name parameters : 
				// abc.log
				// D:\log\abc.log
				// /usr/log/abc.log
				std::string::size_type slash = this->filename_.find_last_of("\\/");

				// abc.log
				if (slash == std::string::npos)
				{
					std::string name = this->filename_;

					this->filename_ = std::filesystem::current_path().string();

					if (this->filename_.back() != '\\' && this->filename_.back() != '/')
						this->filename_ += this->preferred_;

					this->filename_ += "log";
					this->filename_ += this->preferred_;

					std::filesystem::create_directories(std::filesystem::path(this->filename_));

					this->filename_ += name;
				}
				// D:\log\abc.log // /usr/log/abc.log
				else
				{
					std::filesystem::create_directories(std::filesystem::path(this->filename_.substr(0, slash)));
				}
			}

			if (this->file_.is_open())
			{
				this->file_.flush();
				this->file_.close();
			}

			this->size_ = 0;

			this->file_.open(this->filename_, std::ofstream::binary | std::ofstream::out | std::ofstream::app);

			return (*this);
		}

	private:
		/// no copy construct function
		logger(const logger&) = delete;

		/// no operator equal function
		logger& operator=(const logger&) = delete;

	protected:
		std::string    filename_;

		severity_level level_ = severity_level::trace;

		unsigned int   dest_ = console | file;

		std::size_t    roll_size_ = 1024 * 1024 * 1024;

		std::size_t    size_ = 0;

		std::ofstream  file_;

		std::string    endl_;

		std::string    preferred_;

		std::mutex     lock_;

		std::function<void(const std::string & text)> target_;

#if defined(ASIO2_LOGGER_MULTI_MODULE)
		std::thread    thread_;
		std::mutex     mtx_;
		std::condition_variable cv_;
		bool           is_stop_ = false;
		bool           mkflag_  = false;
#endif

	};

#if defined(_MSC_VER)
#	pragma warning(pop) 
#endif

}

#endif // !__ASIO2_LOGGER_HPP__
