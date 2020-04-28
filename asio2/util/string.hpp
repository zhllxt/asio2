/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_STRING_HPP__
#define __ASIO2_STRING_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>
#include <cstdarg>
#include <cstdio>
#include <cwchar>
#include <climits>
#include <cctype>
#include <cstring>

#include <string>
#include <string_view>
#include <vector>
#include <type_traits>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4996)
#endif

namespace asio2
{
	/**
	 * std::string format
	 */
	template<typename = void>
	std::string formatv(const char * format, va_list args)
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

		return s;
	}

	/**
	 * std::wstring format
	 */
	template<typename = void>
	std::wstring formatv(const wchar_t * format, va_list args)
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

		return s;
	}

	/**
	 * std::string format
	 */
	template<typename = void>
	std::string format(const char * format, ...)
	{
		std::string s;

		if (format && *format)
		{
			// under windows and linux system,std::vsnprintf(nullptr, 0, format, args) can get the need buffer len for the output,
			va_list args;
			va_start(args, format);

			s = formatv(format, args);

			va_end(args);
		}

		return s;
	}

	/**
	 * std::wstring format
	 */
	template<typename = void>
	std::wstring format(const wchar_t * format, ...)
	{
		std::wstring s;

		if (format && *format)
		{
			va_list args;
			va_start(args, format);

			s = formatv(format, args);

			va_end(args);
		}

		return s;
	}

	/**
	 * @function : trim each space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
		std::basic_string<CharT, Traits, Allocator>& trim_all(std::basic_string<CharT, Traits, Allocator>& s)
	{
		using size_type = typename std::basic_string<CharT, Traits, Allocator>::size_type;
		for (size_type i = s.size() - 1; i != size_type(-1); i--)
		{
			if (std::isspace(static_cast<unsigned char>(s[i])))
			{
				s.erase(i, 1);
			}
		}
		return s;
	}

	/**
	 * @function : trim left space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
		std::basic_string<CharT, Traits, Allocator>& trim_left(std::basic_string<CharT, Traits, Allocator>& s)
	{
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

	/**
	 * @function : trim right space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
		std::basic_string<CharT, Traits, Allocator>& trim_right(std::basic_string<CharT, Traits, Allocator>& s)
	{
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

	/**
	 * @function : trim left and right space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
		std::basic_string<CharT, Traits, Allocator>& trim_both(std::basic_string<CharT, Traits, Allocator>& s)
	{
		trim_left(s);
		trim_right(s);
		return s;
	}

	/**
	 * @function : Splits the string into multiple strings with the specified delimiters
	 */
	template<class String, class Delimiter>
	inline std::vector<String> split(const String& s, const Delimiter& delimiters = " ")
	{
		using size_type = typename String::size_type;
		std::vector<String> tokens;
		size_type last_pos = s.find_first_not_of(delimiters, 0);
		size_type pos = s.find_first_of(delimiters, last_pos);
		while (String::npos != pos || String::npos != last_pos)
		{
			tokens.emplace_back(s.substr(last_pos, pos - last_pos));
			last_pos = s.find_first_not_of(delimiters, pos);
			pos = s.find_first_of(delimiters, last_pos);
		}
		return tokens;
	}
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // !__ASIO2_STRING_HPP__
