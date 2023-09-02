/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_TYPE_TRAITS_HPP__
#define __ASIO2_TYPE_TRAITS_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>
#include <cctype>

#include <string>
#include <string_view>
#include <type_traits>
#include <memory>
#include <functional>
#include <tuple>
#include <utility>

namespace asio2::detail
{
	template<class T>
	struct remove_cvref
	{
		typedef std::remove_cv_t<std::remove_reference_t<T>> type;
	};

	template< class T >
	using remove_cvref_t = typename remove_cvref<T>::type;


	template <typename Enumeration>
	inline constexpr auto to_underlying(Enumeration const value) noexcept ->
		typename std::underlying_type<Enumeration>::type
	{
		return static_cast<typename std::underlying_type<Enumeration>::type>(value);
	}


	template <typename... Ts>
	inline constexpr void ignore_unused(Ts const& ...) noexcept {}

	template <typename... Ts>
	inline constexpr void ignore_unused() noexcept {}


	// https://stackoverflow.com/questions/53945490/how-to-assert-that-a-constexpr-if-else-clause-never-happen
	// https://en.cppreference.com/w/cpp/utility/variant/visit
	// https://en.cppreference.com/w/cpp/language/if#Constexpr_If
	template<class...> inline constexpr bool always_false_v = false;


	template <typename Tup, typename Fun, std::size_t... I>
	inline void for_each_tuple_impl(Tup&& t, Fun&& f, std::index_sequence<I...>)
	{
		(f(std::get<I>(std::forward<Tup>(t))), ...);
	}

	template <typename Tup, typename Fun>
	inline void for_each_tuple(Tup&& t, Fun&& f)
	{
		for_each_tuple_impl(std::forward<Tup>(t), std::forward<Fun>(f), std::make_index_sequence<
			std::tuple_size_v<detail::remove_cvref_t<Tup>>>{});
	}


	// example : static_assert(is_template_instantiable_v<std::vector, double>);
	//           static_assert(is_template_instantiable_v<std::optional, int, int>);
	template<template<typename...> typename T, typename AlwaysVoid, typename... Args>
	struct is_template_instantiable : std::false_type {};

	template<template<typename...> typename T, typename... Args>
	struct is_template_instantiable<T, std::void_t<T<Args...>>, Args...> : std::true_type {};

	template<template<typename...> typename T, typename... Args>
	inline constexpr bool is_template_instantiable_v = is_template_instantiable<T, void, Args...>::value;


	// example : static_assert(is_template_instance_of<std::vector, std::vector<int>>);
	template<template<typename...> class U, typename T>
	struct is_template_instance_of : std::false_type {};

	template<template<typename...> class U, typename...Args>
	struct is_template_instance_of<U, U<Args...>> : std::true_type {};

	template<template<typename...> class U, typename...Args>
	inline constexpr bool is_template_instance_of_v = is_template_instance_of<U, Args...>::value;

	template<typename T> struct is_tuple : is_template_instance_of<std::tuple, T> {};


	template<typename, typename = void>
	struct is_char : std::false_type {};

	template<typename T>
	struct is_char<T, std::void_t<typename std::enable_if_t<
			std::is_same_v<detail::remove_cvref_t<T>, char    > ||
			std::is_same_v<detail::remove_cvref_t<T>, wchar_t > ||
		#if defined(__cpp_lib_char8_t)
			std::is_same_v<detail::remove_cvref_t<T>, char8_t > ||
		#endif
			std::is_same_v<detail::remove_cvref_t<T>, char16_t> ||
			std::is_same_v<detail::remove_cvref_t<T>, char32_t>
		>>> : std::true_type {};

	template<class T>
	inline constexpr bool is_char_v = is_char<detail::remove_cvref_t<T>>::value;


	template<typename, typename = void>
	struct is_string : std::false_type {};

	template<typename T>
	struct is_string<T, std::void_t<typename T::value_type, typename T::traits_type, typename T::allocator_type,
		typename std::enable_if_t<std::is_same_v<T,
		std::basic_string<typename T::value_type, typename T::traits_type, typename T::allocator_type>>>>>
		: std::true_type {};

	template<class T>
	inline constexpr bool is_string_v = is_string<detail::remove_cvref_t<T>>::value;


	template<typename, typename = void>
	struct is_string_view : std::false_type {};

	template<typename T>
	struct is_string_view<T, std::void_t<typename T::value_type, typename T::traits_type,
		typename std::enable_if_t<std::is_same_v<T,
		std::basic_string_view<typename T::value_type, typename T::traits_type>>>>> : std::true_type {};

	template<class T>
	inline constexpr bool is_string_view_v = is_string_view<detail::remove_cvref_t<T>>::value;


	template<typename, typename = void>
	struct is_char_pointer : std::false_type {};

	// char const * 
	// detail::remove_cvref_t<std::remove_pointer_t<detail::remove_cvref_t<T>>>
	// char
	template<typename T>
	struct is_char_pointer<T, std::void_t<typename std::enable_if_t<
		 std::is_pointer_v<                                             detail::remove_cvref_t<T>>  &&
		!std::is_pointer_v<detail::remove_cvref_t<std::remove_pointer_t<detail::remove_cvref_t<T>>>> &&
		 detail::is_char_v<std::remove_pointer_t<detail::remove_cvref_t<T>>>
		>>> : std::true_type {};

	template<class T>
	inline constexpr bool is_char_pointer_v = is_char_pointer<detail::remove_cvref_t<T>>::value;


	template<typename, typename = void>
	struct is_char_array : std::false_type {};

	template<typename T>
	struct is_char_array<T, std::void_t<typename std::enable_if_t <
		std::is_array_v<detail::remove_cvref_t<T>>  &&
		detail::is_char_v<std::remove_all_extents_t<detail::remove_cvref_t<T>>>
		>>> : std::true_type {};

	template<class T>
	inline constexpr bool is_char_array_v = is_char_array<detail::remove_cvref_t<T>>::value;


	template<class T, bool F = detail::is_char_v<T> || detail::is_char_pointer_v<T> || detail::is_char_array_v<T>>
	struct char_type;

	template<class T>
	struct char_type<T, true>
	{
		using type = detail::remove_cvref_t<std::remove_pointer_t<std::remove_all_extents_t<detail::remove_cvref_t<T>>>>;
	};

	template<class T>
	struct char_type<T, false>
	{
		using type = typename T::value_type;
	};


	template<class T>
	inline constexpr bool is_character_string_v =
		detail::is_string_v      <T> ||
		detail::is_string_view_v <T> ||
		detail::is_char_pointer_v<T> ||
		detail::is_char_array_v  <T>;


	template<class, class, class = void>
	struct has_stream_operator : std::false_type {};

	template<class T, class D>
	struct has_stream_operator<T, D, std::void_t<decltype(T{} << D{})>> : std::true_type{};

	template<class, class, class = void>
	struct has_equal_operator : std::false_type {};

	template<class T, class D>
	struct has_equal_operator<T, D, std::void_t<decltype(T{} = D{})>> : std::true_type{};

	template<class, class = void>
	struct has_bool_operator : std::false_type {};

	template<class T>
	struct has_bool_operator<T, std::void_t<decltype(std::declval<T&>().operator bool())>> : std::true_type{};


	template<class, class = void>
	struct can_convert_to_string : std::false_type {};

	template<class T>
	struct can_convert_to_string<T, std::void_t<decltype(
		std::string(std::declval<T>()).size(),
		std::declval<std::string>() = std::declval<T>()
		)>> : std::true_type{};

	template<class T>
	inline constexpr bool can_convert_to_string_v = can_convert_to_string<detail::remove_cvref_t<T>>::value;


	template<class T>
	struct shared_ptr_adapter
	{
		using rawt = typename detail::remove_cvref_t<T>;
		using type = std::conditional_t<detail::is_template_instance_of_v<std::shared_ptr, rawt>,
			rawt, std::shared_ptr<rawt>>;
	};

	template<class T>
	typename detail::shared_ptr_adapter<T>::type to_shared_ptr(T&& t)
	{
		using rawt = typename detail::remove_cvref_t<T>;

		if constexpr (detail::is_template_instance_of_v<std::shared_ptr, rawt>)
		{
			return std::forward<T>(t);
		}
		else
		{
			return std::make_shared<rawt>(std::forward<T>(t));
		}
	}

	//// The following code will cause element_type_adapter<int> compilation failure:
	//// the "int" don't has a type of element_type.
	//template<class T>
	//struct element_type_adapter
	//{
	//	using rawt = typename remove_cvref_t<T>;
	//	using type = std::conditional_t<is_template_instance_of_v<std::shared_ptr, rawt>,
	//		typename rawt::element_type, rawt>;
	//};

	template<class T>
	struct element_type_adapter
	{
		using type = typename detail::remove_cvref_t<T>;
	};

	template<class T>
	struct element_type_adapter<std::shared_ptr<T>>
	{
		using type = typename detail::remove_cvref_t<T>;
	};

	template<class T>
	struct element_type_adapter<std::unique_ptr<T>>
	{
		using type = typename detail::remove_cvref_t<T>;
	};

	template<class T>
	struct element_type_adapter<T*>
	{
		using type = typename detail::remove_cvref_t<T>;
	};

	template<class T>
	inline typename element_type_adapter<T>::type& to_element_ref(T& value) noexcept
	{
		using rawt = typename detail::remove_cvref_t<T>;

		if /**/ constexpr (is_template_instance_of_v<std::shared_ptr, rawt>)
			return *value;
		else if constexpr (is_template_instance_of_v<std::unique_ptr, rawt>)
			return *value;
		else if constexpr (std::is_pointer_v<rawt>)
			return *value;
		else
			return value;
	}
}

namespace asio2
{
	using detail::to_underlying;
	using detail::ignore_unused;
	using detail::to_shared_ptr;
}

#endif // !__ASIO2_TYPE_TRAITS_HPP__
