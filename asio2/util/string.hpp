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
#  pragma warning(push) 
#  pragma warning(disable:4311)
#  pragma warning(disable:4312)
#  pragma warning(disable:4996)
#endif

#if defined(__GNUC__) || defined(__GNUG__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wunused-variable"
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunused-variable"
#  pragma clang diagnostic ignored "-Wexceptions"
#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
#  pragma clang diagnostic ignored "-Wunused-private-field"
#  pragma clang diagnostic ignored "-Wunused-local-typedef"
#  pragma clang diagnostic ignored "-Wunknown-warning-option"
#endif

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
		inline char ascii_tolower(char c)
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
	 * @function : trim each space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
		std::basic_string<CharT, Traits, Allocator>& trim_all(
			std::basic_string<CharT, Traits, Allocator>& s)
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
	 * @function : trim left space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
		std::basic_string<CharT, Traits, Allocator>& trim_left(
			std::basic_string<CharT, Traits, Allocator>& s)
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
	 * @function : trim right space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
		std::basic_string<CharT, Traits, Allocator>& trim_right(
			std::basic_string<CharT, Traits, Allocator>& s)
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
	 * @function : trim left and right space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
		std::basic_string<CharT, Traits, Allocator>& trim_both(
			std::basic_string<CharT, Traits, Allocator>& s)
	{
		trim_left(s);
		trim_right(s);
		return s;
	}

	/**
	 * @function : trim left space character of the string: space \t \r \n and so on
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
	 * @function : trim right space character of the string: space \t \r \n and so on
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
	 * @function : trim left and right space character of the string: space \t \r \n and so on
	 */
	template<class CharT, class Traits = std::char_traits<CharT>>
	std::basic_string_view<CharT, Traits>& trim_both(std::basic_string_view<CharT, Traits>& s)
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

	/**
	 * @function : Replaces all old_str characters that appear in the string with new_str characters.
	 */
	template<class String, class OldStr, class NewStr>
	inline String& replace(String& s, const OldStr& old_str, const NewStr& new_str)
	{
		using size_type = typename String::size_type;
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
				if constexpr (std::is_pointer_v<new_str_type> || std::is_array_v<new_str_type>)
					s.replace(pos, old_str_size, new_str);
				else
					s.replace(pos, old_str_size, new_str_size, new_str);
			}
			else
				break;
		}
		return s;
	}

	template<class String1, class String2>
	inline std::size_t ifind(const String1& src, const String2& dest, typename String1::size_type pos = 0)
	{
		using str2_type = std::remove_reference_t<std::remove_cv_t<String2>>;
		using raw2_type = std::remove_pointer_t<std::remove_all_extents_t<str2_type>>;

		std::basic_string_view<typename detail::char_type<raw2_type>::type> des{ dest };

		if (pos >= src.size() || des.empty())
			return String1::npos;

		// Outer loop
		for (auto OuterIt = std::next(src.begin(), pos); OuterIt != src.end(); ++OuterIt)
		{
			auto InnerIt = OuterIt;
			auto SubstrIt = des.begin();
			for (; InnerIt != src.end() && SubstrIt != des.end(); ++InnerIt, ++SubstrIt)
			{
				if (std::tolower(*InnerIt) != std::tolower(*SubstrIt))
					break;
			}

			// Substring matching succeeded
			if (SubstrIt == des.end())
				return std::distance(src.begin(), OuterIt);
		}

		return String1::npos;
	}

	/** 
	 * @function : Returns `true` if two strings are equal, using a case-insensitive comparison.
	 */
	template<typename = void>
	inline bool iequals(std::string_view lhs, std::string_view rhs)
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

#if defined(__clang__)
#  pragma clang diagnostic pop
#endif

#if defined(__GNUC__) || defined(__GNUG__)
#  pragma GCC diagnostic pop
#endif

#if defined(_MSC_VER)
#  pragma warning(pop) 
#endif

#endif // !__ASIO2_STRING_HPP__
