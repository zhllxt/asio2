/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_LOG_HPP__
#define __ASIO2_LOG_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cassert>

#include <string>

#include <asio2/external/magic_enum.hpp>

#include <asio2/config.hpp>
#include <asio2/bho/predef.h>

#if defined(ASIO2_ENABLE_LOG)
	#if __has_include(<spdlog/spdlog.h>)
		#if BHO_OS_LINUX || BHO_OS_UNIX
			#if __has_include(<unistd.h>)
				#include <unistd.h>
			#endif
			#if __has_include(<dirent.h>)
				#include <dirent.h>
			#endif
		#elif BHO_OS_WINDOWS
			#ifndef WIN32_LEAN_AND_MEAN
				#define WIN32_LEAN_AND_MEAN
			#endif
			#if __has_include(<Windows.h>)
				#include <Windows.h>
			#endif
		#elif BHO_OS_MACOS
			#if __has_include(<mach-o/dyld.h>)
				#include <mach-o/dyld.h>
			#endif
		#endif

		#include <spdlog/spdlog.h>
		#include <spdlog/sinks/basic_file_sink.h>
	#else
		#undef ASIO2_ENABLE_LOG
	#endif
#endif

namespace asio2::detail
{
#if defined(ASIO2_ENABLE_LOG)
	static std::shared_ptr<spdlog::logger>& get_logger()
	{
		static std::shared_ptr<spdlog::logger> logger;
		static std::mutex mutex;

		if (!logger)
		{
			std::lock_guard guard(mutex);

			if (!logger)
			{
				std::string                  filepath;

			#if BHO_OS_LINUX || BHO_OS_UNIX
				filepath.resize(PATH_MAX);
				readlink("/proc/self/exe", (char *)filepath.data(), PATH_MAX);
			#elif BHO_OS_WINDOWS
				filepath.resize(MAX_PATH);
				filepath.resize(::GetModuleFileNameA(NULL, (LPSTR)filepath.data(), MAX_PATH));
			#elif BHO_OS_MACOS
				filepath.resize(PATH_MAX);
				std::uint32_t bufsize = std::uint32_t(PATH_MAX);
				_NSGetExecutablePath(filepath.data(), &bufsize);
			#endif

				std::string::size_type pos;

				pos = filepath.find_last_of("\\/");
				if (pos != std::string::npos)
					filepath = filepath.substr(pos + 1);

				pos = filepath.rfind('.');
				if (pos != std::string::npos)
					filepath = filepath.substr(0, pos);

				// only for linux
				pos = filepath.find('\0');
				if (pos != std::string::npos)
					filepath = filepath.substr(0, pos);

			#if defined(_DEBUG) || defined(DEBUG)
				assert(!filepath.empty());
			#endif

				filepath += ".log";
				filepath.insert(0, "asio2_");

				auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filepath, true);
				file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
				auto loger = std::make_shared<spdlog::logger>("ASIO2_LOG", std::move(file_sink));
				loger->set_level(spdlog::level::debug);
				loger->flush_on(spdlog::level::err);

				logger = std::move(loger);
			}
		}

		return logger;
	}

	template<typename FormatString, typename... Args>
	inline void log(spdlog::level::level_enum lvl, const FormatString &fmt, Args&&...args)
	{
		std::shared_ptr<spdlog::logger>& loger = get_logger();

		if (loger)
		{
			loger->log(lvl, fmt, std::forward<Args>(args)...);
		}
	}
#else
	template <typename... Ts>
	inline constexpr void log(Ts const& ...) noexcept {}
#endif

	class [[maybe_unused]] external_linkaged_log
	{
	public:
		[[maybe_unused]] static bool& has_unexpected_behavior() noexcept
		{
			static bool flag = false;
			return flag;
		}
	};

	namespace internal_linkaged_log
	{
		[[maybe_unused]] static bool& has_unexpected_behavior() noexcept
		{
			static bool flag = false;
			return flag;
		}
	}

	// just for : debug, find bug ...
	[[maybe_unused]] static bool& has_unexpected_behavior() noexcept
	{
		return external_linkaged_log::has_unexpected_behavior();
	}
}

#ifdef ASIO2_LOG
	static_assert(false, "Unknown ASIO2_LOG definition will affect the relevant functions of this program.");
#endif

#if defined(ASIO2_ENABLE_LOG)
	#define ASIO2_LOG(...) asio2::detail::log(__VA_ARGS__)
#else
	#define ASIO2_LOG(...) ((void)0)
#endif

#endif // !__ASIO2_LOG_HPP__
