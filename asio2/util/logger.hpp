/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
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

#define FMT_HEADER_ONLY
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
			static const char * name[]{ "TRACE","DEBUG","INFOR","WARNS","ERROR","FATAL","REPOT", };

			return ((level >= severity_level::trace && level <= severity_level::report) ?
				name[static_cast<typename std::underlying_type<severity_level>::type>(level)] : "");
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
					fmt::format(format_str, std::forward<Args>(args)...) +
					this->endl_;
			}

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
					std::filesystem::create_directories(
						std::filesystem::path(this->filename_.substr(0, slash)));
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
