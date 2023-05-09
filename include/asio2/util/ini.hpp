/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_INI_HPP__
#define __ASIO2_INI_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if defined(__GNUC__) || defined(__GNUG__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Warray-bounds"
#endif

#include <cstring>
#include <cctype>
#include <cstdarg>
#include <clocale>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <ctime>

#include <memory>
#include <string>
#include <locale>
#include <string_view>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <fstream>
#include <sstream>
#include <type_traits>
#include <system_error>
#include <limits>
#include <algorithm>
#include <tuple>
#include <chrono>

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


// when compiled with "Visual Studio 2017 - Windows XP (v141_xp)"
// there is hasn't shared_mutex
#ifndef ASIO2_HAS_SHARED_MUTEX
	#if defined(_MSC_VER)
		#if defined(_HAS_SHARED_MUTEX)
			#if _HAS_SHARED_MUTEX
				#define ASIO2_HAS_SHARED_MUTEX 1
				#define asio2_shared_mutex std::shared_mutex
				#define asio2_shared_lock  std::shared_lock
				#define asio2_unique_lock  std::unique_lock
			#else
				#define ASIO2_HAS_SHARED_MUTEX 0
				#define asio2_shared_mutex std::mutex
				#define asio2_shared_lock  std::lock_guard
				#define asio2_unique_lock  std::lock_guard
			#endif
		#else
				#define ASIO2_HAS_SHARED_MUTEX 1
				#define asio2_shared_mutex std::shared_mutex
				#define asio2_shared_lock  std::shared_lock
				#define asio2_unique_lock  std::unique_lock
		#endif
	#else
				#define ASIO2_HAS_SHARED_MUTEX 1
				#define asio2_shared_mutex std::shared_mutex
				#define asio2_shared_lock  std::shared_lock
				#define asio2_unique_lock  std::unique_lock
	#endif
#endif


#if defined(unix) || defined(__unix) || defined(_XOPEN_SOURCE) || defined(_POSIX_SOURCE) || \
	defined(linux) || defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
#	if __has_include(<unistd.h>)
#		include <unistd.h>
#	endif
#	if __has_include(<sys/types.h>)
#		include <sys/types.h>
#	endif
#	if __has_include(<sys/stat.h>)
#		include <sys/stat.h>
#	endif
#	if __has_include(<dirent.h>)
#		include <dirent.h>
#	endif
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || \
	defined(_WINDOWS_) || defined(__WINDOWS__) || defined(__TOS_WIN__)
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	if __has_include(<Windows.h>)
#		include <Windows.h>
#	endif
#	if __has_include(<tchar.h>)
#		include <tchar.h>
#	endif
#	if __has_include(<io.h>)
#		include <io.h>
#	endif
#	if __has_include(<direct.h>)
#		include <direct.h>
#	endif
#elif defined(__APPLE__) && defined(__MACH__)
#	if __has_include(<mach-o/dyld.h>)
#		include <mach-o/dyld.h>
#	endif
#endif

/*
 * mutex:
 * Linux platform needs to add -lpthread option in link libraries
 */

namespace asio2
{
	template<class T>
	struct convert;

	template<>
	struct convert<bool>
	{
		/**
		 * @brief Returns `true` if two strings are equal, using a case-insensitive comparison.
		 */
		template<typename = void>
		inline static bool iequals(std::string_view lhs, std::string_view rhs) noexcept
		{
			auto n = lhs.size();
			if (rhs.size() != n)
				return false;
			auto p1 = lhs.data();
			auto p2 = rhs.data();
			char a, b;
			// fast loop
			while (n--)
			{
				a = *p1++;
				b = *p2++;
				if (a != b)
					goto slow;
			}
			return true;
		slow:
			do
			{
				if (std::tolower(a) != std::tolower(b))
					return false;
				a = *p1++;
				b = *p2++;
			} while (n--);
			return true;
		}

		template<
			class CharT,
			class Traits = std::char_traits<CharT>,
			class Allocator = std::allocator<CharT>
		>
		inline static bool stov(std::basic_string<CharT, Traits, Allocator>& val)
		{
			if (iequals(val, "true"))
				return true;
			if (iequals(val, "false"))
				return false;
			return (!(std::stoi(val) == 0));
		}
	};

	template<>
	struct convert<char>
	{
		template<class ...Args>
		inline static char stov(Args&&... args)
		{ return static_cast<char>(std::stoi(std::forward<Args>(args)...)); }
	};

	template<>
	struct convert<signed char>
	{
		template<class ...Args>
		inline static signed char stov(Args&&... args)
		{ return static_cast<signed char>(std::stoi(std::forward<Args>(args)...)); }
	};

	template<>
	struct convert<unsigned char>
	{
		template<class ...Args>
		inline static unsigned char stov(Args&&... args)
		{ return static_cast<unsigned char>(std::stoul(std::forward<Args>(args)...)); }
	};

	template<>
	struct convert<short>
	{
		template<class ...Args>
		inline static short stov(Args&&... args)
		{ return static_cast<short>(std::stoi(std::forward<Args>(args)...)); }
	};

	template<>
	struct convert<unsigned short>
	{
		template<class ...Args>
		inline static unsigned short stov(Args&&... args)
		{ return static_cast<unsigned short>(std::stoul(std::forward<Args>(args)...)); }
	};

	template<>
	struct convert<int>
	{
		template<class ...Args>
		inline static int stov(Args&&... args)
		{ return std::stoi(std::forward<Args>(args)...); }
	};

	template<>
	struct convert<unsigned int>
	{
		template<class ...Args>
		inline static unsigned int stov(Args&&... args)
		{ return static_cast<unsigned int>(std::stoul(std::forward<Args>(args)...)); }
	};

	template<>
	struct convert<long>
	{
		template<class ...Args>
		inline static long stov(Args&&... args)
		{ return std::stol(std::forward<Args>(args)...); }
	};

	template<>
	struct convert<unsigned long>
	{
		template<class ...Args>
		inline static unsigned long stov(Args&&... args)
		{ return std::stoul(std::forward<Args>(args)...); }
	};

	template<>
	struct convert<long long>
	{
		template<class ...Args>
		inline static long long stov(Args&&... args)
		{ return std::stoll(std::forward<Args>(args)...); }
	};

	template<>
	struct convert<unsigned long long>
	{
		template<class ...Args>
		inline static unsigned long long stov(Args&&... args)
		{ return std::stoull(std::forward<Args>(args)...); }
	};

	template<>
	struct convert<float>
	{
		template<class ...Args>
		inline static float stov(Args&&... args)
		{ return std::stof(std::forward<Args>(args)...); }
	};

	template<>
	struct convert<double>
	{
		template<class ...Args>
		inline static double stov(Args&&... args)
		{ return std::stod(std::forward<Args>(args)...); }
	};

	template<>
	struct convert<long double>
	{
		template<class ...Args>
		inline static long double stov(Args&&... args)
		{ return std::stold(std::forward<Args>(args)...); }
	};

	template<class CharT, class Traits, class Allocator>
	struct convert<std::basic_string<CharT, Traits, Allocator>>
	{
		template<class ...Args>
		inline static std::basic_string<CharT, Traits, Allocator> stov(Args&&... args)
		{ return std::basic_string<CharT, Traits, Allocator>(std::forward<Args>(args)...); }
	};

	template<class CharT, class Traits>
	struct convert<std::basic_string_view<CharT, Traits>>
	{
		template<class ...Args>
		inline static std::basic_string_view<CharT, Traits> stov(Args&&... args)
		{ return std::basic_string_view<CharT, Traits>(std::forward<Args>(args)...); }
	};

	template<class Rep, class Period>
	struct convert<std::chrono::duration<Rep, Period>>
	{
		// referenced from: C# TimeSpan
		// 30                 - 30 seconds
		// 00:00:00.0000036   - 36 milliseconds
		// 00:00:00           - 0 seconds
		// 2.10:36:45         - 2 days 10 hours 36 minutes 45 seconds
		// 2.00:00:00.0000036 - 
		template<class S>
		inline static std::chrono::duration<Rep, Period> stov(S&& s)
		{
			std::size_t n1 = s.find(':');

			if (n1 == std::string::npos)
				return std::chrono::seconds(std::stoll(s));

			int day = 0, hour = 0, min = 0, sec = 0, msec = 0;

			std::size_t m1 = s.find('.');

			if (m1 < n1)
			{
				day = std::stoi(s.substr(0, m1));
				s.erase(0, m1 + 1);
			}

			n1 = s.find(':');
			hour = std::stoi(s.substr(0, n1));
			s.erase(0, n1 + 1);

			n1 = s.find(':');
			min = std::stoi(s.substr(0, n1));
			s.erase(0, n1 + 1);

			n1 = s.find('.');
			sec = std::stoi(s.substr(0, n1));

			if (n1 != std::string::npos)
			{
				s.erase(0, n1 + 1);

				msec = std::stoi(s);
			}

			return
				std::chrono::hours(day * 24) +
				std::chrono::hours(hour) +
				std::chrono::minutes(min) +
				std::chrono::seconds(sec) +
				std::chrono::milliseconds(msec);
		}
	};

	template<class Clock, class Duration>
	struct convert<std::chrono::time_point<Clock, Duration>>
	{
		template<class S>
		inline static std::chrono::time_point<Clock, Duration> stov(S&& s)
		{
			std::stringstream ss;
			ss << std::forward<S>(s);

			std::tm t{};

			if (s.find('/') != std::string::npos)
				ss >> std::get_time(&t, "%m/%d/%Y %H:%M:%S");
			else
				ss >> std::get_time(&t, "%Y-%m-%d %H:%M:%S");

			return Clock::from_time_t(std::mktime(&t));
		}
	};
}

namespace asio2
{
	// use namespace asio2::detail::util to avoid conflict with asio2::detail in file "asio2/base/detail/util.hpp"
	// is_string_view ...
	namespace detail::util
	{
		template<typename, typename = void>
		struct is_fstream : std::false_type {};

		template<typename T>
		struct is_fstream<T, std::void_t<typename T::char_type, typename T::traits_type,
			typename std::enable_if_t<std::is_same_v<T,
			std::basic_fstream<typename T::char_type, typename T::traits_type>>>>> : std::true_type {};

		template<class T>
		inline constexpr bool is_fstream_v = is_fstream<std::remove_cv_t<std::remove_reference_t<T>>>::value;

		template<typename, typename = void>
		struct is_ifstream : std::false_type {};

		template<typename T>
		struct is_ifstream<T, std::void_t<typename T::char_type, typename T::traits_type,
			typename std::enable_if_t<std::is_same_v<T,
			std::basic_ifstream<typename T::char_type, typename T::traits_type>>>>> : std::true_type {};

		template<class T>
		inline constexpr bool is_ifstream_v = is_ifstream<std::remove_cv_t<std::remove_reference_t<T>>>::value;

		template<typename, typename = void>
		struct is_ofstream : std::false_type {};

		template<typename T>
		struct is_ofstream<T, std::void_t<typename T::char_type, typename T::traits_type,
			typename std::enable_if_t<std::is_same_v<T,
			std::basic_ofstream<typename T::char_type, typename T::traits_type>>>>> : std::true_type {};

		template<class T>
		inline constexpr bool is_ofstream_v = is_ofstream<std::remove_cv_t<std::remove_reference_t<T>>>::value;

		template<class T>
		inline constexpr bool is_file_stream_v = is_fstream_v<T> || is_ifstream_v<T> || is_ofstream_v<T>;


		template<typename, typename = void>
		struct is_string_view : std::false_type {};

		template<typename T>
		struct is_string_view<T, std::void_t<typename T::value_type, typename T::traits_type,
			typename std::enable_if_t<std::is_same_v<T,
			std::basic_string_view<typename T::value_type, typename T::traits_type>>>>> : std::true_type {};

		template<class T>
		inline constexpr bool is_string_view_v = is_string_view<T>::value;


		template<typename, typename = void>
		struct is_char_pointer : std::false_type {};

		// char const * 
		// std::remove_cv_t<std::remove_pointer_t<std::remove_cv_t<std::remove_reference_t<T>>>
		// char
		template<typename T>
		struct is_char_pointer<T, std::void_t<typename std::enable_if_t <
			 std::is_pointer_v<                      std::remove_cv_t<std::remove_reference_t<T>>>  &&
			!std::is_pointer_v<std::remove_pointer_t<std::remove_cv_t<std::remove_reference_t<T>>>> &&
			(
				std::is_same_v<std::remove_cv_t<std::remove_pointer_t<std::remove_cv_t<std::remove_reference_t<T>>>>, char    > ||
				std::is_same_v<std::remove_cv_t<std::remove_pointer_t<std::remove_cv_t<std::remove_reference_t<T>>>>, wchar_t > ||
			#if defined(__cpp_lib_char8_t)
				std::is_same_v<std::remove_cv_t<std::remove_pointer_t<std::remove_cv_t<std::remove_reference_t<T>>>>, char8_t > ||
			#endif
				std::is_same_v<std::remove_cv_t<std::remove_pointer_t<std::remove_cv_t<std::remove_reference_t<T>>>>, char16_t> ||
				std::is_same_v<std::remove_cv_t<std::remove_pointer_t<std::remove_cv_t<std::remove_reference_t<T>>>>, char32_t>
			)
			>>> : std::true_type {};

		template<class T>
		inline constexpr bool is_char_pointer_v = is_char_pointer<T>::value;


		template<typename, typename = void>
		struct is_char_array : std::false_type {};

		template<typename T>
		struct is_char_array<T, std::void_t<typename std::enable_if_t <
			std::is_array_v<std::remove_cv_t<std::remove_reference_t<T>>>  &&
			(
				std::is_same_v<std::remove_cv_t<std::remove_all_extents_t<std::remove_cv_t<std::remove_reference_t<T>>>>, char    > ||
				std::is_same_v<std::remove_cv_t<std::remove_all_extents_t<std::remove_cv_t<std::remove_reference_t<T>>>>, wchar_t > ||
			#if defined(__cpp_lib_char8_t)
				std::is_same_v<std::remove_cv_t<std::remove_all_extents_t<std::remove_cv_t<std::remove_reference_t<T>>>>, char8_t > ||
			#endif
				std::is_same_v<std::remove_cv_t<std::remove_all_extents_t<std::remove_cv_t<std::remove_reference_t<T>>>>, char16_t> ||
				std::is_same_v<std::remove_cv_t<std::remove_all_extents_t<std::remove_cv_t<std::remove_reference_t<T>>>>, char32_t>
			)
			>>> : std::true_type {};

		template<class T>
		inline constexpr bool is_char_array_v = is_char_array<T>::value;


		template<class R>
		struct return_type
		{
			template<class T, bool> struct string_view_traits { using type = T; };

			template<class T> struct string_view_traits<T, true>
			{
				using type = std::basic_string<typename std::remove_cv_t<std::remove_reference_t<R>>::value_type>;
			};

			using type = typename std::conditional_t<is_char_pointer_v<R> || is_char_array_v<R>,
				std::basic_string<std::remove_cv_t<std::remove_all_extents_t<
				std::remove_pointer_t<std::remove_cv_t<std::remove_reference_t<R>>>>>>,
				typename string_view_traits<R, is_string_view_v<R>>::type>;
		};
	}

	namespace detail
	{
		template<class Stream>
		class basic_file_ini_impl : public Stream
		{
		public:
			template<class ...Args>
			basic_file_ini_impl(Args&&... args)
			{
				std::ios_base::openmode mode{};

				if constexpr /**/ (sizeof...(Args) == 0)
				{
				#if defined(unix) || defined(__unix) || defined(_XOPEN_SOURCE) || defined(_POSIX_SOURCE) || \
					defined(linux) || defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
					filepath_.resize(PATH_MAX);
					auto r = readlink("/proc/self/exe", (char *)filepath_.data(), PATH_MAX);
					std::ignore = r; // gcc 7 warning: ignoring return value of ... [-Wunused-result]
				#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || \
					defined(_WINDOWS_) || defined(__WINDOWS__) || defined(__TOS_WIN__)
					filepath_.resize(MAX_PATH);
					filepath_.resize(::GetModuleFileNameA(NULL, (LPSTR)filepath_.data(), MAX_PATH));
				#elif defined(__APPLE__) && defined(__MACH__)
					filepath_.resize(PATH_MAX);
					std::uint32_t bufsize = std::uint32_t(PATH_MAX);
					_NSGetExecutablePath(filepath_.data(), std::addressof(bufsize));
				#endif
					
					if (std::string::size_type pos = filepath_.find('\0'); pos != std::string::npos)
						filepath_.resize(pos);

				#if defined(_DEBUG) || defined(DEBUG)
					assert(!filepath_.empty());
				#endif

					std::filesystem::path path{ filepath_ };

					std::string name = path.filename().string();

					std::string ext = path.extension().string();

					name.resize(name.size() - ext.size());

					filepath_ = path.parent_path().append(name).string() + ".ini";
				}
				else if constexpr (sizeof...(Args) == 1)
				{
					filepath_ = std::move(std::get<0>(std::make_tuple(std::forward<Args>(args)...)));
				}
				else if constexpr (sizeof...(Args) == 2)
				{
					auto t = std::make_tuple(std::forward<Args>(args)...);

					filepath_ = std::move(std::get<0>(t));
					mode |= std::get<1>(t);
				}
				else
				{
					std::ignore = true;
				}

				std::error_code ec;

				// if file is not exists, create it
				if (bool b = std::filesystem::exists(filepath_, ec); !b && !ec)
				{
					Stream f(filepath_, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
				}

				if constexpr /**/ (detail::util::is_fstream_v<Stream>)
				{
					mode |= std::ios_base::in | std::ios_base::out | std::ios_base::binary;
				}
				else if constexpr (detail::util::is_ifstream_v<Stream>)
				{
					mode |= std::ios_base::in | std::ios_base::binary;
				}
				else if constexpr (detail::util::is_ofstream_v<Stream>)
				{
					mode |= std::ios_base::out | std::ios_base::binary;
				}
				else
				{
					mode |= std::ios_base::in | std::ios_base::out | std::ios_base::binary;
				}

				Stream::open(filepath_, mode);
			}

			~basic_file_ini_impl()
			{
				using pos_type = typename Stream::pos_type;

				Stream::clear();
				Stream::seekg(0, std::ios::end);
				auto filesize = Stream::tellg();

				if (filesize)
				{
					pos_type spaces = pos_type(0);

					do
					{
						Stream::clear();
						Stream::seekg(filesize - spaces - pos_type(1));

						char c;
						if (!Stream::get(c))
							break;

						if (c == ' ' || c == '\0')
							spaces = spaces + pos_type(1);
						else
							break;

					} while (true);

					if (spaces)
					{
						std::error_code ec;
						std::filesystem::resize_file(filepath_, filesize - spaces, ec);
					}
				}
			}

			inline std::string filepath() { return filepath_; }

		protected:
			std::string                  filepath_;
		};

		template<class Stream>
		class basic_ini_impl : public Stream
		{
		public:
			template<class ...Args>
			basic_ini_impl(Args&&... args) : Stream(std::forward<Args>(args)...) {}
		};

		template<class... Ts>
		class basic_ini_impl<std::basic_fstream<Ts...>> : public basic_file_ini_impl<std::basic_fstream<Ts...>>
		{
		public:
			template<class ...Args>
			basic_ini_impl(Args&&... args) : basic_file_ini_impl<std::basic_fstream<Ts...>>(std::forward<Args>(args)...) {}
		};

		template<class... Ts>
		class basic_ini_impl<std::basic_ifstream<Ts...>> : public basic_file_ini_impl<std::basic_ifstream<Ts...>>
		{
		public:
			template<class ...Args>
			basic_ini_impl(Args&&... args) : basic_file_ini_impl<std::basic_ifstream<Ts...>>(std::forward<Args>(args)...) {}
		};

		template<class... Ts>
		class basic_ini_impl<std::basic_ofstream<Ts...>> : public basic_file_ini_impl<std::basic_ofstream<Ts...>>
		{
		public:
			template<class ...Args>
			basic_ini_impl(Args&&... args) : basic_file_ini_impl<std::basic_ofstream<Ts...>>(std::forward<Args>(args)...) {}
		};
	}

	/**
	 * basic_ini operator class
	 */
	template<class Stream>
	class basic_ini : public detail::basic_ini_impl<Stream>
	{
	public:
		using char_type = typename Stream::char_type;
		using pos_type  = typename Stream::pos_type;
		using size_type = typename std::basic_string<char_type>::size_type;

		template<class ...Args>
		basic_ini(Args&&... args) : detail::basic_ini_impl<Stream>(std::forward<Args>(args)...)
		{
			this->endl_ = { '\n' };
		#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || \
			defined(_WINDOWS_) || defined(__WINDOWS__) || defined(__TOS_WIN__)
			this->endl_ = { '\r','\n' };
		#elif defined(__APPLE__) && defined(__MACH__)
			// on the macos 9, the newline character is '\r'.
			// the last macos 9 version is 9.2.2 (20011205)
			//this->endl_ = { '\r' };
		#endif
		}

	protected:
		template<class Traits = std::char_traits<char_type>, class Allocator = std::allocator<char_type>>
		bool _get(
			std::basic_string_view<char_type, Traits> sec,
			std::basic_string_view<char_type, Traits> key,
			std::basic_string<char_type, Traits, Allocator> & val)
		{
			asio2_shared_lock guard(this->mutex_);

			Stream::clear();
			if (Stream::operator bool())
			{
				std::basic_string<char_type, Traits, Allocator> line;
				std::basic_string<char_type, Traits, Allocator> s, k, v;
				pos_type posg;

				Stream::seekg(0, std::ios::beg);

				char ret;
				while ((ret = this->_getline(line, s, k, v, posg)) != 'n')
				{
					switch (ret)
					{
					case 'a':break;
					case 's':
						if (s == sec)
						{
							do
							{
								ret = this->_getline(line, s, k, v, posg);
								if (ret == 'k' && k == key)
								{
									val = std::move(v);
									return true;
								}
							} while (ret == 'k' || ret == 'a' || ret == 'o');

							return false;
						}
						break;
					case 'k':
						if (s == sec)
						{
							if (k == key)
							{
								val = std::move(v);
								return true;
							}
						}
						break;
					case 'o':break;
					default:break;
					}
				}
			}
			return false;
		}

	public:
		/**
		 * get the value associated with a key in the specified section of an ini file.
		 * This function does not throw an exception.
		 * example : 
		 * asio2::ini ini("config.ini");
		 * std::string   host = ini.get("main", "host", "127.0.0.1");
		 * std::uint16_t port = ini.get("main", "port", 8080);
		 * or : 
		 * std::string   host = ini.get<std::string  >("main", "host");
		 * std::uint16_t port = ini.get<std::uint16_t>("main", "port");
		 */
		template<class R, class Sec, class Key, class Traits = std::char_traits<char_type>,
			class Allocator = std::allocator<char_type>>
		inline typename detail::util::return_type<R>::type
			get(const Sec& sec, const Key& key, R default_val = R())
		{
			using return_t = typename detail::util::return_type<R>::type;

			try
			{
				std::basic_string<char_type, Traits, Allocator> val;

				bool flag = this->_get(
					std::basic_string_view<char_type, Traits>(sec),
					std::basic_string_view<char_type, Traits>(key),
					val);

				if constexpr (detail::util::is_char_pointer_v<R> || detail::util::is_char_array_v<R>)
				{
					return (flag ? val : return_t{ default_val });
				}
				else if constexpr (detail::util::is_string_view_v<R>)
				{
					return (flag ? val : return_t{ default_val });
				}
				else
				{
					return (flag ? asio2::convert<R>::stov(val) : default_val);
				}
			}
			catch (std::invalid_argument const&)
			{
			}
			catch (std::out_of_range const&)
			{
			}
			catch (std::exception const&)
			{
			}

			return return_t{ default_val };
		}

		/**
		 * set the value associated with a key in the specified section of an ini file.
		 * example :
		 * asio2::ini ini("config.ini");
		 * ini.set("main", "host", "127.0.0.1");
		 * ini.set("main", "port", 8080);
		 */
		template<class Sec, class Key, class Val, class Traits = std::char_traits<char_type>>
		inline typename std::enable_if_t<std::is_same_v<decltype(
			std::basic_string_view<char_type, Traits>(std::declval<Sec>()),
			std::basic_string_view<char_type, Traits>(std::declval<Key>()),
			std::basic_string_view<char_type, Traits>(std::declval<Val>()),
			std::true_type()), std::true_type>, bool>
			set(const Sec& sec, const Key& key, const Val& val)
		{
			return this->set(
				std::basic_string_view<char_type, Traits>(sec),
				std::basic_string_view<char_type, Traits>(key),
				std::basic_string_view<char_type, Traits>(val));
		}

		/**
		 * set the value associated with a key in the specified section of an ini file.
		 * example :
		 * asio2::ini ini("config.ini");
		 * ini.set("main", "host", "127.0.0.1");
		 * ini.set("main", "port", 8080);
		 */
		template<class Sec, class Key, class Val, class Traits = std::char_traits<char_type>>
		inline typename std::enable_if_t<std::is_same_v<decltype(
			std::basic_string_view<char_type, Traits>(std::declval<Sec>()),
			std::basic_string_view<char_type, Traits>(std::declval<Key>()),
			std::to_string(std::declval<Val>()),
			std::true_type()), std::true_type>, bool>
			set(const Sec& sec, const Key& key, Val val)
		{
			std::basic_string<char_type, Traits> v = std::to_string(val);
			return this->set(
				std::basic_string_view<char_type, Traits>(sec),
				std::basic_string_view<char_type, Traits>(key),
				std::basic_string_view<char_type, Traits>(v));
		}

		/**
		 * set the value associated with a key in the specified section of an ini file.
		 * example :
		 * asio2::ini ini("config.ini");
		 * ini.set("main", "host", "127.0.0.1");
		 * ini.set("main", "port", 8080);
		 */
		template<class Traits = std::char_traits<char_type>, class Allocator = std::allocator<char_type>>
		bool set(
			std::basic_string_view<char_type, Traits> sec,
			std::basic_string_view<char_type, Traits> key,
			std::basic_string_view<char_type, Traits> val)
		{
			asio2_unique_lock guard(this->mutex_);

			Stream::clear();
			if (Stream::operator bool())
			{
				std::basic_string<char_type, Traits, Allocator> line;
				std::basic_string<char_type, Traits, Allocator> s, k, v;
				pos_type posg = 0;
				char ret;

				auto update_v = [&]() mutable -> bool
				{
					try
					{
						if (val != v)
						{
							Stream::clear();
							Stream::seekg(0, std::ios::end);
							auto filesize = Stream::tellg();

							std::basic_string<char_type, Traits, Allocator> content;
							auto pos = line.find_first_of('=');
							++pos;
							while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t'))
								++pos;
							content += line.substr(0, pos);
							content += val;
							content += this->endl_;

							int pos_diff = int(line.size() + 1 - content.size());

							Stream::clear();
							Stream::seekg(posg + pos_type(line.size() + 1));

							int remain = int(filesize - (posg + pos_type(line.size() + 1)));
							if (remain > 0)
							{
								content.resize(size_type(content.size() + remain));
								Stream::read(content.data() + content.size() - remain, remain);
							}

							if (pos_diff > 0) content.append(pos_diff, ' ');

							while (!content.empty() &&
								(pos_type(content.size()) + posg > filesize) &&
								content.back() == ' ')
							{
								content.erase(content.size() - 1);
							}

							Stream::clear();
							Stream::seekp(posg);
							//*this << content;
							Stream::write(content.data(), content.size());
							Stream::flush();
						}
						return true;
					}
					catch (std::exception &) {}
					return false;
				};

				Stream::seekg(0, std::ios::beg);

				while ((ret = this->_getline(line, s, k, v, posg)) != 'n')
				{
					switch (ret)
					{
					case 'a':break;
					case 's':
						if (s == sec)
						{
							do
							{
								ret = this->_getline(line, s, k, v, posg);
								if (ret == 'k' && k == key)
								{
									return update_v();
								}
							} while (ret == 'k' || ret == 'a' || ret == 'o');

							// can't find the key, add a new key
							std::basic_string<char_type, Traits, Allocator> content;

							if (posg == pos_type(-1))
							{
								Stream::clear();
								Stream::seekg(0, std::ios::end);
								posg = Stream::tellg();
							}

							content += this->endl_;
							content += key;
							content += '=';
							content += val;
							content += this->endl_;

							Stream::clear();
							Stream::seekg(0, std::ios::end);
							auto filesize = Stream::tellg();

							Stream::clear();
							Stream::seekg(posg);

							int remain = int(filesize - posg);
							if (remain > 0)
							{
								content.resize(size_type(content.size() + remain));
								Stream::read(content.data() + content.size() - remain, remain);
							}

							while (!content.empty() &&
								(pos_type(content.size()) + posg > filesize) &&
								content.back() == ' ')
							{
								content.erase(content.size() - 1);
							}

							Stream::clear();
							Stream::seekp(posg);
							//*this << content;
							Stream::write(content.data(), content.size());
							Stream::flush();

							return true;
						}
						break;
					case 'k':
						if (s == sec)
						{
							if (k == key)
							{
								return update_v();
							}
						}
						break;
					case 'o':break;
					default:break;
					}
				}

				// can't find the sec and key, add a new sec and key.
				std::basic_string<char_type, Traits, Allocator> content;

				Stream::clear();
				Stream::seekg(0, std::ios::end);
				auto filesize = Stream::tellg();
				content.resize(size_type(filesize));

				Stream::clear();
				Stream::seekg(0, std::ios::beg);
				Stream::read(content.data(), content.size());

				if (!content.empty() && content.back() == '\n') content.erase(content.size() - 1);
				if (!content.empty() && content.back() == '\r') content.erase(content.size() - 1);

				std::basic_string<char_type, Traits, Allocator> buffer;

				if (!sec.empty())
				{
					buffer += '[';
					buffer += sec;
					buffer += ']';
					buffer += this->endl_;
				}

				buffer += key;
				buffer += '=';
				buffer += val;
				buffer += this->endl_;

				if (!sec.empty())
				{
					if (content.empty())
						content = std::move(buffer);
					else
						content = content + this->endl_ + buffer;
				}
				// if the sec is empty ( mean global), must add the key at the begin.
				else
				{
					if (content.empty())
						content = std::move(buffer);
					else
						content = buffer + content;
				}

				while (!content.empty() &&
					(pos_type(content.size()) > filesize) &&
					content.back() == ' ')
				{
					content.erase(content.size() - 1);
				}

				Stream::clear();
				Stream::seekp(0, std::ios::beg);
				//*this << content;
				Stream::write(content.data(), content.size());
				Stream::flush();

				return true;
			}
			return false;
		}

	protected:
		template<class Traits = std::char_traits<char_type>, class Allocator = std::allocator<char_type>>
		char _getline(
			std::basic_string<char_type, Traits, Allocator> & line,
			std::basic_string<char_type, Traits, Allocator> & sec,
			std::basic_string<char_type, Traits, Allocator> & key,
			std::basic_string<char_type, Traits, Allocator> & val,
			pos_type & posg)
		{
			Stream::clear();
			if (Stream::good() && !Stream::eof())
			{
				posg = Stream::tellg();

				if (posg != pos_type(-1) && std::getline(*this, line, this->endl_.back()))
				{
					auto trim_line = line;

					trim_left(trim_line);
					trim_right(trim_line);

					if (trim_line.empty())
					{
						return 'o'; // other
					}

					// current line is code annotation
					if (
						(trim_line.size() > 0 && (trim_line[0] == ';' || trim_line[0] == '#' || trim_line[0] == ':'))
						||
						(trim_line.size() > 1 && trim_line[0] == '/' && trim_line[1] == '/')
						)
					{
						return 'a'; // annotation
					}

					auto pos1 = trim_line.find_first_of('[');
					auto pos2 = trim_line.find_first_of(']');

					// current line is section
					if (
						pos1 == 0
						&&
						pos2 != std::basic_string<char_type, Traits, Allocator>::npos
						&&
						pos2 > pos1
						)
					{
						sec = trim_line.substr(pos1 + 1, pos2 - pos1 - 1);

						trim_left(sec);
						trim_right(sec);

						return 's'; // section
					}

					auto sep = trim_line.find_first_of('=');

					// current line is key and val
					if (sep != std::basic_string<char_type, Traits, Allocator>::npos && sep > 0)
					{
						key = trim_line.substr(0, sep);
						trim_left(key);
						trim_right(key);

						val = trim_line.substr(sep + 1);
						trim_left(val);
						trim_right(val);

						return 'k'; // kv
					}

					return 'o'; // other
				}
			}

			return 'n'; // null
		}

		template<
			class CharT,
			class Traits = std::char_traits<CharT>,
			class Allocator = std::allocator<CharT>
		>
		std::basic_string<CharT, Traits, Allocator>& trim_left(std::basic_string<CharT, Traits, Allocator>& s)
		{
			if (s.empty())
				return s;
			using size_type = typename std::basic_string<CharT, Traits, Allocator>::size_type;
			size_type pos = 0;
			for (; pos < s.size(); ++pos)
			{
				if (!std::isspace(static_cast<unsigned char>(s[pos])))
					break;
			}
			s.erase(0, pos);
			return s;
		}

		template<
			class CharT,
			class Traits = std::char_traits<CharT>,
			class Allocator = std::allocator<CharT>
		>
		std::basic_string<CharT, Traits, Allocator>& trim_right(std::basic_string<CharT, Traits, Allocator>& s)
		{
			if (s.empty())
				return s;
			using size_type = typename std::basic_string<CharT, Traits, Allocator>::size_type;
			size_type pos = s.size() - 1;
			for (; pos != size_type(-1); pos--)
			{
				if (!std::isspace(static_cast<unsigned char>(s[pos])))
					break;
			}
			s.erase(pos + 1);
			return s;
		}

	protected:
		mutable asio2_shared_mutex   mutex_;

		std::basic_string<char_type> endl_;
	};

	using ini     = basic_ini<std::fstream>;
}

#if defined(__GNUC__) || defined(__GNUG__)
#  pragma GCC diagnostic pop
#endif

#endif // !__ASIO2_INI_HPP__
