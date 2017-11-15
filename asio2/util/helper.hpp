/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 *
 */

#ifndef __ASIO2_HELPER_HPP__
#define __ASIO2_HELPER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cctype>
#include <cstdlib>
#include <cstdarg>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <clocale>
#include <climits>

#include <string>
#include <locale>
#include <limits>
#include <algorithm>
#include <future>


#if defined(__unix__) || defined(__linux__)
#	include <unistd.h>
#	include <sys/types.h>
#	include <sys/stat.h>
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	include <Windows.h>
#	include <io.h>
#	include <direct.h>
#endif

#ifdef _MSC_VER
#	pragma warning(push)
#	pragma warning(disable:4996)
#endif


#if defined(__GNUC__) || defined(__GNUG__)
#	define __UNUSED__ __attribute__((unused))
#else
#	define __UNUSED__ 
#endif

namespace asio2
{
	// use anonymous namespace to resolve global function redefinition problem
	namespace
	{

		/**
		 * get current directory of running application,note : the directory string will be end with '\' or '/'
		 */
		__UNUSED__ std::string get_current_directory()
		{
#if defined(__unix__) || defined(__linux__)
			std::string dir(PATH_MAX, '\0');
			readlink("/proc/self/exe", (char *)(&dir[0]), PATH_MAX);
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
			std::string dir(MAX_PATH, '\0');
			::GetModuleFileNameA(NULL, (LPSTR)(&dir[0]), MAX_PATH);
#endif
			auto pos = dir.find_last_of('\\', dir.length());
			if (pos == std::string::npos)
				pos = dir.find_last_of('/', dir.length());
			dir.resize(pos + 1);

			return std::move(dir);
		}

		/**
		 * std::string format
		 */
		__UNUSED__ std::string format(const char * format, va_list args)
		{
			std::string s;

			// under windows and linux system,std::vsnprintf(nullptr, 0, format, args) can get the need buffer len for the output,
			va_list args_copy;

			va_copy(args_copy, args);
			int len = std::vsnprintf(nullptr, 0, format, args_copy);

			if (len > 0)
			{
				s.resize(len);

				va_copy(args_copy, args);
				std::vsprintf((char*)(&s[0]), format, args_copy);
			}

			return std::move(s);
		}

		/**
		 * std::wstring format
		 */
		__UNUSED__ std::wstring format(const wchar_t * format, va_list args)
		{
			std::wstring s;

			va_list args_copy;

			while (true)
			{
				s.resize(s.capacity());

				va_copy(args_copy, args);

				// if provided buffer size is less than required size,vswprintf will return -1
				// so if len equal -1,we increase the buffer size again, and has to use a loop 
				// to get the correct output buffer len,
				int len = std::vswprintf((wchar_t*)(&s[0]), s.size(), format, args_copy);
				if (len == -1)
					s.reserve(s.capacity() * 2);
				else
				{
					s.resize(len);
					break;
				}
			}

			return std::move(s);
		}

		/**
		 * std::string format
		 */
		__UNUSED__ std::string format(const char * format, ...)
		{
			std::string s;

			// under windows and linux system,std::vsnprintf(nullptr, 0, format, args) can get the need buffer len for the output,
			va_list args, args_copy;
			va_start(args, format);

			va_copy(args_copy, args);
			int len = std::vsnprintf(nullptr, 0, format, args_copy);
			if (len > 0)
			{
				s.resize(len);

				va_copy(args_copy, args);
				std::vsprintf((char*)(&s[0]), format, args_copy);
			}

			va_end(args);

			return std::move(s);
		}

		/**
		 * std::wstring format
		 */
		__UNUSED__ std::wstring format(const wchar_t * format, ...)
		{
			std::wstring s;

			va_list args, args_copy;
			va_start(args, format);

			while (true)
			{
				s.resize(s.capacity());

				va_copy(args_copy, args);

				// if provided buffer size is less than required size,vswprintf will return -1
				// so if len equal -1,we increase the buffer size again, and has to use a loop 
				// to get the correct output buffer len,
				int len = std::vswprintf((wchar_t*)(&s[0]), s.size(), format, args_copy);
				if (len == -1)
					s.reserve(s.capacity() * 2);
				else
				{
					s.resize(len);
					break;
				}
			}
			va_end(args);

			return std::move(s);
		}

		/**
		 * @function : trim the space character : space \t \r \n and so on
		 */
		__UNUSED__ std::string & trim(std::string & s)
		{
			for (std::size_t i = s.length() - 1; i != (size_t)-1; i--)
			{
				if (std::isspace(static_cast<unsigned char>(s[i])))
				{
					s.erase(i, 1);
				}
			}
			return s;
		}

		/**
		 * @function : get a number which is the nth power of 2 ,and greater than v.
		 * eg : when v = 3 then return 4,when v = 410 then return 512
		 */
		template<typename T>
		__UNUSED__ T get_power_number(T v)
		{
			uint8_t bits = 0;
			T t = v;

			for (; t > 0; bits++)
			{
				t = t >> 1;
			}

			t = ((v << 1) & ((-1) << bits));

			if (t <= 0)
				t = 8;

			return t;
		}

		/**
		 * @function : recursive create directory
		 */
		__UNUSED__ bool create_directory(std::string path)
		{
			if (path.length() > 0 && (path[path.length() - 1] == '/' || path[path.length() - 1] == '\\'))
				path.erase(path.length() - 1, 1);

			if (path.empty())
				return false;

			if (0 == access(path.data(), 0))
				return true;

			std::size_t sep = path.find_last_of("/\\");
			if (std::string::npos != sep && sep > 0)
				create_directory(path.substr(0, sep).data());
#if defined(__unix__) || defined(__linux__)
			return (0 == mkdir(path.data(), S_IRWXU | S_IRWXG | S_IROTH));
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
			return (0 == mkdir(path.data()));
#endif
		}


	}


	template<class T>
	class auto_promise
	{
	public:
		auto_promise(std::promise<T> & p, T v) : _p(p), _v(v) {}
		~auto_promise() { _p.set_value(_v); }
	protected:
		std::promise<T> & _p;
		T _v;
	};

	template<>
	class auto_promise<void>
	{
	public:
		auto_promise(std::promise<void> & p) : _p(p) {}
		~auto_promise() { _p.set_value(); }
	protected:
		std::promise<void> & _p;
	};


}

#ifdef _MSC_VER
#	pragma warning(pop)
#endif

#endif // !__ASIO2_HELPER_HPP__
