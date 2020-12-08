/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 *
 * C++0x : Storing any type of std::function in a std::map
 * https://stackoverflow.com/questions/7624017/c0x-storing-any-type-of-stdfunction-in-a-stdmap
 */

#ifndef __ASIO2_LISTENER_HPP__
#define __ASIO2_LISTENER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <functional>
#include <array>
#include <tuple>
#include <type_traits>

#include <asio2/base/error.hpp>
#include <asio2/base/detail/util.hpp>

namespace asio2::detail
{
	enum class event_type : std::int8_t
	{
		recv,
		connect,
		disconnect,
		accept,
		handshake,
		upgrade,
		init,
		start,
		stop,
		max
	};

	class base_observer
	{
	public:
		virtual ~base_observer() {}
	};

	template<class... Args>
	class observer_t : public base_observer
	{
	public:
		using func_type = std::function<void(Args...)>;
		using args_type = std::tuple<Args...>;

		explicit observer_t(const func_type & fn) : fn_(fn) {}
		explicit observer_t(func_type && fn) : fn_(std::move(fn)) {}
		explicit observer_t(const observer_t<Args...> & other) : fn_(other.fn_) {}
		explicit observer_t(observer_t<Args...> && other) : fn_(std::move(other.fn_)) {}

		template<class F, class ...C>
		explicit observer_t(F&& f, C&&... c)
		{
			static_assert(sizeof...(C) == 0 || sizeof...(C) == 1,
				"the class object parameters of C&&... c can only be none or one");
			this->bind(std::forward<F>(f), std::forward<C>(c)...);
		}

		template<class F>
		inline void bind(F&& f)
		{
			this->fn_ = func_type(std::forward<F>(f));
		}

		template<class F, class C>
		inline void bind(F&& f, C&& c)
		{
			if constexpr /**/ (std::is_pointer_v<C>)
			{
				this->fn_ = [fn = std::forward<F>(f), s = std::forward<C>(c)](Args&&... args) mutable
				{
					(s->*fn)(std::forward<Args>(args)...);
				};
			}
			else if constexpr (std::is_reference_v<C>)
			{
				this->fn_ = [fn = std::forward<F>(f), s = std::forward<C>(c)](Args&&... args) mutable
				{
					(s.*fn)(std::forward<Args>(args)...);
				};
			}
			else
			{
				ASIO2_ASSERT(false);
				//static_assert(false,
				//	"the class object parameters of C&& c must be pointer or refrence");
			}
		}

		inline void operator()(Args&&... args)
		{
			if (this->fn_)
				this->fn_(std::forward<Args>(args)...);
		}

		inline func_type move() { return std::move(this->fn_); }

	protected:
		func_type fn_;
	};

	class listener_t
	{
	public:
		listener_t() {}
		~listener_t() = default;

		template<class T>
		inline void bind(event_type e, T&& observer)
		{
			this->observers_[enum_to_int(e)] = std::unique_ptr<base_observer>(new T(std::forward<T>(observer)));
		}

		template<class... Args>
		inline void notify(event_type e, Args&&... args)
		{
			using observer_type = observer_t<Args...>;
			observer_type * observer_ptr = static_cast<observer_type *>(this->observers_[enum_to_int(e)].get());
			if (observer_ptr)
			{
				(*observer_ptr)(std::forward<Args>(args)...);
			}
		}

		inline std::unique_ptr<base_observer>& find(event_type e)
		{
			return this->observers_[enum_to_int(e)];
		}

	protected:
		std::array<std::unique_ptr<base_observer>, enum_to_int(event_type::max)> observers_;
	};
}

#endif // !__ASIO2_LISTENER_HPP__
