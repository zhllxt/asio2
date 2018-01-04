/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 *
 *          : windows default language environment of china is gbk .
 *            linux   default language environment of china is utf-8 .
 * 
 *            if source file format is "utf8 with bom" and include chinese character in source file : 
 *                compile -> 
 *                           windows : visual studio ,correct
 *                           linux   : g++ ,correct
 *                printf  ->
 *                           windows : visual studio ,correct
 *                           linux   : g++ ,rest with std::setlocale(LC_ALL, "...");
 * 
 *            if source file format is "utf8 no bom" and include chinese character in source file :
 *                compile ->
 *                           windows : visual studio ,warning,unidentified character of chinese character
 *                           linux   : g++ ,correct
 *                printf  ->
 *                           windows : visual studio ,correct
 *                           linux   : g++ ,rest with std::setlocale(LC_ALL, "...");
 * 
 *            if this file saved use "GB2312 .936" format,when compiler on visual studio(windows),it's correct,but linux
 *            file format is always "utf8",so when compiler on g++(linux),it's can't read this file correct in some situation.
 * 
 *            but if use visual studio 2017 to compiler linux application on windows system,it's can use "GB2312 .936"
 *            format,and still can compiler linux application correct.
 * 
 *            when on linux system,if source file format is "GB2312 .936", and you has't call std::setlocale(LC_ALL, "zh_CN.gbk");
 *            at this time the linux system default language environment is "utf-8",so when you call wcstombs to get the target
 *            buffer size,it will return a invalid too large value,may be cause crash.
 */

#ifndef __ASIO2_HELPER_HPP__
#define __ASIO2_HELPER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if !defined(NDEBUG) && !defined(DEBUG) && !defined(_DEBUG)
#	define NDEBUG
#endif

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

#include <asio2/util/def.hpp>

#if defined(LINUX)
#	include <unistd.h>
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <iconv.h> // gcc 4.8.1 and earlier version has't codecvt,we has to use iconv,wtf
#	include <dirent.h>
#elif defined(WINDOWS)
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	include <Windows.h>
#	include <tchar.h>
#	include <io.h>
#	include <direct.h>
#	include <codecvt>
#endif

#ifdef _MSC_VER
#	pragma warning(push)
#	pragma warning(disable:4996)
#endif

namespace asio2
{
	// use anonymous namespace to resolve global function redefinition problem
	namespace
	{

		/**
		 * BKDR Hash Function
		 */
		template<class T>
		inline std::size_t string_hash(const T * s, std::size_t size = -1)
		{
			std::size_t hash = 0;
			if (size == -1)
			{
				while (*s)
				{
					hash = hash * 131 + static_cast<std::size_t>(*s);
					s++;
				}
			}
			else
			{
				for (std::size_t i = 0; i < size; i++)
				{
					hash = hash * 131 + static_cast<std::size_t>(*s);
					s++;
				}
			}
			return hash;
		}

		/**
		 * get current directory of running application,note : the directory string will be end with '\' or '/'
		 */
		std::string get_current_directory()
		{
#if   defined(LINUX)
			std::string dir(PATH_MAX, '\0');
			readlink("/proc/self/exe", (char *)dir.data(), PATH_MAX);
#elif defined(WINDOWS)
			std::string dir(MAX_PATH, '\0');
			dir.resize(::GetModuleFileNameA(NULL, (LPSTR)dir.data(), MAX_PATH));
#endif
			size_t pos = dir.find_last_of(SLASH);
			if (pos != std::string::npos)
				dir.resize(pos + 1);

			return std::move(dir);
		}

		/**
		 * get current directory of running application,note : the directory string will be end with '\' or '/'
		 */
		bool get_current_directory(char * buf, std::size_t size)
		{
			if (!buf || size == 0)
				return false;
#if   defined(LINUX)
			ssize_t ret = readlink("/proc/self/exe", buf, size);
			if (ret > 0)
			{
				ret = ((std::size_t)ret < size ? ret : (ssize_t)(size - 1));
				buf[ret] = '\0';
				auto p = std::strrchr(buf, SLASH);
				if (p)
					*(p + 1) = '\0';
				return true;
			}
#elif defined(WINDOWS)
			if (::GetModuleFileNameA(NULL, buf, static_cast<DWORD>(size)) > 0)
			{
				auto p = std::strrchr(buf, SLASH);
				if (p)
					*(p + 1) = '\0';
				return true;
			}
#endif
			return false;
		}

		/**
		 * find in the src string,wether some one character of des is in the src
		 */
		char * strfind(char * src, const char * des)
		{
			if (src && des && *des)
			{
				std::size_t src_size = std::strlen(src);
				for (std::size_t i = 0; i < src_size; i++)
				{
					auto p = std::strchr(des, src[i]);
					if (p)
					{
						return (src + i);
					}
				}
			}
			return nullptr;
		}

		/**
		 * reverse find in the src string,wether some one character of des is in the src
		 */
		char * strrfind(char * src, const char * des)
		{
			if (src && des && *des)
			{
				int src_size = static_cast<int>(std::strlen(src));
				for (int i = src_size - 1; i >= 0; i--)
				{
					auto p = std::strchr(des, src[i]);
					if (p)
					{
						return (src + i);
					}
				}
			}
			return nullptr;
		}

		/**
		 * traverse the folder for specified type files,don't traverse the sub folder
		 */
#if   defined(LINUX)
		template<typename _handler>
		void traverse_file(const char * directory, const char * filetype, const _handler & handler)
		{
			if (!directory || !(*directory))
				return;

			DIR * dir = opendir(directory);
			dirent * p = nullptr;
			while ((p = readdir(dir)) != nullptr)
			{
				if (p->d_name[0] != '.')
				{
					std::string path(directory);
					path += p->d_name;

					if (!filetype || !(*filetype))
						handler(path.data());
					else if (path.substr(path.size() - 3) == (filetype + 2))
						handler(path.data());
				}
			}
			closedir(dir);
		}
#elif defined(WINDOWS)
		template<typename _handler>
		void traverse_file(const char * directory, const char * filetype, const _handler & handler)
		{
			if (!directory || !(*directory))
				return;

			std::string path(directory);
			if (path[path.size() - 1] != '\\' && path[path.size() - 1] != '/')
				path += SLASH;

			std::string search_file(path);

			if (!filetype || !(*filetype))
				search_file += "*.*";
			else
				search_file += filetype;

			_finddata_t find;
			intptr_t handle = _findfirst(search_file.data(), &find);
			if (handle != -1)
			{
				do
				{
					std::string filepath(path);
					filepath += find.name;

					handler(filepath.data());
				} while (_findnext(handle, &find) == 0);

				_findclose(handle);
			}
		}
#endif

		/**
		 * on different platform and use different compiler ,the c++ std library is not support perfect enough,but the "c" library
		 * support is better,so we should most use "c" library for cross platform.some times the std::cout and std::wcout can't
		 * show chinese character,but the printf and wprintf can show chinese character.
		 *
		 * in order to show character corrent,should call "std::setlocale(LC_ALL, "");" to init the default language environment
		 * when the application startup.
		 *
		 * don't use "std::setlocale(LC_ALL, "");" in the character convert function( for example ws2s and s2ws),beause when use
		 * those function in multi thread,will cause the language environment changed, eg : when application startup,the user
		 * set the language environment for "en_US.utf8",after use those character convert function in multi thread,the language
		 * environment may be changed to "zh_CN.utf8",and use it in mutli thread may be cause crash.
		 *
		 * should not use printf and wprintf together,it will cause incorrent,no defiened result.we can use printf to show wchar_t
		 * unicode character use format "ls",like this : printf("ls",L"chinese character");
		 */

		// 
		// the wcstombs and mbstowcs function result affected by system language coding.
		// you must insure that the buffer of "s" language format is equal to the setlocale(LC_ALL,"") format,otherwise it will crash
		// in linux system.
		// eg : if source file format is gb2312(then the buffer of "s" format is also gb2312),but in linux the locale is zh_CN.UTF-8,
		// then call wcstombs and mbstowcs will cause incorrect.so you need to call setlocale(LC_ALL,"chs") at the application startup.
		// 
		// when use wcstombs and mbstowcs in a dll,you also should set the locale by call setlocale(LC_ALL,"xxx") first,otherwise it
		// not worked correctly.

		/**
		 * convert unicode string to asni string
		 */
		std::string ws2s(const wchar_t * s)
		{
			if (!s || !(*s))
				return std::string();
			std::size_t des_len = std::wcstombs(nullptr, s, 0);
			std::string des_str(des_len, '\0');
			std::wcstombs((char*)des_str.data(), s, des_len);
			return des_str;
		}
		std::string ws2s(const std::wstring & s)
		{
			return ws2s(s.data());
		}

		/**
		 * convert asni string to unicode string
		 */
		std::wstring s2ws(const char * s)
		{
			if (!s || !(*s))
				return std::wstring();
			std::size_t src_len = std::strlen(s);
			std::size_t des_len = std::mbstowcs(nullptr, s, src_len);
			std::wstring des_str(des_len, '\0');
			std::mbstowcs((wchar_t*)des_str.data(), s, src_len);
			return des_str;
		}
		std::wstring s2ws(const std::string & s)
		{
			return s2ws(s.data());
		}

		/**
		 * std::string format
		 */
		std::string format(const char * format, va_list args)
		{
			std::string s;

			if (format && *format)
			{
				// under windows and linux system,std::vsnprintf(nullptr, 0, format, args) can get the need buffer len for the output,
				va_list args_copy;

				va_copy(args_copy, args);
				int len = std::vsnprintf(nullptr, 0, format, args_copy);

				if (len > 0)
				{
					s.resize(len);

					va_copy(args_copy, args);
					std::vsprintf((char*)s.data(), format, args_copy);
				}
			}

			return std::move(s);
		}

		/**
		 * std::wstring format
		 */
		std::wstring format(const wchar_t * format, va_list args)
		{
			std::wstring s;

			if (format && *format)
			{
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
			}

			return std::move(s);
		}

		/**
		 * std::string format
		 */
		std::string format(const char * format, ...)
		{
			std::string s;

			if (format && *format)
			{
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
			}

			return std::move(s);
		}

		/**
		 * std::wstring format
		 */
		std::wstring format(const wchar_t * format, ...)
		{
			std::wstring s;

			if (format && *format)
			{
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
			}

			return std::move(s);
		}

#if   defined(LINUX)
		std::string convert(const char * from_charset, const char * to_charset, char * inbuf)
		{
			if ((!from_charset || std::strlen(from_charset) == 0) ||
				(!to_charset || std::strlen(to_charset) == 0) ||
				(!inbuf || std::strlen(inbuf) == 0))
				return "";

			iconv_t cd = iconv_open(to_charset, from_charset);
			if ((std::size_t)cd == (std::size_t)(-1))
				return "";

			std::size_t inlen = std::strlen(inbuf);
			std::size_t outlen = inlen * 3;
			std::size_t outlen_old = outlen;
			std::string result(outlen, '\0');

			char * outbuf = (char *)result.data();

			std::size_t ret = iconv(
				cd,      // handle generated by iconv_open function
				&inbuf,  // input buf
				&inlen,  // input buf len,save the not converted character count
				&outbuf, // output buf
				&outlen  // output buf len,save the not used space(character count) of ouput buf 
			);
			while (((std::size_t)ret == (std::size_t)(-1)) && outlen < std::numeric_limits<unsigned short>::max())
			{
				outlen *= 2;
				outlen_old = outlen;
				result.resize(outlen, '\0');
				outbuf = (char *)result.data();

				ret = iconv(cd, &inbuf, &inlen, &outbuf, &outlen);
			}

			if ((std::size_t)ret == (std::size_t)(-1))
			{
				iconv_close(cd);
				return "";
			}

			result.resize(outlen_old - outlen);

			iconv_close(cd);

			return result;
		}
		std::string convert(const wchar_t * from_charset, const wchar_t * to_charset, wchar_t * inbuf)
		{
			return convert(ws2s(from_charset).data(), ws2s(to_charset).data(), (char *)ws2s(inbuf).data());
		}
#endif


		/**
		 * convert string to utf8 format
		 */
		std::string gbk_to_utf8(const char * s)
		{
			if (!s || !(*s))
				return std::string();
#if   defined(LINUX)
			return convert("gbk", "utf-8", (char *)s);
#elif defined(WINDOWS)
			return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(s2ws(s));
#endif
		}
		std::string gbk_to_utf8(const wchar_t * s)
		{
			if (!s || !(*s))
				return std::string();
#if   defined(LINUX)
			return convert("gbk", "utf-8", (char *)ws2s(s).data());
#elif defined(WINDOWS)
			// std::codecvt_utf8<class Elem,...>  Elem - either char16_t, char32_t, or wchar_t
			return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(s);
#endif
		}
		std::string gbk_to_utf8(const std::wstring & s)
		{
			return gbk_to_utf8(s.data());
		}
		std::string gbk_to_utf8(const std::string & s)
		{
			return gbk_to_utf8(s.data());
		}


		/**
		 * convert utf8 format to string
		 */
		std::string utf8_to_gbk(const char * s)
		{
			if (!s || !(*s))
				return std::string();
#if   defined(LINUX)
			return convert("utf-8", "gbk", (char *)s);
#elif defined(WINDOWS)
			return ws2s(std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(s));
#endif
		}
		std::string utf8_to_gbk(const wchar_t * s)
		{
			return utf8_to_gbk(ws2s(s).data());
		}
		std::string utf8_to_gbk(const std::string & s)
		{
			return utf8_to_gbk(s.data());
		}
		std::string utf8_to_gbk(const std::wstring & s)
		{
			return utf8_to_gbk(ws2s(s).data());
		}

		/**
		 * @function : trim the space character : space \t \r \n and so on
		 */
		std::string & trim(std::string & s)
		{
			for (std::size_t i = s.size() - 1; i != (size_t)-1; i--)
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
		T get_power_number(T v)
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
		bool create_directory(std::string path)
		{
			if (path.size() > 0 && (path[path.size() - 1] == '/' || path[path.size() - 1] == '\\'))
				path.erase(path.size() - 1, 1);

			if (path.empty())
				return true;

			if (0 == access(path.data(), 0)) // Each of these functions returns 0 if the file has the given mode.
				return true;

			std::size_t sep = path.find_last_of("\\/");
			if (std::string::npos != sep && sep > 0)
				create_directory(path.substr(0, sep));

#if   defined(LINUX)
			return (0 == mkdir(path.data(), S_IRWXU | S_IRWXG | S_IROTH));
#elif defined(WINDOWS)
			return (0 == mkdir(path.data()));
#endif
		}

	}

}

#ifdef _MSC_VER
#	pragma warning(pop)
#endif

#endif // !__ASIO2_HELPER_HPP__
