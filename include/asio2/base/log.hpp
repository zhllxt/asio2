/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_LOG_HPP__
#define __ASIO2_LOG_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <cassert>

#include <string>

#include <asio2/external/predef.h>

#include <asio2/base/detail/filesystem.hpp>

#if defined(ASIO2_ENABLE_LOG)
	#if __has_include(<spdlog/spdlog.h>)
		#if ASIO2_OS_LINUX || ASIO2_OS_UNIX
			#if __has_include(<unistd.h>)
				#include <unistd.h>
			#endif
			#if __has_include(<dirent.h>)
				#include <dirent.h>
			#endif
		#elif ASIO2_OS_WINDOWS
			#ifndef WIN32_LEAN_AND_MEAN
				#define WIN32_LEAN_AND_MEAN
			#endif
			#if __has_include(<Windows.h>)
				#include <Windows.h>
			#endif
		#elif ASIO2_OS_MACOS
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
				std::string filepath;

			#if ASIO2_OS_LINUX || ASIO2_OS_UNIX
				filepath.resize(PATH_MAX);
				auto r = readlink("/proc/self/exe", (char *)filepath.data(), PATH_MAX);
				std::ignore = r; // gcc 7 warning: ignoring return value of ... [-Wunused-result]
			#elif ASIO2_OS_WINDOWS
				filepath.resize(MAX_PATH);
				filepath.resize(::GetModuleFileNameA(NULL, (LPSTR)filepath.data(), MAX_PATH));
			#elif ASIO2_OS_MACOS
				filepath.resize(PATH_MAX);
				std::uint32_t bufsize = std::uint32_t(PATH_MAX);
				_NSGetExecutablePath(filepath.data(), std::addressof(bufsize));
			#endif

				if (std::string::size_type pos = filepath.find('\0'); pos != std::string::npos)
					filepath.resize(pos);

			#if defined(_DEBUG) || defined(DEBUG)
				assert(!filepath.empty());
			#endif

				std::filesystem::path path{ filepath };

				std::string name = path.filename().string();

				std::string ext = path.extension().string();

				name.resize(name.size() - ext.size());

				filepath = path.parent_path().append(name).string() + ".asio2.log";

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

	template<typename... Args>
	inline void log(spdlog::level::level_enum lvl, fmt::format_string<Args...> fmtstr, Args&&...args)
	{
		std::shared_ptr<spdlog::logger>& loger = get_logger();

		if (loger)
		{
			loger->log(lvl, fmtstr, std::forward<Args>(args)...);
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

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_LOG_HPP__
