/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
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

/*
 * 
 * How to determine whether to use <filesystem> or <experimental/filesystem>
 * https://stackoverflow.com/questions/53365538/how-to-determine-whether-to-use-filesystem-or-experimental-filesystem/53365539#53365539
 * 
 * 
 */

// We haven't checked which filesystem to include yet
#ifndef INCLUDE_STD_FILESYSTEM_EXPERIMENTAL

// Check for feature test macro for <filesystem>
#   if defined(__cpp_lib_filesystem)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0

// Check for feature test macro for <experimental/filesystem>
#   elif defined(__cpp_lib_experimental_filesystem)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1

// We can't check if headers exist...
// Let's assume experimental to be safe
#   elif !defined(__has_include)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1

// Check if the header "<filesystem>" exists
#   elif __has_include(<filesystem>)

// If we're compiling on Visual Studio and are not compiling with C++17, we need to use experimental
#       ifdef _MSC_VER

// Check and include header that defines "_HAS_CXX17"
#           if __has_include(<yvals_core.h>)
#               include <yvals_core.h>

// Check for enabled C++17 support
#               if defined(_HAS_CXX17) && _HAS_CXX17
// We're using C++17, so let's use the normal version
#                   define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#               endif
#           endif

// If the marco isn't defined yet, that means any of the other VS specific checks failed, so we need to use experimental
#           ifndef INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
#               define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#           endif

// Not on Visual Studio. Let's use the normal version
#       else // #ifdef _MSC_VER
#           define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#       endif

// Check if the header "<filesystem>" exists
#   elif __has_include(<experimental/filesystem>)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1

// Fail if neither header is available with a nice error message
#   else
#       error Could not find system header "<filesystem>" or "<experimental/filesystem>"
#   endif

// We priously determined that we need the exprimental version
#   if INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
// Include it
#       include <experimental/filesystem>

// We need the alias from std::experimental::filesystem to std::filesystem
namespace std {
    namespace filesystem = experimental::filesystem;
}

// We have a decent compiler and can use the normal version
#   else
// Include it
#       include <filesystem>
#   endif

#endif // #ifndef INCLUDE_STD_FILESYSTEM_EXPERIMENTAL

#ifndef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY
#endif
#include <fmt/format.h>
#include <fmt/chrono.h>

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
 # don't use "FILE fwrite ..." to handle the file,because if you declare a logger object,
   when call "fopen" in exe module, and call "fwite" in dll module,it will crash. use 
   std::ofstream can avoid this problem.
 
 # why use "ASIO2_LOGGER_MULTI_MODULE" and create the file in a thread ? when call 
   std::ofstream::open in exe module,and close file in dll module,it will crash.
   we should ensure that which module create the object,the object must destroy by
   the same module. so we create a thread,when need recreate the file,we post a 
   notify event to the thread,and the thread will create the file in the module
   which create the file.
 
 # why don't use boost::log ? beacuse if you want use boost::log on multi module
   (eg:exe and dll),the boost::log must be compilered by shared link(a dll),and
   when the exe is compilered with MT/MTd,there will be a lot of compilation problems.
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

		this->endl_ = { '\n' };
		this->preferred_ = { '/' };

		#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || \
			defined(_WINDOWS_) || defined(__WINDOWS__) || defined(__TOS_WIN__)
			this->endl_ = { '\r','\n' };
			this->preferred_ = { '\\' };
		#endif

		#if defined(ASIO2_LOGGER_MULTI_MODULE)
			std::thread t([this]() mutable
			{
				while (true)
				{
					std::unique_lock<std::mutex> lock(this->mtx_);
					this->cv_.wait(lock, [this]() { return this->is_stop_ || this->mkflag_; });

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

			std::lock_guard<std::mutex> g(this->mutex_);
			if (this->file_.is_open())
			{
				this->file_.flush();
				this->file_.close();
			}
		}

		static logger & get() noexcept { static logger log; return log; }

		inline const char * level2severity(severity_level level) noexcept
		{
			static const char * name[]{ "TRACE","DEBUG","INFOR","WARNS","ERROR","FATAL","REPOT", };

			return ((level >= severity_level::trace && level <= severity_level::report) ?
				name[static_cast<typename std::underlying_type<severity_level>::type>(level)] : "");
		}

		/**
		 * set the log ouput level,if you has't call this function,the defaul level is trace.
		 */
		inline logger & set_level(severity_level level) noexcept
		{
			std::lock_guard<std::mutex> g(this->mutex_);
			this->level_ = level;
			if (this->level_ < severity_level::trace || this->level_ > severity_level::report)
				this->level_ = severity_level::debug;
			return (*this);
		}

		/**
		 * set the log ouput level,if you has't call this function,the defaul level is trace.
		 * same as set_level
		 */
		inline logger & level(severity_level level) noexcept
		{
			return this->set_level(level);
		}

		/**
		 * get the log ouput level
		 */
		inline severity_level get_level() noexcept
		{
			std::lock_guard<std::mutex> g(this->mutex_);
			return this->level_;
		}

		/**
		 * get the log ouput level, same as get_level
		 */
		inline severity_level level() noexcept
		{
			std::lock_guard<std::mutex> g(this->mutex_);
			return this->level_;
		}

		/**
		 * set the log ouput dest, console or file or both.
		 */
		inline logger & set_dest(unsigned int dest) noexcept
		{
			std::lock_guard<std::mutex> g(this->mutex_);
			this->dest_ = dest;
			return (*this);
		}

		/**
		 * set the log ouput dest, console or file or both. same as set_dest
		 */
		inline logger & dest(unsigned int dest) noexcept
		{
			return this->set_dest(dest);
		}

		/**
		 * get the log ouput dest, console or file or both.
		 */
		inline unsigned int get_dest() noexcept
		{
			std::lock_guard<std::mutex> g(this->mutex_);
			return this->dest_;
		}

		/**
		 * get the log ouput dest, console or file or both. same as get_dest
		 */
		inline unsigned int dest() noexcept
		{
			std::lock_guard<std::mutex> g(this->mutex_);
			return this->dest_;
		}

		/**
		 * set a user custom function log target.
		 * @Fun : void(const std::string & text)
		 */
		template<class Fun>
		inline logger & set_target(Fun&& target) noexcept
		{
			std::lock_guard<std::mutex> g(this->mutex_);
			this->target_ = std::forward<Fun>(target);
			return (*this);
		}

		/**
		 * set a user custom function log target. same as set_target
		 * @Fun : void(const std::string & text)
		 */
		template<class Fun>
		inline logger & target(Fun&& target) noexcept
		{
			return this->set_target(std::forward<Fun>(target));
		}

		/**
		 * get the user custom function log target.
		 */
		inline std::function<void(const std::string & text)> & get_target() noexcept
		{
			return this->target_;
		}

		/**
		 * get the user custom function log target. same as get_target
		 */
		inline std::function<void(const std::string & text)> & target() noexcept
		{
			return this->target_;
		}

		/**
		 * convert the level value to string
		 */
		inline const char * level2string(severity_level level) noexcept
		{
			static const char * name[]{ "trace","debug","info","warn","error","fatal","report", };

			return ((level >= severity_level::trace && level <= severity_level::report) ?
				name[static_cast<typename std::underlying_type<severity_level>::type>(level)] : "");
		}

		template <typename S, typename... Args>
		inline void log_trace(const S& format_str, Args&&... args)
		{
			if (logger::severity_level::trace >= this->level_)
			{
				this->log(logger::severity_level::trace, format_str, std::forward<Args>(args)...);
			}
		}

		template <typename S, typename... Args>
		inline void log_debug(const S& format_str, Args&&... args)
		{
			if (logger::severity_level::debug >= this->level_)
			{
				this->log(logger::severity_level::debug, format_str, std::forward<Args>(args)...);
			}
		}

		template <typename S, typename... Args>
		inline void log_info(const S& format_str, Args&&... args)
		{
			if (logger::severity_level::info >= this->level_)
			{
				this->log(logger::severity_level::info, format_str, std::forward<Args>(args)...);
			}
		}

		template <typename S, typename... Args>
		inline void log_warn(const S& format_str, Args&&... args)
		{
			if (logger::severity_level::warn >= this->level_)
			{
				this->log(logger::severity_level::warn, format_str, std::forward<Args>(args)...);
			}
		}

		template <typename S, typename... Args>
		inline void log_error(const S& format_str, Args&&... args)
		{
			if (logger::severity_level::error >= this->level_)
			{
				this->log(logger::severity_level::error, format_str, std::forward<Args>(args)...);
			}
		}

		template <typename S, typename... Args>
		inline void log_fatal(const S& format_str, Args&&... args)
		{
			if (logger::severity_level::fatal >= this->level_)
			{
				this->log(logger::severity_level::fatal, format_str, std::forward<Args>(args)...);
			}
		}

		template <typename S, typename... Args>
		inline void log_report(const S& format_str, Args&&... args)
		{
			if (logger::severity_level::report >= this->level_)
			{
				this->log(logger::severity_level::report, format_str, std::forward<Args>(args)...);
			}
		}

		template <typename S, typename... Args>
		void log(severity_level level, const S& format_str, Args&&... args)
		{
			std::lock_guard<std::mutex> g(this->mutex_);

			if (level < this->level_)
				return;

			std::time_t t = std::time(nullptr);
			std::string content;
			if constexpr (sizeof...(Args) == 0)
			{
				content =
					fmt::format("[{}] [{:%Y-%m-%d %H:%M:%S}] ", level2severity(level), *std::localtime(&t)) +
					fmt::format("{}", format_str) +
					this->endl_;
			}
			else
			{
				content =
					fmt::format("[{}] [{:%Y-%m-%d %H:%M:%S}] ", level2severity(level), *std::localtime(&t)) +
					fmt::vformat(format_str, fmt::make_format_args(std::forward<Args>(args)...)) +
					this->endl_;
			}

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
				//std::printf("%.*s", static_cast<int>(content.size()), content.data());
				fmt::print("{}", content);
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

		inline void flush()
		{
			std::lock_guard<std::mutex> g(this->mutex_);
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
				strftime(tmbuf, 20, "%Y-%m-%d %H.%M.%S", std::addressof(tm));

				this->filename_ += tmbuf;
				this->filename_ += ".log";
			}
			else
			{
				// Compatible with three file name parameters : 
				// abc.log
				// D:\log\abc.log
				// /usr/log/abc.log
				if (std::filesystem::is_directory(filename_))
				{
					if (this->filename_.back() != '\\' && this->filename_.back() != '/')
						this->filename_ += this->preferred_;

					this->filename_ += "log";
					this->filename_ += this->preferred_;

					std::filesystem::create_directories(std::filesystem::path(this->filename_));

					time_t t;
					time(&t);                 /* Get time in seconds */
					struct tm tm = *localtime(&t);  /* Convert time to struct */
					char tmbuf[20] = { 0 };
					strftime(tmbuf, 20, "%Y-%m-%d %H.%M.%S", std::addressof(tm));

					this->filename_ += tmbuf;
					this->filename_ += ".log";
				}
				else
				{
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
						std::filesystem::create_directories(
							std::filesystem::path(this->filename_.substr(0, slash)));
					}
				}
			}

			if (this->file_.is_open())
			{
				this->file_.flush();
				this->file_.close();
			}

			this->size_ = 0;

			this->file_.open(this->filename_,
				std::ofstream::binary | std::ofstream::out | std::ofstream::app);

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

		std::mutex     mutex_;

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
