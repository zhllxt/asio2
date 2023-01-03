/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
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
		send,
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

	class observer_base
	{
	public:
		virtual ~observer_base() noexcept {}
	};

	template<class... Args>
	class observer_t : public observer_base
	{
	public:
		using func_type = std::function<void(Args...)>;
		using args_type = std::tuple<Args...>;

		explicit observer_t(const func_type &  fn) : fn_(fn) {}
		explicit observer_t(      func_type && fn) : fn_(std::move(fn)) {}
		explicit observer_t(const observer_t<Args...> &  other) : fn_(other.fn_) {}
		explicit observer_t(      observer_t<Args...> && other) : fn_(std::move(other.fn_)) {}

		template<class F, class ...C>
		explicit observer_t(F&& f, C&&... c)
		{
			this->bind(std::forward<F>(f), std::forward<C>(c)...);
		}

		template<class F, class ...C>
		inline void bind(F&& f, C&&... c)
		{
			if constexpr (sizeof...(C) == std::size_t(0))
			{
				this->fn_ = func_type(std::forward<F>(f));
			}
			else
			{
				if constexpr (std::is_member_function_pointer_v<detail::remove_cvref_t<F>>)
				{
					if constexpr (sizeof...(C) == std::size_t(1))
					{
						this->bind_memfn(std::forward<F>(f), std::forward<C>(c)...);
					}
					else
					{
						this->bind_memfn_front(std::forward<F>(f), std::forward<C>(c)...);
					}
				}
				else
				{
					this->bind_fn_front(std::forward<F>(f), std::forward<C>(c)...);
				}
			}
		}

		template<class F, class C>
		inline void bind_memfn(F&& f, C&& c)
		{
			if constexpr /**/ (std::is_pointer_v<detail::remove_cvref_t<C>>)
			{
				this->fn_ = [fn = std::forward<F>(f), s = std::forward<C>(c)](Args&&... args) mutable
				{
					(s->*fn)(std::forward<Args>(args)...);
				};
			}
			else if constexpr (std::is_reference_v<std::remove_cv_t<C>>)
			{
				this->fn_ = [fn = std::forward<F>(f), s = std::forward<C>(c)](Args&&... args) mutable
				{
					(s.*fn)(std::forward<Args>(args)...);
				};
			}
			else
			{
				static_assert(detail::always_false_v<F>,
					"the class object parameters of C&& c must be pointer or refrence");
			}
		}

		template<class F, class C, class... Ts>
		inline void bind_memfn_front(F&& f, C&& c, Ts&&... ts)
		{
			if constexpr /**/ (std::is_pointer_v<detail::remove_cvref_t<C>>)
			{
				this->fn_ = [fn = std::forward<F>(f), s = std::forward<C>(c), tp = std::tuple(std::forward<Ts>(ts)...)]
				(Args&&... args) mutable
				{
					invoke_memfn_front(fn, s, std::make_index_sequence<sizeof...(Ts)>{}, tp, std::forward<Args>(args)...);
				};
			}
			else if constexpr (std::is_reference_v<std::remove_cv_t<C>>)
			{
				this->fn_ = [fn = std::forward<F>(f), s = std::forward<C>(c), tp = std::tuple(std::forward<Ts>(ts)...)]
				(Args&&... args) mutable
				{
					invoke_memfn_front(fn, std::addressof(s), std::make_index_sequence<sizeof...(Ts)>{}, tp, std::forward<Args>(args)...);
				};
			}
			else
			{
				static_assert(detail::always_false_v<F>,
					"the class object parameters of C&& c must be pointer or refrence");
			}
		}

		template<typename F, typename C, std::size_t... I, typename... Ts>
		inline static void invoke_memfn_front(F& f, C* c, std::index_sequence<I...>, std::tuple<Ts...>& tp, Args&&... args)
		{
			(c->*f)(std::get<I>(tp)..., std::forward<Args>(args)...);
		}

		template<class F, class... Ts>
		inline void bind_fn_front(F&& f, Ts&&... ts)
		{
			this->fn_ = [fn = std::forward<F>(f), tp = std::tuple(std::forward<Ts>(ts)...)]
			(Args&&... args) mutable
			{
				invoke_fn_front(fn, std::make_index_sequence<sizeof...(Ts)>{}, tp, std::forward<Args>(args)...);
			};
		}

		template<typename F, std::size_t... I, typename... Ts>
		inline static void invoke_fn_front(F& f, std::index_sequence<I...>, std::tuple<Ts...>& tp, Args&&... args)
		{
			f(std::get<I>(tp)..., std::forward<Args>(args)...);
		}

		inline void operator()(Args&&... args)
		{
			if (this->fn_)
				this->fn_(std::forward<Args>(args)...);
		}

		inline func_type move() noexcept { return std::move(this->fn_); }

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
			this->observers_[detail::to_underlying(e)] =
				std::unique_ptr<observer_base>(new T(std::forward<T>(observer)));
		}

		template<class... Args>
		inline void notify(event_type e, Args&&... args)
		{
			using observer_type = observer_t<Args...>;

			observer_type* observer_ptr = static_cast<observer_type*>(
				this->observers_[detail::to_underlying(e)].get());
			if (observer_ptr)
			{
				(*observer_ptr)(std::forward<Args>(args)...);
			}
		}

		inline std::unique_ptr<observer_base>& find(event_type e) noexcept
		{
			return this->observers_[detail::to_underlying(e)];
		}

	protected:
		std::array<std::unique_ptr<observer_base>, detail::to_underlying(event_type::max)> observers_;
	};
}

#endif // !__ASIO2_LISTENER_HPP__
