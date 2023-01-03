/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * refrenced from : https://github.com/qicosmos/rest_rpc/blob/master/include/meta_util.hpp
 */

#ifndef __ASIO2_FUNCTION_TRAITS_HPP__
#define __ASIO2_FUNCTION_TRAITS_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <functional>
#include <type_traits>

namespace asio2::detail
{
	template<typename, typename = void>
	struct lambda_dummy_t { };

	template<typename Ret, typename... Args>
	struct lambda_dummy_t<Ret(Args...)>
	{
		template<typename R = Ret>
		inline R operator()(const Args&...)
		{
			if /**/ constexpr (std::is_same_v<void, std::remove_cv_t<std::remove_reference_t<R>>>)
			{
				return;
			}
			else if constexpr (std::is_reference_v<std::remove_cv_t<R>>)
			{
				static typename std::remove_reference_t<R> s;
				return s;
			}
			else
			{
				return R{};
			}
		}
	};

	/*
	 * 1. function type								==>	Ret(Args...)
	 * 2. function pointer							==>	Ret(*)(Args...)
	 * 3. function reference						==>	Ret(&)(Args...)
	 * 4. pointer to non-static member function		==> Ret(T::*)(Args...)
	 * 5. function object and functor				==> &T::operator()
	 * 6. function with generic operator call		==> template <typeanme ... Args> &T::operator()
	 */
	template<typename, typename = void>
	struct function_traits { static constexpr bool is_callable = false; };

	template<typename Ret, typename... Args>
	struct function_traits<Ret(Args...)>
	{
	public:
		static constexpr std::size_t argc = sizeof...(Args);
		static constexpr bool is_callable = true;

		typedef Ret function_type(Args...);
		typedef Ret return_type;
		using stl_function_type = std::function<function_type>;
		using stl_lambda_type = lambda_dummy_t<function_type>;
		typedef Ret(*pointer)(Args...);
		using class_type = void;

		template<std::size_t I>
		struct args
		{
			static_assert(I < argc, "index is out of range, index must less than sizeof Args");
			using type = typename std::tuple_element<I, std::tuple<Args...>>::type;
		};

		typedef std::tuple<Args...> tuple_type;
		typedef std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> pod_tuple_type;
	};

	template<typename Class, typename Ret, typename... Args>
	struct function_traits<Class, Ret(Args...)>
	{
	public:
		static constexpr std::size_t argc = sizeof...(Args);
		static constexpr bool is_callable = true;

		typedef Ret function_type(Args...);
		typedef Ret return_type;
		using stl_function_type = std::function<function_type>;
		using stl_lambda_type = lambda_dummy_t<function_type>;
		typedef Ret(*pointer)(Args...);
		using class_type = Class;

		template<std::size_t I>
		struct args
		{
			static_assert(I < argc, "index is out of range, index must less than sizeof Args");
			using type = typename std::tuple_element<I, std::tuple<Args...>>::type;
		};

		typedef std::tuple<Args...> tuple_type;
		typedef std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> pod_tuple_type;
	};

	// regular function pointer
	template<typename Ret, typename... Args>
	struct function_traits<Ret(*)(Args...)> : function_traits<Ret(Args...)> {};

	// non-static member function pointer
	template <typename Ret, typename Class, typename... Args>
	struct function_traits<Ret(Class::*)(Args...)> : function_traits<Class, Ret(Args...)> {};

	template <typename Ret, typename Class, typename... Args>
	struct function_traits<Ret(Class::*)(Args...) const> : function_traits<Class, Ret(Args...)> {};

	template <typename Ret, typename Class, typename... Args>
	struct function_traits<Ret(Class::*)(Args...) volatile> : function_traits<Class, Ret(Args...)> {};

	template <typename Ret, typename Class, typename... Args>
	struct function_traits<Ret(Class::*)(Args...) const volatile> : function_traits<Class, Ret(Args...)> {};

	template <typename Ret, typename Class, typename... Args>
	struct function_traits<Ret(Class::*)(Args...) &> : function_traits<Class, Ret(Args...)> {};

	template <typename Ret, typename Class, typename... Args>
	struct function_traits<Ret(Class::*)(Args...) const &> : function_traits<Class, Ret(Args...)> {};

	template <typename Ret, typename Class, typename... Args>
	struct function_traits<Ret(Class::*)(Args...) volatile &> : function_traits<Class, Ret(Args...)> {};

	template <typename Ret, typename Class, typename... Args>
	struct function_traits<Ret(Class::*)(Args...) const volatile &> : function_traits<Class, Ret(Args...)> {};

	template <typename Ret, typename Class, typename... Args>
	struct function_traits<Ret(Class::*)(Args...) && > : function_traits<Class, Ret(Args...)> {};

	template <typename Ret, typename Class, typename... Args>
	struct function_traits<Ret(Class::*)(Args...) const &&> : function_traits<Class, Ret(Args...)> {};

	template <typename Ret, typename Class, typename... Args>
	struct function_traits<Ret(Class::*)(Args...) volatile &&> : function_traits<Class, Ret(Args...)> {};

	template <typename Ret, typename Class, typename... Args>
	struct function_traits<Ret(Class::*)(Args...) const volatile &&> : function_traits<Class, Ret(Args...)> {};

	// non-static member function pointer -- noexcept versions for (C++17 and later)
	template <typename Ret, typename Class, typename... Args>
	struct function_traits<Ret(Class::*)(Args...) noexcept> : function_traits<Class, Ret(Args...)> {};

	template <typename Ret, typename Class, typename... Args>
	struct function_traits<Ret(Class::*)(Args...) const noexcept> : function_traits<Class, Ret(Args...)> {};

	template <typename Ret, typename Class, typename... Args>
	struct function_traits<Ret(Class::*)(Args...) volatile noexcept> : function_traits<Class, Ret(Args...)> {};

	template <typename Ret, typename Class, typename... Args>
	struct function_traits<Ret(Class::*)(Args...) const volatile noexcept> : function_traits<Class, Ret(Args...)> {};

	template <typename Ret, typename Class, typename... Args>
	struct function_traits<Ret(Class::*)(Args...) & noexcept> : function_traits<Class, Ret(Args...)> {};

	template <typename Ret, typename Class, typename... Args>
	struct function_traits<Ret(Class::*)(Args...) const & noexcept> : function_traits<Class, Ret(Args...)> {};

	template <typename Ret, typename Class, typename... Args>
	struct function_traits<Ret(Class::*)(Args...) volatile & noexcept> : function_traits<Class, Ret(Args...)> {};

	template <typename Ret, typename Class, typename... Args>
	struct function_traits<Ret(Class::*)(Args...) const volatile& noexcept>
		: function_traits<Class, Ret(Args...)> {};

	template <typename Ret, typename Class, typename... Args>
	struct function_traits<Ret(Class::*)(Args...) && noexcept> : function_traits<Class, Ret(Args...)> {};

	template <typename Ret, typename Class, typename... Args>
	struct function_traits<Ret(Class::*)(Args...) const && noexcept> : function_traits<Class, Ret(Args...)> {};

	template <typename Ret, typename Class, typename... Args>
	struct function_traits<Ret(Class::*)(Args...) volatile && noexcept> : function_traits<Class, Ret(Args...)> {};

	template <typename Ret, typename Class, typename... Args>
	struct function_traits<Ret(Class::*)(Args...) const volatile&& noexcept>
		: function_traits<Class, Ret(Args...)> {};

	// functor lambda
	template<typename Callable>
	struct function_traits<Callable, std::void_t<decltype(&Callable::operator()), char>>
		: function_traits<decltype(&Callable::operator())> {};

	// std::function
	template <typename Ret, typename... Args>
	struct function_traits<std::function<Ret(Args...)>> : function_traits<Ret(Args...)> {};


	template <typename F>
	typename function_traits<F>::stl_function_type to_function(F&& lambda)
	{
		return static_cast<typename function_traits<F>::stl_function_type>(std::forward<F>(lambda));
	}

	template <typename F>
	typename function_traits<F>::pointer to_function_pointer(const F& lambda) noexcept
	{
		return static_cast<typename function_traits<F>::pointer>(lambda);
	}


	template< class F >
	inline constexpr bool is_callable_v = function_traits<std::decay_t<F>>::is_callable;


	template<typename F, typename Void, typename... Args>
	struct is_template_callable : std::false_type {};

	template<typename F, typename... Args>
	struct is_template_callable<F, std::void_t<decltype(std::declval<std::decay_t<F>&>()
		((std::declval<Args>())...)), char>, Args...> : std::true_type {};

	template<typename F, typename... Args>
	inline constexpr bool is_template_callable_v = is_template_callable<F, void, Args...>::value;
}

#endif // !__ASIO2_FUNCTION_TRAITS_HPP__
