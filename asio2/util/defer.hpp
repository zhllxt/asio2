/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
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
	public:
		defer() = default;

		// movable
		defer(defer&&) = default;
		defer& operator=(defer&&) = default;

		// non copyable
		defer(const defer&) = delete;
		void operator=(const defer&) = delete;

		template <typename Fun, typename... Args>
		defer(Fun&& fun, Args&&... args)
		{
			this->fn_ = std::bind(std::forward<Fun>(fun), std::forward<Args>(args)...);
		}

		~defer() noexcept
		{
			if (this->fn_) this->fn_();
		}

	protected:
		std::function<void()> fn_;
	};

#define ASIO2_CONCAT(a, b) a##b
#define ASIO2_MAKE_DEFER(line) ::asio2::defer ASIO2_CONCAT(_asio2_defer_, line) = [&]()
#define ASIO2_DEFER ASIO2_MAKE_DEFER(__LINE__)

}

#endif  // !__ASIO2_DEFER_HPP__
