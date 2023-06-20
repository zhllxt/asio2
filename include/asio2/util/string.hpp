/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 * https://github.com/Shot511/strutil
 * 
 */

#ifndef __ASIO2_STRING_HPP__
#define __ASIO2_STRING_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#if defined(__GNUC__) || defined(__GNUG__)
#  pragma GCC diagnostic ignored "-Warray-bounds"
#endif

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
#include <sstream>
#include <regex>
#include <map>

#include <asio2/base/detail/type_traits.hpp>

namespace asio2
{
	/**
	 * @brief Converts any datatype into std::basic_string.
	 * @tparam T
	 * @param v - will be converted into std::basic_string.
	 * @return Converted value as std::basic_string.
	 */
	template<typename T>
	inline auto to_basic_string(T&& v)
	{
		using type  = typename detail::remove_cvref_t<T>;
		using CharT = typename detail::char_type<type>::type;

		if /**/ constexpr (detail::is_string_view_v<type>)
		{
			return std::basic_string<CharT>{ v.data(), v.size() };
		}
		else if constexpr (detail::is_string_v<type>)
		{
			return std::forward<T>(v);
		}
		else if constexpr (detail::is_char_v<type>)
		{
			return std::basic_string<CharT>{ 1, v };
		}
		else if constexpr (std::is_integral_v<type>)
		{
			std::basic_string<CharT> r;
			std::string s = std::to_string(v);
			for (auto c : s)
			{
				r += static_cast<CharT>(c);
			}
			return r;
		}
		else if constexpr (std::is_floating_point_v<type>)
		{
			std::basic_string<CharT> r;
			std::string s = std::to_string(v);
			for (auto c : s)
			{
				r += static_cast<CharT>(c);
			}
			return r;
		}
		else if constexpr (detail::is_char_pointer_v<type>)
		{
			if (v)
				return std::basic_string<CharT>{ v };
			else
				return std::basic_string<CharT>{ };
		}
		else if constexpr (detail::is_char_array_v<type>)
		{
			return std::basic_string<CharT>{ reinterpret_cast<const CharT*>(v) };
		}
		else
		{
			std::basic_stringstream<CharT> ss;
			ss << std::forward<T>(v);
			return ss.str();
		}
	}

	/**
	 * @brief Converts any datatype into std::basic_string_view.
	 * @tparam T
	 * @param v - will be converted into std::basic_string_view.
	 * @return Converted value as std::basic_string_view.
	 */
	template<typename T>
	inline auto to_basic_string_view(const T& v)
	{
		using type  = typename detail::remove_cvref_t<T>;
		using CharT = typename detail::char_type<type>::type;

		if /**/ constexpr (detail::is_string_view_v<type>)
		{
			return v;
		}
		else if constexpr (detail::is_string_v<type>)
		{
			return std::basic_string_view<CharT>{ v };
		}
		else if constexpr (detail::is_char_v<type>)
		{
			return std::basic_string_view<CharT>{ std::addressof(v), 1 };
		}
		else if constexpr (detail::is_char_pointer_v<type>)
		{
			return (v ? std::basic_string_view<CharT>{ v } : std::basic_string_view<CharT>{});
		}
		else if constexpr (detail::is_char_array_v<type>)
		{
			return std::basic_string_view<CharT>{ reinterpret_cast<const CharT*>(v) };
		}
		else
		{
			return std::basic_string_view<CharT>{ v };
		}
	}

	/**
	 * @brief Converts any datatype into std::string.
	 * @tparam T
	 * @param v - will be converted into std::string.
	 * @return Converted value as std::string.
	 */
	template<typename T>
	inline std::string to_string(T&& v)
	{
		using type = detail::remove_cvref_t<T>;

		std::string s;

		if /**/ constexpr (std::is_same_v<std::string_view, type>)
		{
			s = { v.data(), v.size() };
		}
		else if constexpr (std::is_same_v<std::string, type>)
		{
			s = std::forward<T>(v);
		}
		else if constexpr (std::is_integral_v<type>)
		{
			s = std::to_string(v);
		}
		else if constexpr (std::is_floating_point_v<type>)
		{
			s = std::to_string(v);
		}
		else if constexpr (detail::is_char_pointer_v<type>)
		{
			if (v) s = v;
		}
		else if constexpr (detail::is_char_array_v<type>)
		{
			s = std::forward<T>(v);
		}
		else
		{
			std::stringstream ss;
			ss << std::forward<T>(v);
			s = ss.str();
		}
		return s;
	}

	/**
	 * @brief Converts any datatype into std::string_view.
	 * @tparam T
	 * @param v - will be converted into std::string_view.
	 * @return Converted value as std::string_view.
	 */
	template<typename T>
	inline std::string_view to_string_view(const T& v)
	{
		using type = detail::remove_cvref_t<T>;

		if /**/ constexpr (std::is_same_v<std::string_view, type>)
		{
			return std::string_view{ v };
		}
		else if constexpr (std::is_same_v<std::string, type>)
		{
			return std::string_view{ v };
		}
		else if constexpr (detail::is_char_v<type>)
		{
			return std::string_view{ std::addressof(v), 1 };
		}
		else if constexpr (detail::is_char_pointer_v<type>)
		{
			return (v ? std::string_view{ v } : std::string_view{});
		}
		else if constexpr (detail::is_char_array_v<type>)
		{
			return std::string_view{ v };
		}
		else
		{
			return std::string_view{ v };
		}
	}

	/**
	 * @brief Converts iterator range into std::string_view.
	 * @tparam T
	 * @param v - will be converted into std::string_view.
	 * @return Converted value as std::string_view.
	 */
	template<typename Iterator>
	inline std::string_view to_string_view(const Iterator& first, const Iterator& last)
	{
		using iter_type = typename detail::remove_cvref_t<Iterator>;
		using diff_type = typename std::iterator_traits<iter_type>::difference_type;

		diff_type n = std::distance(first, last);

		if (n <= static_cast<diff_type>(0))
		{
			return std::string_view{};
		}

		if constexpr (std::is_pointer_v<iter_type>)
		{
			return { first, static_cast<std::string_view::size_type>(n) };
		}
		else
		{
			return { first.operator->(), static_cast<std::string_view::size_type>(n) };
		}
	}

	/**
	 * @brief Converts any datatype into a numeric.
	 * @tparam IntegerType - integer or floating
	 * @param v - will be converted into numeric.
	 * @return Converted value as numeric.
	 */
	template<typename IntegerType, typename T>
	inline IntegerType to_numeric(T&& v) noexcept
	{
		using type = detail::remove_cvref_t<T>;

		if /**/ constexpr (std::is_integral_v<type>)
		{
			return static_cast<IntegerType>(v);
		}
		else if constexpr (std::is_floating_point_v<type>)
		{
			return static_cast<IntegerType>(v);
		}
		else
		{
			std::string s = asio2::to_string(std::forward<T>(v));
			int rx = 10;
			if (s.size() >= std::size_t(2) && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
				rx = 16;
			return static_cast<IntegerType>(std::strtoull(s.data(), nullptr, rx));
		}
	}

	/**
	 * @brief Converts std::string into any datatype.
	 *        Datatype must support << operator.
	 * @tparam T
	 * @param str - std::string that will be converted into datatype T.
	 * @return Variable of datatype T.
	 */
	template<typename T>
	inline T string_to(const std::string& str)
	{
		T result{};
		std::istringstream(str) >> result;

		return result;
	}

	/**
	 * @brief Returns `true` if two strings are equal, using a case-insensitive comparison.
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>
	>
	inline bool iequals(
		std::basic_string_view<CharT, Traits> str1,
		std::basic_string_view<CharT, Traits> str2) noexcept
	{
		auto n = str1.size();
		if (str2.size() != n)
			return false;
		auto p1 = str1.data();
		auto p2 = str2.data();
		CharT a, b;
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

	/**
	 * @brief Returns `true` if two strings are equal, using a case-insensitive comparison.
	 */
	template<class String1, class String2>
	inline bool iequals(const String1& str1, const String2& str2) noexcept
	{
		return asio2::iequals(asio2::to_basic_string_view(str1), asio2::to_basic_string_view(str2));
	}

	/**
	 * @brief Compares two std::strings ignoring their case (lower/upper).
	 * @param str1 - string to compare
	 * @param str2 - string to compare
	 * @return True if str1 and str2 are equal, false otherwise.
	 */
	template<class String1, class String2>
	inline bool compare_ignore_case(const String1& str1, const String2& str2)
	{
		return asio2::iequals(str1, str2);
	}

	/**
	 * std::string format
	 */
	inline std::string formatv(const char * format, va_list args)
	{
		std::string str;

		if (format && *format)
		{
			// under windows and linux system,std::vsnprintf(nullptr, 0, format, args)
			// can get the need buffer len for the output,
			va_list args_copy;

			va_copy(args_copy, args);
			int len = std::vsnprintf(nullptr, 0, format, args_copy);

			if (len > 0)
			{
				str.resize(len);

				va_copy(args_copy, args);
				std::vsprintf((char*)str.data(), format, args_copy);
			}
		}

		return str;
	}

	/**
	 * std::wstring format
	 */
	inline std::wstring formatv(const wchar_t * format, va_list args)
	{
		std::wstring str;

		if (format && *format)
		{
			va_list args_copy;

			while (true)
			{
				str.resize(str.capacity());

				va_copy(args_copy, args);

				// if provided buffer size is less than required size,vswprintf will return -1
				// so if len equal -1,we increase the buffer size again, and has to use a loop 
				// to get the correct output buffer len,
				int len = std::vswprintf((wchar_t*)(&str[0]), str.size(), format, args_copy);
				if (len == -1)
					str.reserve(str.capacity() * 2);
				else
				{
					str.resize(len);
					break;
				}
			}
		}

		return str;
	}

	/**
	 * std::string format
	 */
	inline std::string format(const char * format, ...)
	{
		std::string str;

		if (format && *format)
		{
			// under windows and linux system,std::vsnprintf(nullptr, 0, format, args)
			// can get the need buffer len for the output,
			va_list args;
			va_start(args, format);

			str = formatv(format, args);

			va_end(args);
		}

		return str;
	}

	/**
	 * std::wstring format
	 */
	inline std::wstring format(const wchar_t * format, ...)
	{
		std::wstring str;

		if (format && *format)
		{
			va_list args;
			va_start(args, format);

			str = formatv(format, args);

			va_end(args);
		}

		return str;
	}

	/**
	 * @brief Converts string to lower case.
	 * @param str - string that needs to be converted.
	 * @return Lower case input string.
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator>& to_lower(std::basic_string<CharT, Traits, Allocator>& str)
	{
		std::transform(str.begin(), str.end(), str.begin(), [](CharT c) -> CharT
		{
			return static_cast<CharT>(std::tolower(c));
		});

		return str;
	}

	/**
	 * @brief Converts string to lower case.
	 * @param str - string that needs to be converted.
	 * @return Lower case input string.
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator> to_lower(std::basic_string<CharT, Traits, Allocator>&& str)
	{
		std::transform(str.begin(), str.end(), str.begin(), [](CharT c) -> CharT
		{
			return static_cast<CharT>(std::tolower(c));
		});

		return std::move(str);
	}

	/**
	 * @brief Converts string to upper case.
	 * @param str - string that needs to be converted.
	 * @return Upper case input string.
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator>& to_upper(std::basic_string<CharT, Traits, Allocator>& str)
	{
		std::transform(str.begin(), str.end(), str.begin(), [](CharT c) -> CharT
		{
			return static_cast<CharT>(std::toupper(c));
		});

		return str;
	}

	/**
	 * @brief Converts string to upper case.
	 * @param str - string that needs to be converted.
	 * @return Upper case input string.
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator> to_upper(std::basic_string<CharT, Traits, Allocator>&& str)
	{
		std::transform(str.begin(), str.end(), str.begin(), [](CharT c) -> CharT
		{
			return static_cast<CharT>(std::toupper(c));
		});

		return std::move(str);
	}

	/**
	 * @brief Converts the first character of a string to uppercase letter and lowercases all other characters, if any.
	 * @param str - input string to be capitalized.
	 * @return A string with the first letter capitalized and all other characters lowercased.
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator>& capitalize(std::basic_string<CharT, Traits, Allocator>& str)
	{
		asio2::to_lower(str);

		if (!str.empty())
		{
			str.front() = static_cast<CharT>(std::toupper(str.front()));
		}

		return str;
	}

	/**
	 * @brief Converts the first character of a string to uppercase letter and lowercases all other characters, if any.
	 * @param str - input string to be capitalized.
	 * @return A string with the first letter capitalized and all other characters lowercased.
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator> capitalize(std::basic_string<CharT, Traits, Allocator>&& str)
	{
		asio2::to_lower(str);

		if (!str.empty())
		{
			str.front() = static_cast<CharT>(std::toupper(str.front()));
		}

		return std::move(str);
	}

	/**
	 * @brief Converts only the first character of a string to uppercase letter, all other characters stay unchanged.
	 * @param str - input string to be modified.
	 * @return A string with the first letter capitalized. All other characters stay unchanged.
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator>& capitalize_first_char(
		std::basic_string<CharT, Traits, Allocator>& str)
	{
		if (!str.empty())
		{
			str.front() = static_cast<CharT>(std::toupper(str.front()));
		}

		return str;
	}

	/**
	 * @brief Converts only the first character of a string to uppercase letter, all other characters stay unchanged.
	 * @param str - input string to be modified.
	 * @return A string with the first letter capitalized. All other characters stay unchanged.
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator> capitalize_first_char(
		std::basic_string<CharT, Traits, Allocator>&& str)
	{
		if (!str.empty())
		{
			str.front() = static_cast<CharT>(std::toupper(str.front()));
		}

		return std::move(str);
	}

	/**
	 * @brief Checks if input string str contains specified substring.
	 * @param str - string to be checked.
	 * @param substring - searched substring or character.
	 * @return True if substring or character was found in str, false otherwise.
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>
	>
	inline bool contains(std::basic_string_view<CharT, Traits> str, std::basic_string_view<CharT, Traits> substring)
	{
		return str.find(substring) != std::string_view::npos;
	}

	/**
	 * @brief Checks if input string str contains specified substring.
	 * @param str - string to be checked.
	 * @param substring - searched substring or character.
	 * @return True if substring was found in str, false otherwise.
	 */
	template<class String1, class String2>
	inline bool contains(const String1& str, const String2& substring)
	{
		return asio2::contains(asio2::to_basic_string_view(str), asio2::to_basic_string_view(substring));
	}

	/**
	 * @brief trim each space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator>& trim_all(std::basic_string<CharT, Traits, Allocator>& str)
	{
		// https://zh.cppreference.com/w/cpp/algorithm/remove
		str.erase(std::remove_if(str.begin(), str.end(), [](int x) {return std::isspace(x); }), str.end());
		return str;
	}

	/**
	 * @brief trim each space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator> trim_all(std::basic_string<CharT, Traits, Allocator>&& str)
	{
		// https://zh.cppreference.com/w/cpp/algorithm/remove
		str.erase(std::remove_if(str.begin(), str.end(), [](int x) {return std::isspace(x); }), str.end());
		return std::move(str);
	}

	/**
	 * @brief trim left space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator>& trim_left(std::basic_string<CharT, Traits, Allocator>& str)
	{
		str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int ch) { return !std::isspace(ch); }));
		return str;
	}

	/**
	 * @brief trim left space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator> trim_left(std::basic_string<CharT, Traits, Allocator>&& str)
	{
		str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int ch) { return !std::isspace(ch); }));
		return std::move(str);
	}

	/**
	 * @brief trim left space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator>& ltrim(std::basic_string<CharT, Traits, Allocator>& str)
	{
		return asio2::trim_left(str);
	}

	/**
	 * @brief trim left space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator> ltrim(std::basic_string<CharT, Traits, Allocator>&& str)
	{
		return asio2::trim_left(std::move(str));
	}

	/**
	 * @brief trim right space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator>& trim_right(std::basic_string<CharT, Traits, Allocator>& str)
	{
		str.erase(std::find_if(str.rbegin(), str.rend(), [](int ch) { return !std::isspace(ch); }).base(), str.end());
		return str;
	}

	/**
	 * @brief trim right space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator> trim_right(std::basic_string<CharT, Traits, Allocator>&& str)
	{
		str.erase(std::find_if(str.rbegin(), str.rend(), [](int ch) { return !std::isspace(ch); }).base(), str.end());
		return std::move(str);
	}

	/**
	 * @brief trim right space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator>& rtrim(std::basic_string<CharT, Traits, Allocator>& str)
	{
		return asio2::trim_right(str);
	}

	/**
	 * @brief trim right space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator> rtrim(std::basic_string<CharT, Traits, Allocator>&& str)
	{
		return asio2::trim_right(std::move(str));
	}

	/**
	 * @brief trim left and right space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator>& trim_both(std::basic_string<CharT, Traits, Allocator>& str)
	{
		return trim_right(trim_left(str));
	}

	/**
	 * @brief trim left and right space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator> trim_both(std::basic_string<CharT, Traits, Allocator>&& str)
	{
		return trim_right(trim_left(std::move(str)));
	}

	/**
	 * @brief trim left and right space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator>& trim(std::basic_string<CharT, Traits, Allocator>& str)
	{
		return asio2::trim_both(str);
	}

	/**
	 * @brief trim left and right space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator> trim(std::basic_string<CharT, Traits, Allocator>&& str)
	{
		return asio2::trim_both(std::move(str));
	}

	/**
	 * @brief Trims white spaces from the left side of string.
	 * @param str - input string to remove white spaces from.
	 * @return Copy of input str with trimmed white spaces.
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator> trim_left_copy(std::basic_string<CharT, Traits, Allocator> str)
	{
		return asio2::trim_left(std::move(str));
	}

	/**
	 * @brief Trims white spaces from the left side of string.
	 * @param str - input string to remove white spaces from.
	 * @return Copy of input str with trimmed white spaces.
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator> ltrim_copy(std::basic_string<CharT, Traits, Allocator> str)
	{
		return asio2::trim_left(std::move(str));
	}

	/**
	  * @brief Trims white spaces from the right side of string.
	  * @param str - input string to remove white spaces from.
	  * @return Copy of input str with trimmed white spaces.
	  */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator> trim_right_copy(std::basic_string<CharT, Traits, Allocator> str)
	{
		return asio2::trim_right(std::move(str));
	}

	/**
	  * @brief Trims white spaces from the right side of string.
	  * @param str - input string to remove white spaces from.
	  * @return Copy of input str with trimmed white spaces.
	  */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator> rtrim_copy(std::basic_string<CharT, Traits, Allocator> str)
	{
		return asio2::trim_right(std::move(str));
	}

	/**
	  * @brief Trims white spaces from the both sides of string.
	  * @param str - input string to remove white spaces from.
	  * @return Copy of input str with trimmed white spaces.
	  */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator> trim_copy(std::basic_string<CharT, Traits, Allocator> str)
	{
		return asio2::trim(std::move(str));
	}

	/**
	 * @brief trim left space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>
	>
	inline std::basic_string_view<CharT, Traits>& trim_left(std::basic_string_view<CharT, Traits>& str)
	{
		if (str.empty())
			return str;
		using size_type = typename std::basic_string_view<CharT, Traits>::size_type;
		size_type pos = 0;
		for (; pos < str.size(); ++pos)
		{
			if (!std::isspace(static_cast<unsigned char>(str[pos])))
				break;
		}
		str.remove_prefix(pos);
		return str;
	}

	/**
	 * @brief trim left space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>
	>
	inline std::basic_string_view<CharT, Traits> trim_left(std::basic_string_view<CharT, Traits>&& str)
	{
		if (str.empty())
			return std::move(str);
		using size_type = typename std::basic_string_view<CharT, Traits>::size_type;
		size_type pos = 0;
		for (; pos < str.size(); ++pos)
		{
			if (!std::isspace(static_cast<unsigned char>(str[pos])))
				break;
		}
		str.remove_prefix(pos);
		return std::move(str);
	}

	/**
	 * @brief trim left space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>
	>
	inline std::basic_string_view<CharT, Traits>& ltrim(std::basic_string_view<CharT, Traits>& str)
	{
		return asio2::trim_left(str);
	}

	/**
	 * @brief trim left space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>
	>
	inline std::basic_string_view<CharT, Traits> ltrim(std::basic_string_view<CharT, Traits>&& str)
	{
		return asio2::trim_left(std::move(str));
	}

	/**
	 * @brief trim right space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>
	>
	inline std::basic_string_view<CharT, Traits>& trim_right(std::basic_string_view<CharT, Traits>& str)
	{
		if (str.empty())
			return str;
		using size_type = typename std::basic_string_view<CharT, Traits>::size_type;
		size_type pos = str.size() - 1;
		for (; pos != size_type(-1); pos--)
		{
			if (!std::isspace(static_cast<unsigned char>(str[pos])))
				break;
		}
		str.remove_suffix(str.size() - pos - 1);
		return str;
	}

	/**
	 * @brief trim right space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>
	>
	inline std::basic_string_view<CharT, Traits> trim_right(std::basic_string_view<CharT, Traits>&& str)
	{
		if (str.empty())
			return std::move(str);
		using size_type = typename std::basic_string_view<CharT, Traits>::size_type;
		size_type pos = str.size() - 1;
		for (; pos != size_type(-1); pos--)
		{
			if (!std::isspace(static_cast<unsigned char>(str[pos])))
				break;
		}
		str.remove_suffix(str.size() - pos - 1);
		return std::move(str);
	}

	/**
	 * @brief trim right space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>
	>
	inline std::basic_string_view<CharT, Traits>& rtrim(std::basic_string_view<CharT, Traits>& str)
	{
		return asio2::trim_right(str);
	}

	/**
	 * @brief trim right space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>
	>
	inline std::basic_string_view<CharT, Traits> rtrim(std::basic_string_view<CharT, Traits>&& str)
	{
		return asio2::trim_right(std::move(str));
	}

	/**
	 * @brief trim left and right space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>
	>
	inline std::basic_string_view<CharT, Traits>& trim_both(std::basic_string_view<CharT, Traits>& str)
	{
		return asio2::trim_right(asio2::trim_left(str));
	}

	/**
	 * @brief trim left and right space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>
	>
	inline std::basic_string_view<CharT, Traits> trim_both(std::basic_string_view<CharT, Traits>&& str)
	{
		return asio2::trim_right(asio2::trim_left(std::move(str)));
	}

	/**
	 * @brief trim left and right space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>
	>
	inline std::basic_string_view<CharT, Traits>& trim(std::basic_string_view<CharT, Traits>& str)
	{
		return asio2::trim_both(str);
	}

	/**
	 * @brief trim left and right space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>
	>
	inline std::basic_string_view<CharT, Traits> trim(std::basic_string_view<CharT, Traits>&& str)
	{
		return asio2::trim_both(std::move(str));
	}

	/**
	 * @brief Replaces (in-place) the first occurrence of target with replacement.
	 * @param str - input string that will be modified.
	 * @param target - substring that will be replaced with replacement.
	 * @param replacement - substring that will replace target.
	 * @return Replacemented input string.
	 */
	template<
		class String1,
		class String2,
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator>& replace_first(
		std::basic_string<CharT, Traits, Allocator>& str,
		const String1& target,
		const String2& replacement)
	{
		auto t = asio2::to_basic_string_view(target);
		auto r = asio2::to_basic_string_view(replacement);

		const std::size_t start_pos = str.find(t);
		if (start_pos == std::string::npos)
		{
			return str;
		}

		str.replace(start_pos, t.length(), r);
		return str;
	}

	/**
	 * @brief Replaces (in-place) the first occurrence of target with replacement.
	 * @param str - input string that will be modified.
	 * @param target - substring that will be replaced with replacement.
	 * @param replacement - substring that will replace target.
	 * @return Replacemented input string.
	 */
	template<
		class String1,
		class String2,
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator> replace_first(
		std::basic_string<CharT, Traits, Allocator>&& str,
		const String1& target,
		const String2& replacement)
	{
		auto t = asio2::to_basic_string_view(target);
		auto r = asio2::to_basic_string_view(replacement);

		const std::size_t start_pos = str.find(t);
		if (start_pos == std::string::npos)
		{
			return std::move(str);
		}

		str.replace(start_pos, t.length(), r);
		return std::move(str);
	}

	/**
	 * @brief Replaces (in-place) last occurrence of target with replacement.
	 * @param str - input string that will be modified.
	 * @param target - substring that will be replaced with replacement.
	 * @param replacement - substring that will replace target.
	 * @return Replacemented input string.
	 */
	template<
		class String1,
		class String2,
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator>& replace_last(
		std::basic_string<CharT, Traits, Allocator>& str,
		const String1& target,
		const String2& replacement)
	{
		auto t = asio2::to_basic_string_view(target);
		auto r = asio2::to_basic_string_view(replacement);

		std::size_t start_pos = str.rfind(t);
		if (start_pos == std::string::npos)
		{
			return str;
		}

		str.replace(start_pos, t.length(), r);
		return str;
	}

	/**
	 * @brief Replaces (in-place) last occurrence of target with replacement.
	 * @param str - input string that will be modified.
	 * @param target - substring that will be replaced with replacement.
	 * @param replacement - substring that will replace target.
	 * @return Replacemented input string.
	 */
	template<
		class String1,
		class String2,
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator> replace_last(
		std::basic_string<CharT, Traits, Allocator>&& str,
		const String1& target,
		const String2& replacement)
	{
		auto t = asio2::to_basic_string_view(target);
		auto r = asio2::to_basic_string_view(replacement);

		std::size_t start_pos = str.rfind(t);
		if (start_pos == std::string::npos)
		{
			return std::move(str);
		}

		str.replace(start_pos, t.length(), r);
		return std::move(str);
	}

	/**
	 * @brief Replaces (in-place) all occurrences of target with replacement.
	 * @param str - input string that will be modified.
	 * @param target - substring that will be replaced with replacement.
	 * @param replacement - substring that will replace target.
	 * @return Replacemented input string.
	 */
	template<
		class String1,
		class String2,
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator>& replace_all(
		std::basic_string<CharT, Traits, Allocator>& str,
		const String1& target,
		const String2& replacement)
	{
		auto t = asio2::to_basic_string_view(target);
		auto r = asio2::to_basic_string_view(replacement);

		if (t.empty())
		{
			return str;
		}

		std::size_t start_pos = 0;

		while ((start_pos = str.find(t, start_pos)) != std::string::npos)
		{
			str.replace(start_pos, t.length(), r);
			start_pos += r.length();
		}

		return str;
	}

	/**
	 * @brief Replaces (in-place) all occurrences of target with replacement.
	 * @param str - input string that will be modified.
	 * @param target - substring that will be replaced with replacement.
	 * @param replacement - substring that will replace target.
	 * @return Replacemented input string.
	 */
	template<
		class String1,
		class String2,
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator> replace_all(
		std::basic_string<CharT, Traits, Allocator>&& str,
		const String1& target,
		const String2& replacement)
	{
		auto t = asio2::to_basic_string_view(target);
		auto r = asio2::to_basic_string_view(replacement);

		if (t.empty())
		{
			return std::move(str);
		}

		std::size_t start_pos = 0;

		while ((start_pos = str.find(t, start_pos)) != std::string::npos)
		{
			str.replace(start_pos, t.length(), r);
			start_pos += r.length();
		}

		return std::move(str);
	}

	/**
	 * @brief Replaces (in-place) all occurrences of target with replacement.
	 * @param str - input string that will be modified.
	 * @param target - substring that will be replaced with replacement.
	 * @param replacement - substring that will replace target.
	 * @return Replacemented input string.
	 */
	template<
		class String1,
		class String2,
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator>& replace(
		std::basic_string<CharT, Traits, Allocator>& str,
		const String1& target,
		const String2& replacement)
	{
		return asio2::replace_all(str, target, replacement);
	}

	/**
	 * @brief Replaces (in-place) all occurrences of target with replacement.
	 * @param str - input string that will be modified.
	 * @param target - substring that will be replaced with replacement.
	 * @param replacement - substring that will replace target.
	 * @return Replacemented input string.
	 */
	template<
		class String1,
		class String2,
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator> replace(
		std::basic_string<CharT, Traits, Allocator>&& str,
		const String1& target,
		const String2& replacement)
	{
		return asio2::replace_all(std::move(str), target, replacement);
	}

	/**
	 * @brief Checks if string str ends with specified suffix.
	 * @param str - input string that will be checked.
	 * @param suffix - searched suffix in str.
	 * @return True if suffix was found, false otherwise.
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>
	>
	inline bool ends_with(
		std::basic_string_view<CharT, Traits> str,
		std::basic_string_view<CharT, Traits> suffix)
	{
		const auto suffix_start = str.size() - suffix.size();
		const auto result = str.find(suffix, suffix_start);
		return (result == suffix_start) && (result != std::string_view::npos);
	}

	/**
	 * @brief Checks if string str ends with specified suffix.
	 * @param str - input string that will be checked.
	 * @param suffix - searched suffix in str.
	 * @return True if suffix was found, false otherwise.
	 */
	template<class String1, class String2>
	inline bool ends_with(const String1& str1, const String2& str2)
	{
		return asio2::ends_with(asio2::to_basic_string_view(str1), asio2::to_basic_string_view(str2));
	}

	/**
	 * @brief Checks if string str starts with specified prefix.
	 * @param str - input string that will be checked.
	 * @param prefix - searched prefix in str.
	 * @return True if prefix was found, false otherwise.
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>
	>
	inline bool starts_with(
		std::basic_string_view<CharT, Traits> str,
		std::basic_string_view<CharT, Traits> prefix)
	{
		return str.rfind(prefix, 0) == 0;
	}

	/**
	 * @brief Checks if string str starts with specified prefix.
	 * @param str - input string that will be checked.
	 * @param prefix - searched prefix in str.
	 * @return True if prefix was found, false otherwise.
	 */
	template<class String1, class String2>
	inline bool starts_with(const String1& str1, const String2& str2)
	{
		return asio2::starts_with(asio2::to_basic_string_view(str1), asio2::to_basic_string_view(str2));
	}

	/**
	 * @brief Splits input string str according to input string delim.
	 * @param str - string that will be split.
	 * @param delim - the delimiter.
	 * @return std::vector<string> that contains all splitted tokens.
	 */
	template<
		class String1,
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::vector<std::basic_string<CharT, Traits, Allocator>> split(
		const std::basic_string<CharT, Traits, Allocator>& str,
		const String1& delim)
	{
		auto d = asio2::to_basic_string_view(delim);

		std::size_t pos_start = 0, pos_end, delim_len = d.length();
		std::basic_string<CharT, Traits, Allocator> token;
		std::vector<std::basic_string<CharT, Traits, Allocator>> tokens;

		while ((pos_end = str.find(d, pos_start)) != std::string::npos)
		{
			token = str.substr(pos_start, pos_end - pos_start);
			pos_start = pos_end + delim_len;
			tokens.emplace_back(std::move(token));
		}

		tokens.emplace_back(str.substr(pos_start));
		return tokens;
	}

	/**
	 * @brief Splits input string str according to input string delim.
	 * @param str - string that will be split.
	 * @param delim - the delimiter.
	 * @return std::vector<string_view> that contains all splitted tokens.
	 */
	template<
		class String1,
		class CharT,
		class Traits = std::char_traits<CharT>
	>
	inline std::vector<std::basic_string_view<CharT, Traits>> split(
		const std::basic_string_view<CharT, Traits>& str,
		const String1& delim)
	{
		auto d = asio2::to_basic_string_view(delim);

		std::size_t pos_start = 0, pos_end, delim_len = d.length();
		std::basic_string_view<CharT, Traits> token;
		std::vector<std::basic_string_view<CharT, Traits>> tokens;

		while ((pos_end = str.find(d, pos_start)) != std::string::npos)
		{
			token = str.substr(pos_start, pos_end - pos_start);
			pos_start = pos_end + delim_len;
			tokens.emplace_back(std::move(token));
		}

		tokens.emplace_back(str.substr(pos_start));
		return tokens;
	}

	/**
	 * @brief Splits input string str according to input string delim.
	 * @param str - string that will be split.
	 * @param delim - the delimiter.
	 * @return std::vector<string> that contains all splitted tokens.
	 */
	template<class String1, class String2>
	inline auto split(const String1& str, const String2& delim)
	{
		using CharT = typename detail::char_type<String1>::type;

		auto s = asio2::to_basic_string_view(str);
		auto d = asio2::to_basic_string_view(delim);

		std::size_t pos_start = 0, pos_end, delim_len = d.length();
		std::basic_string<CharT> token;
		std::vector<std::basic_string<CharT>> tokens;

		while ((pos_end = s.find(d, pos_start)) != std::string::npos)
		{
			token = s.substr(pos_start, pos_end - pos_start);
			pos_start = pos_end + delim_len;
			tokens.emplace_back(std::move(token));
		}

		tokens.emplace_back(s.substr(pos_start));
		return tokens;
	}

	/**
	 * @brief Splits input string using regex as a delimiter.
	 * @param src - string that will be split.
	 * @param rgx_str - the set of delimiter characters.
	 * @return vector of resulting tokens.
	 */
	template<class String1, class String2>
	inline auto regex_split(const String1& src, const String2& rgx_str)
	{
		using CharT = typename detail::char_type<String1>::type;

		auto s = asio2::to_basic_string(src);
		auto d = asio2::to_basic_string_view(rgx_str);

		using IterType = typename std::basic_string<CharT>::const_iterator;

		std::vector<std::basic_string<CharT>> elems;
		const std::basic_regex<CharT> rgx(d.begin(), d.end());
		std::regex_token_iterator<IterType> iter(s.begin(), s.end(), rgx, -1);
		std::regex_token_iterator<IterType> end;
		while (iter != end)
		{
			elems.emplace_back(*iter);
			++iter;
		}
		return elems;
	}

	/**
	 * @brief Splits input string using regex as a delimiter.
	 * @param src - string that will be split.
	 * @param dest - map of matched delimiter and those being splitted.
	 * @param rgx_str - the set of delimiter characters.
	 * @return True if the parsing is successfully done.
	 */
	template<class String1, class String2>
	inline auto regex_split_map(const String1& src, const String2& rgx_str)
	{
		using CharT = typename detail::char_type<String1>::type;

		auto d = asio2::to_basic_string_view(rgx_str);

		using IterType = typename std::basic_string<CharT>::const_iterator;

		std::map<std::basic_string<CharT>, std::basic_string<CharT>> dest;
		std::basic_string<CharT> tstr = src + static_cast<CharT>(' ');
		std::basic_regex<CharT> rgx(d.begin(), d.end());
		std::regex_token_iterator<IterType> niter(tstr.begin(), tstr.end(), rgx);
		std::regex_token_iterator<IterType> viter(tstr.begin(), tstr.end(), rgx, -1);
		std::regex_token_iterator<IterType> end;
		++viter;
		while (niter != end)
		{
			dest[*niter] = *viter;
			++niter;
			++viter;
		}

		return dest;
	}

	/**
	 * @brief Splits input string using any delimiter in the given set.
	 * @param str - string that will be split.
	 * @param delims - the set of delimiter characters.
	 * @return vector of resulting tokens.
	 */
	template<
		class String1,
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::vector<std::basic_string<CharT, Traits, Allocator>> split_any(
		const std::basic_string<CharT, Traits, Allocator>& str,
		const String1& delims)
	{
		auto d = asio2::to_basic_string_view(delims);

		std::basic_string<CharT, Traits, Allocator> token;
		std::vector<std::basic_string<CharT, Traits, Allocator>> tokens;

		std::size_t pos_start = 0;
		for (std::size_t pos_end = 0; pos_end < str.length(); ++pos_end)
		{
			if (asio2::contains(d, str[pos_end]))
			{
				token = str.substr(pos_start, pos_end - pos_start);
				tokens.emplace_back(std::move(token));
				pos_start = pos_end + 1;
			}
		}

		tokens.emplace_back(str.substr(pos_start));
		return tokens;
	}

	/**
	 * @brief Splits input string using any delimiter in the given set.
	 * @param str - string that will be split.
	 * @param delims - the set of delimiter characters.
	 * @return vector of resulting tokens.
	 */
	template<
		class String1,
		class CharT,
		class Traits = std::char_traits<CharT>
	>
	inline std::vector<std::basic_string_view<CharT, Traits>> split_any(
		const std::basic_string_view<CharT, Traits>& str,
		const String1& delims)
	{
		auto d = asio2::to_basic_string_view(delims);

		std::basic_string_view<CharT, Traits> token;
		std::vector<std::basic_string_view<CharT, Traits>> tokens;

		std::size_t pos_start = 0;
		for (std::size_t pos_end = 0; pos_end < str.length(); ++pos_end)
		{
			if (asio2::contains(d, str[pos_end]))
			{
				token = str.substr(pos_start, pos_end - pos_start);
				tokens.emplace_back(std::move(token));
				pos_start = pos_end + 1;
			}
		}

		tokens.emplace_back(str.substr(pos_start));
		return tokens;
	}

	/**
	 * @brief Splits input string using any delimiter in the given set.
	 * @param str - string that will be split.
	 * @param delims - the set of delimiter characters.
	 * @return vector of resulting tokens.
	 */
	template<class String1, class String2>
	inline auto split_any(const String1& str, const String2& delims)
	{
		using CharT = typename detail::char_type<String1>::type;

		auto s = asio2::to_basic_string_view(str);
		auto d = asio2::to_basic_string_view(delims);

		std::basic_string<CharT> token;
		std::vector<std::basic_string<CharT>> tokens;

		std::size_t pos_start = 0;
		for (std::size_t pos_end = 0; pos_end < s.length(); ++pos_end)
		{
			if (asio2::contains(d, s[pos_end]))
			{
				token = s.substr(pos_start, pos_end - pos_start);
				tokens.emplace_back(std::move(token));
				pos_start = pos_end + 1;
			}
		}

		tokens.emplace_back(s.substr(pos_start));
		return tokens;
	}

	/**
	 * @brief Joins all elements of std::vector tokens of arbitrary datatypes
	 *        into one string with delimiter delim.
	 * @tparam T - arbitrary datatype.
	 * @param tokens - vector of tokens.
	 * @param delim - the delimiter.
	 * @return string with joined elements of vector tokens with delimiter delim.
	 */
	template<
		class T,
		class String1
	>
	inline auto join(const std::vector<T>& tokens, const String1& delim)
	{
		using CharT = typename detail::char_type<String1>::type;

		std::basic_ostringstream<CharT> result;
		for (auto it = tokens.begin(); it != tokens.end(); ++it)
		{
			if (it != tokens.begin())
			{
				result << delim;
			}

			result << *it;
		}

		return result.str();
	}

	/**
	 * @brief Inplace removal of all empty strings in a vector<string>
	 * @param tokens - vector of strings.
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline void drop_empty(std::vector<std::basic_string<CharT, Traits, Allocator>>& tokens)
	{
		auto last = std::remove_if(tokens.begin(), tokens.end(),
			[](const std::basic_string<CharT, Traits, Allocator>& s) { return s.empty(); });
		tokens.erase(last, tokens.end());
	}

	/**
	 * @brief Inplace removal of all empty strings in a vector<string>
	 * @param tokens - vector of strings.
	 * @return vector of non-empty tokens.
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::vector<std::basic_string<CharT, Traits, Allocator>> drop_empty_copy(
		std::vector<std::basic_string<CharT, Traits, Allocator>> tokens)
	{
		drop_empty(tokens);
		return tokens;
	}

	/**
	 * @brief Inplace removal of all duplicate strings in a vector<string> where order is not to be maintained
	 *        Taken from: C++ Primer V5
	 * @param tokens - vector of strings.
	 * @return vector of non-duplicate tokens.
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline void drop_duplicate(std::vector<std::basic_string<CharT, Traits, Allocator>>& tokens)
	{
		std::sort(tokens.begin(), tokens.end());
		auto end_unique = std::unique(tokens.begin(), tokens.end());
		tokens.erase(end_unique, tokens.end());
	}

	/**
	 * @brief Removal of all duplicate strings in a vector<string> where order is not to be maintained
	 *        Taken from: C++ Primer V5
	 * @param tokens - vector of strings.
	 * @return vector of non-duplicate tokens.
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::vector<std::basic_string<CharT, Traits, Allocator>> drop_duplicate_copy(
		std::vector<std::basic_string<CharT, Traits, Allocator>> tokens)
	{
		std::sort(tokens.begin(), tokens.end());
		auto end_unique = std::unique(tokens.begin(), tokens.end());
		tokens.erase(end_unique, tokens.end());
		return tokens;
	}

	/**
	 * @brief Creates new string with repeated n times substring str.
	 * @param str - substring that needs to be repeated.
	 * @param n - number of iterations.
	 * @return string with repeated substring str.
	 */
	template<class String1>
	inline auto repeat(const String1& str, unsigned n)
	{
		using CharT = typename detail::char_type<String1>::type;

		std::basic_string<CharT> result;

		for (unsigned i = 0; i < n; ++i)
		{
			result += str;
		}

		return result;
	}

	/**
	 * @brief Checks if input string str matches specified reular expression regex.
	 * @param str - string to be checked.
	 * @param regex - the std::regex regular expression.
	 * @return True if regex matches str, false otherwise.
	 */
	template<class String1>
	inline bool matches(const String1& str, const std::basic_regex<typename detail::char_type<String1>::type>& regex)
	{
		return std::regex_match(str, regex);
	}

	/**
	 * @brief Sort input std::vector<string> strs in ascending order.
	 * @param strs - std::vector<string> to be checked.
	 */
	template<typename T>
	inline void sorting_ascending(std::vector<T>& strs)
	{
		std::sort(strs.begin(), strs.end());
	}

	/**
	 * @brief Sorted input std::vector<string> strs in descending order.
	 * @param strs - std::vector<string> to be checked.
	 */
	template<typename T>
	inline void sorting_descending(std::vector<T>& strs)
	{
		std::sort(strs.begin(), strs.end(), std::greater<T>());
	}

	/**
	 * @brief Reverse input std::vector<string> strs.
	 * @param strs - std::vector<string> to be checked.
	 */
	template<typename T>
	inline void reverse_inplace(std::vector<T>& strs)
	{
		std::reverse(strs.begin(), strs.end());
	}

	/**
	 * @brief Reverse input std::vector<string> strs.
	 * @param strs - std::vector<string> to be checked.
	 */
	template<typename T>
	inline std::vector<T> reverse_copy(std::vector<T> strs)
	{
		std::reverse(strs.begin(), strs.end());
		return strs;
	}

	/**
	 * @brief Find substring in the string src, using a case-insensitive comparison.
	 * @return The finded index, or std::string::npos if not found.
	 */
	template<class String1, class String2>
	inline std::size_t ifind(const String1& src, const String2& dest, std::string::size_type pos = 0) noexcept
	{
		auto s = asio2::to_basic_string_view(src);
		auto d = asio2::to_basic_string_view(dest);

		if (pos >= s.size() || d.empty())
			return std::string::npos;

		// Outer loop
		for (auto OuterIt = std::next(s.begin(), pos); OuterIt != s.end(); ++OuterIt)
		{
			auto InnerIt = OuterIt;
			auto SubstrIt = d.begin();
			for (; InnerIt != s.end() && SubstrIt != d.end(); ++InnerIt, ++SubstrIt)
			{
				if (std::tolower(*InnerIt) != std::tolower(*SubstrIt))
					break;
			}

			// Substring matching succeeded
			if (SubstrIt == d.end())
				return std::distance(s.begin(), OuterIt);
		}

		return std::string::npos;
	}
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_STRING_HPP__
