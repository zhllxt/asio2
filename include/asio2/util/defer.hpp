/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * refrenced from https://github.com/loveyacper/ananas
 */

#ifndef __ASIO2_DEFER_HPP__
#define __ASIO2_DEFER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <functional>

namespace asio2
{
	class defer
	{
	protected:
		template<typename F, typename Void, typename... Args>
		struct is_callable : std::false_type {};

		template<typename F, typename... Args>
		struct is_callable<F, std::void_t<decltype(std::declval<std::decay_t<F>&>()
			((std::declval<Args>())...)), char>, Args...> : std::true_type {};

	public:
		defer() noexcept = default;

		// movable
		defer(defer&&) noexcept = default;
		defer& operator=(defer&&) noexcept = default;

		// non copyable
		defer(const defer&) = delete;
		void operator=(const defer&) = delete;

		template <typename Fun, typename... Args,
			std::enable_if_t<is_callable<Fun, void, Args...>::value, int> = 0>
		defer(Fun&& fun, Args&&... args)
		{
			this->fn_ = std::bind(std::forward<Fun>(fun), std::forward<Args>(args)...);
		}

		template <typename Constructor, typename Destructor,
			std::enable_if_t<is_callable<Constructor, void>::value && is_callable<Destructor, void>::value, int> = 0>
		defer(Constructor&& ctor, Destructor&& dtor)
		{
			(ctor)();

			this->fn_ = std::forward<Destructor>(dtor);
		}

		~defer() noexcept
		{
			if (this->fn_) this->fn_();
		}

	protected:
		std::function<void()> fn_;
	};

#ifndef ASIO2_CONCAT
#define ASIO2_CONCAT(a, b) a##b
#endif
#define ASIO2_MAKE_DEFER(line) ::asio2::defer ASIO2_CONCAT(_asio2_defer_, line) = [&]()
#define ASIO2_DEFER ASIO2_MAKE_DEFER(__LINE__)

}

#endif  // !__ASIO2_DEFER_HPP__
