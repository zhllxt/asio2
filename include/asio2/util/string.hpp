/*
 * Copyright (c) 2017-2023 zhllxt
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

#include <asio2/base/detail/push_options.hpp>

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
#include <algorithm>

namespace asio2
{
	namespace detail
	{
		template<class T, bool Flag = std::is_trivial_v<T>>
		struct char_type;

		template<class T>
		struct char_type<T, true>
		{
			using type = T;
		};

		template<class T>
		struct char_type<T, false>
		{
			using type = typename T::value_type;
		};

		template<typename = void>
		inline char ascii_tolower(char c) noexcept
		{
			return char(((static_cast<unsigned>(c) - 65U) < 26) ? c + 'a' - 'A' : c);
		}
	}

	/**
	 * std::string format
	 */
	template<typename = void>
	std::string formatv(const char * format, va_list args)
	{
		std::string s;

		if (format && *format)
		{
			// under windows and linux system,std::vsnprintf(nullptr, 0, format, args)
			// can get the need buffer len for the output,
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
			// under windows and linux system,std::vsnprintf(nullptr, 0, format, args)
			// can get the need buffer len for the output,
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
	 * @brief trim each space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	std::basic_string<CharT, Traits, Allocator>& trim_all(std::basic_string<CharT, Traits, Allocator>& s)
	{
		if (s.empty())
			return s;
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
	 * @brief trim left space character of the string: space \t \r \n and so on
	 */
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

	/**
	 * @brief trim right space character of the string: space \t \r \n and so on
	 */
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

	/**
	 * @brief trim left and right space character of the string: space \t \r \n and so on
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
	 * @brief trim left space character of the string: space \t \r \n and so on
	 */
	template<class CharT, class Traits = std::char_traits<CharT>>
	std::basic_string_view<CharT, Traits>& trim_left(std::basic_string_view<CharT, Traits>& s)
	{
		if (s.empty())
			return s;
		using size_type = typename std::basic_string_view<CharT, Traits>::size_type;
		size_type pos = 0;
		for (; pos < s.size(); ++pos)
		{
			if (!std::isspace(static_cast<unsigned char>(s[pos])))
				break;
		}
		s.remove_prefix(pos);
		return s;
	}

	/**
	 * @brief trim right space character of the string: space \t \r \n and so on
	 */
	template<class CharT, class Traits = std::char_traits<CharT>>
	std::basic_string_view<CharT, Traits>& trim_right(std::basic_string_view<CharT, Traits>& s)
	{
		if (s.empty())
			return s;
		using size_type = typename std::basic_string_view<CharT, Traits>::size_type;
		size_type pos = s.size() - 1;
		for (; pos != size_type(-1); pos--)
		{
			if (!std::isspace(static_cast<unsigned char>(s[pos])))
				break;
		}
		s.remove_suffix(s.size() - pos - 1);
		return s;
	}

	/**
	 * @brief trim left and right space character of the string: space \t \r \n and so on
	 */
	template<class CharT, class Traits = std::char_traits<CharT>>
	std::basic_string_view<CharT, Traits>& trim_both(std::basic_string_view<CharT, Traits>& s)
	{
		trim_left(s);
		trim_right(s);
		return s;
	}

	/**
	 * @brief Splits the string into multiple strings with the specified delimiters
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

	/**
	 * @brief Replaces all old_str characters that appear in the string with new_str characters.
	 */
	template<class String, class OldStr, class NewStr>
	inline String& replace(String& s, const OldStr& old_str, const NewStr& new_str)
	{
		using size_type  = typename String::size_type;
		using value_type = typename String::value_type;
		using old_str_type = std::remove_reference_t<std::remove_cv_t<OldStr>>;
		using new_str_type = std::remove_reference_t<std::remove_cv_t<NewStr>>;
		using old_raw_type = std::remove_pointer_t<std::remove_all_extents_t<old_str_type>>;
		using new_raw_type = std::remove_pointer_t<std::remove_all_extents_t<new_str_type>>;

		size_type old_str_size = 0;
		size_type new_str_size = 0;

		// char* char[] char
		if constexpr (std::is_trivial_v<old_raw_type>)
		{
			// char* char[]
			if constexpr (std::is_pointer_v<old_str_type> || std::is_array_v<old_str_type>)
				old_str_size = std::basic_string_view<old_raw_type>{ old_str }.size();
			// char
			else
				old_str_size = sizeof(old_str) / sizeof(old_raw_type);
		}
		// std::string
		else
		{
			old_str_size = old_str.size();
		}

		if (old_str_size == size_type(0))
			return s;

		if constexpr (std::is_trivial_v<new_raw_type>)
		{
			if constexpr (std::is_pointer_v<new_str_type> || std::is_array_v<new_str_type>)
				new_str_size = std::basic_string_view<new_raw_type>{ new_str }.size();
			else
				new_str_size = sizeof(new_str) / sizeof(new_raw_type);
		}
		else
		{
			new_str_size = new_str.size();
		}

		for (size_type pos(0); pos != String::npos; pos += new_str_size)
		{
			pos = s.find(old_str, pos);
			if (pos != String::npos)
			{
				// to avoid this problem:
				// s = "a\r\nb"; replace(s, "\r", "\r\n"); s == "a\r\n\nb";
				// at this time, the s should be: s == "a\r\nb";
				if (old_str_size < new_str_size)
				{
					std::basic_string_view<value_type> v{ std::addressof(s[pos]), (std::min)(new_str_size, s.size() - pos) };
					if (v.size() == new_str_size && v.find(new_str) == size_type(0))
					{
						continue;
					}
				}

				// char* char[] char
				if constexpr (std::is_trivial_v<new_raw_type>)
				{
					// char* char[]
					if constexpr (std::is_pointer_v<new_str_type> || std::is_array_v<new_str_type>)
						s.replace(pos, old_str_size, new_str);
					// char
					else
						s.replace(pos, old_str_size, new_str_size, new_str);
				}
				// std::string
				else
				{
					s.replace(pos, old_str_size, new_str);
				}
			}
			else
			{
				break;
			}
		}
		return s;
	}

	template<class String1, class String2>
	inline std::size_t ifind(const String1& src, const String2& dest, std::string::size_type pos = 0) noexcept
	{
		using str1_type = std::remove_reference_t<std::remove_cv_t<String1>>;
		using raw1_type = std::remove_pointer_t<std::remove_all_extents_t<str1_type>>;

		using str2_type = std::remove_reference_t<std::remove_cv_t<String2>>;
		using raw2_type = std::remove_pointer_t<std::remove_all_extents_t<str2_type>>;

		std::basic_string_view<typename detail::char_type<raw1_type>::type> suc{ src  };
		std::basic_string_view<typename detail::char_type<raw2_type>::type> des{ dest };

		if (pos >= suc.size() || des.empty())
			return std::string::npos;

		// Outer loop
		for (auto OuterIt = std::next(suc.begin(), pos); OuterIt != suc.end(); ++OuterIt)
		{
			auto InnerIt = OuterIt;
			auto SubstrIt = des.begin();
			for (; InnerIt != suc.end() && SubstrIt != des.end(); ++InnerIt, ++SubstrIt)
			{
				if (std::tolower(*InnerIt) != std::tolower(*SubstrIt))
					break;
			}

			// Substring matching succeeded
			if (SubstrIt == des.end())
				return std::distance(suc.begin(), OuterIt);
		}

		return std::string::npos;
	}

	/** 
	 * @brief Returns `true` if two strings are equal, using a case-insensitive comparison.
	 */
	template<typename = void>
	inline bool iequals(std::string_view lhs, std::string_view rhs) noexcept
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
			if (detail::ascii_tolower(a) != detail::ascii_tolower(b))
				return false;
			a = *p1++;
			b = *p2++;
		} while (n--);
		return true;
	}
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_STRING_HPP__
