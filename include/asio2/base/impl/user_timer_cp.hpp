/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_USER_TIMER_COMPONENT_HPP__
#define __ASIO2_USER_TIMER_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <chrono>
#include <functional>
#include <unordered_map>
#include <string>
#include <string_view>
#include <future>

#include <asio2/base/iopool.hpp>
#include <asio2/base/log.hpp>

#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/allocator.hpp>
#include <asio2/base/detail/function_traits.hpp>

namespace asio2::detail
{
	struct user_timer_handle
	{
		template<typename T, typename = std::void_t<typename std::enable_if_t<!std::is_same_v<
			std::remove_cv_t<std::remove_reference_t<T>>, user_timer_handle>, void>>>
			user_timer_handle(T&& id)
		{
			bind(std::forward<T>(id));
		}

		user_timer_handle(user_timer_handle&& other) = default;
		user_timer_handle(user_timer_handle const& other) = default;

		user_timer_handle& operator=(user_timer_handle&& other) = default;
		user_timer_handle& operator=(user_timer_handle const& other) = default;

		template<typename T>
		inline typename std::enable_if_t<!std::is_same_v<std::remove_cv_t<
			std::remove_reference_t<T>>, user_timer_handle>, void>
			operator=(T&& id)
		{
			bind(std::forward<T>(id));
		}

		inline bool operator==(const user_timer_handle& r) const noexcept
		{
			return (r.handle == this->handle);
		}

		template<typename T>
		inline void bind(T&& id)
		{
			using type = std::remove_cv_t<std::remove_reference_t<T>>;
			using rtype = std::remove_pointer_t<std::remove_all_extents_t<type>>;

			if /**/ constexpr (std::is_same_v<std::string, type>)
			{
				handle = std::forward<T>(id);
			}
			else if constexpr (detail::is_string_view_v<type>)
			{
				handle.resize(sizeof(typename type::value_type) * id.size());
				std::memcpy((void*)handle.data(), (const void*)id.data(),
					sizeof(typename type::value_type) * id.size());
			}
			else if constexpr (detail::is_string_v<type>)
			{
				handle.resize(sizeof(typename type::value_type) * id.size());
				std::memcpy((void*)handle.data(), (const void*)id.data(),
					sizeof(typename type::value_type) * id.size());
			}
			else if constexpr (std::is_integral_v<type>)
			{
				handle = std::to_string(id);
			}
			else if constexpr (std::is_floating_point_v<type>)
			{
				handle.resize(sizeof(type));
				std::memcpy((void*)handle.data(), (const void*)&id, sizeof(type));
			}
			else if constexpr (detail::is_char_pointer_v<type>)
			{
				ASIO2_ASSERT(id && (*id));
				if (id)
				{
					std::basic_string_view<rtype> sv{ id };
					handle.resize(sizeof(rtype) * sv.size());
					std::memcpy((void*)handle.data(), (const void*)sv.data(), sizeof(rtype) * sv.size());
				}
			}
			else if constexpr (detail::is_char_array_v<type>)
			{
				std::basic_string_view<rtype> sv{ id };
				handle.resize(sizeof(rtype) * sv.size());
				std::memcpy((void*)handle.data(), (const void*)sv.data(), sizeof(rtype) * sv.size());
			}
			else if constexpr (std::is_pointer_v<type>)
			{
				handle = std::to_string(std::size_t(id));
			}
			else if constexpr (std::is_array_v<type>)
			{
				static_assert(detail::always_false_v<T>);
			}
			else if constexpr (std::is_same_v<user_timer_handle, type>)
			{
				static_assert(detail::always_false_v<T>);
			}
			else
			{
				static_assert(detail::always_false_v<T>);
			}
		}

		std::string handle;
	};

	struct user_timer_handle_hash
	{
		inline std::size_t operator()(const user_timer_handle& k) const noexcept
		{
			return std::hash<std::string>{}(k.handle);
		}
	};

	struct user_timer_handle_equal
	{
		inline bool operator()(const user_timer_handle& lhs, const user_timer_handle& rhs) const noexcept
		{
			return lhs.handle == rhs.handle;
		}
	};

	struct user_timer_obj
	{
		user_timer_handle                        id;
		asio::steady_timer                       timer;
		std::function<void()>                    callback{};
		typename asio::steady_timer::duration    interval{};
		mutable std::size_t                      repeat = static_cast<std::size_t>(-1);
		mutable bool                             exited = false;

		user_timer_obj(user_timer_handle Id, asio::io_context& context)
			: id(std::move(Id)), timer(context)
		{
		}
	};

	template<class derived_t, class args_t = void>
	class user_timer_cp
	{
	public:
		using user_timer_map = std::unordered_map<user_timer_handle, std::shared_ptr<user_timer_obj>,
			user_timer_handle_hash, user_timer_handle_equal>;

		/**
		 * @brief constructor
		 */
		user_timer_cp() {}

		/**
		 * @brief destructor
		 */
		~user_timer_cp() noexcept {}

	public:
		/**
		 * @brief start a timer
		 * @param timer_id - The timer id can be integer or string, example : 1,2,3 or "id1" "id2",
		 *  If a timer with this id already exists, that timer will be forced to canceled.
		 * @param interval - An integer millisecond value for the amount of time between set and firing.
		 * @param fun - The callback function signature : [](){}
		 */
		template<class TimerId, class IntegerMilliseconds, class Fun, class... Args>
		inline typename std::enable_if_t<is_callable_v<Fun>
			&& std::is_integral_v<detail::remove_cvref_t<IntegerMilliseconds>>, void>
		start_timer(TimerId&& timer_id, IntegerMilliseconds interval, Fun&& fun, Args&&... args)
		{
			this->start_timer(std::forward<TimerId>(timer_id), std::chrono::milliseconds(interval), std::size_t(-1),
				std::chrono::milliseconds(interval), std::forward<Fun>(fun), std::forward<Args>(args)...);
		}

		/**
		 * @brief start a timer
		 * @param timer_id - The timer id can be integer or string, example : 1,2,3 or "id1" "id2",
		 *  If a timer with this id already exists, that timer will be forced to canceled.
		 * @param interval - An integer millisecond value for the amount of time between set and firing.
		 * @param repeat - An integer value to indicate the number of times the timer is repeated.
		 * @param fun - The callback function signature : [](){}
		 */
		template<class TimerId, class IntegerMilliseconds, class Integer, class Fun, class... Args>
		inline typename std::enable_if_t<is_callable_v<Fun>
			&& std::is_integral_v<detail::remove_cvref_t<IntegerMilliseconds>>
			&& std::is_integral_v<detail::remove_cvref_t<Integer>>, void>
		start_timer(TimerId&& timer_id, IntegerMilliseconds interval, Integer repeat, Fun&& fun, Args&&... args)
		{
			this->start_timer(std::forward<TimerId>(timer_id), std::chrono::milliseconds(interval), repeat,
				std::chrono::milliseconds(interval), std::forward<Fun>(fun), std::forward<Args>(args)...);
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @brief start a timer
		 * @param timer_id - The timer id can be integer or string, example : 1,2,3 or "id1" "id2",
		 *  If a timer with this id already exists, that timer will be forced to canceled.
		 * @param interval - The amount of time between set and firing.
		 * @param fun - The callback function signature : [](){}
		 */
		template<class TimerId, class Rep, class Period, class Fun, class... Args>
		inline typename std::enable_if_t<is_callable_v<Fun>, void>
		start_timer(TimerId&& timer_id, std::chrono::duration<Rep, Period> interval, Fun&& fun, Args&&... args)
		{
			this->start_timer(std::forward<TimerId>(timer_id), interval, std::size_t(-1), interval,
				std::forward<Fun>(fun), std::forward<Args>(args)...);
		}

		/**
		 * @brief start a timer
		 * @param timer_id - The timer id can be integer or string, example : 1,2,3 or "id1" "id2",
		 *  If a timer with this id already exists, that timer will be forced to canceled.
		 * @param interval - The amount of time between set and firing.
		 * @param repeat - An integer value to indicate the number of times the timer is repeated.
		 * @param fun - The callback function signature : [](){}
		 */
		template<class TimerId, class Rep, class Period, class Integer, class Fun, class... Args>
		inline typename std::enable_if_t<
			is_callable_v<Fun> && std::is_integral_v<detail::remove_cvref_t<Integer>>, void>
		start_timer(TimerId&& timer_id, std::chrono::duration<Rep, Period> interval, Integer repeat,
			Fun&& fun, Args&&... args)
		{
			this->start_timer(std::forward<TimerId>(timer_id), interval, repeat, interval,
				std::forward<Fun>(fun), std::forward<Args>(args)...);
		}

		/**
		 * @brief start a timer
		 * @param timer_id - The timer id can be integer or string, example : 1,2,3 or "id1" "id2",
		 *  If a timer with this id already exists, that timer will be forced to canceled.
		 * @param interval - The amount of time between set and firing.
		 * @param first_delay - The timeout for the first execute of timer.
		 * @param fun - The callback function signature : [](){}
		 */
		template<class TimerId, class Rep1, class Period1, class Rep2, class Period2, class Fun, class... Args>
		inline typename std::enable_if_t<is_callable_v<Fun>, void>
		start_timer(TimerId&& timer_id, std::chrono::duration<Rep1, Period1> interval,
			std::chrono::duration<Rep2, Period2> first_delay, Fun&& fun, Args&&... args)
		{
			this->start_timer(std::forward<TimerId>(timer_id), interval, std::size_t(-1), first_delay,
				std::forward<Fun>(fun), std::forward<Args>(args)...);
		}

		/**
		 * @brief start a timer
		 * @param timer_id - The timer id can be integer or string, example : 1,2,3 or "id1" "id2",
		 *  If a timer with this id already exists, that timer will be forced to canceled.
		 * @param interval - The amount of time between set and firing.
		 * @param repeat - Total number of times the timer is executed.
		 * @param first_delay - The timeout for the first execute of timer. 
		 * @param fun - The callback function signature : [](){}
		 */
		template<class TimerId, class Rep1, class Period1, class Rep2, class Period2, class Integer, class Fun, class... Args>
		inline typename std::enable_if_t<
			is_callable_v<Fun> && std::is_integral_v<detail::remove_cvref_t<Integer>>, void>
		start_timer(TimerId&& timer_id, std::chrono::duration<Rep1, Period1> interval, Integer repeat,
			std::chrono::duration<Rep2, Period2> first_delay, Fun&& fun, Args&&... args)
		{
			if (repeat == static_cast<std::size_t>(0))
			{
				ASIO2_ASSERT(false);
				return;
			}

			if (interval > std::chrono::duration_cast<
				std::chrono::duration<Rep1, Period1>>((asio::steady_timer::duration::max)()))
				interval = std::chrono::duration_cast<std::chrono::duration<Rep1, Period1>>(
					(asio::steady_timer::duration::max)());

			if (first_delay > std::chrono::duration_cast<
				std::chrono::duration<Rep2, Period2>>((asio::steady_timer::duration::max)()))
				first_delay = std::chrono::duration_cast<std::chrono::duration<Rep2, Period2>>(
					(asio::steady_timer::duration::max)());

			derived_t& derive = static_cast<derived_t&>(*this);

			std::function<void()> t = std::bind(std::forward<Fun>(fun), std::forward<Args>(args)...);

			// Whether or not we run on the io_context thread, We all start the timer by post an asynchronous 
			// event, in order to avoid unexpected problems caused by the user start or stop the 
			// timer again in the timer callback function.
			asio::post(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, &derive, this_ptr = derive.selfptr(),
				timer_handle = user_timer_handle(std::forward<TimerId>(timer_id)),
				interval, repeat, first_delay, callback = std::move(t)]() mutable
			{
				// if the timer is already exists, cancel it first.
				auto iter = this->user_timers_.find(timer_handle);
				if (iter != this->user_timers_.end())
				{
					iter->second->exited = true;
					detail::cancel_timer(iter->second->timer);
				}

				// the asio::steady_timer's constructor may be throw some exception.
				std::shared_ptr<user_timer_obj> timer_obj_ptr = std::make_shared<user_timer_obj>(
					timer_handle, derive.io_->context());

				// after asio::steady_timer's constructor is successed, then set the callback,
				// avoid the callback is empty by std::move(callback) when asio::steady_timer's
				// constructor throw some exception.
				timer_obj_ptr->callback = std::move(callback);
				timer_obj_ptr->interval = interval;
				timer_obj_ptr->repeat   = static_cast<std::size_t>(repeat);

				this->user_timers_[std::move(timer_handle)] = timer_obj_ptr;

				derive.io_->timers().emplace(std::addressof(timer_obj_ptr->timer));

				derive._post_user_timers(std::move(this_ptr), std::move(timer_obj_ptr), first_delay);
			}));
		}

		/**
		 * @brief stop the timer by timer id
		 */
		template<class TimerId>
		inline void stop_timer(TimerId&& timer_id)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// must use post, otherwise when call stop_timer immediately after start_timer
			// will cause the stop_timer has no effect.
			// can't use dispatch, otherwise if call stop_timer in timer callback, it will 
			// cause this : the stop_timer will cancel the timer, after stop_timer, the callback
			// will executed continue, then it will post next timer, beacuse the cancel is 
			// before the post next timer, so the cancel will has no effect. (so there was 
			// a flag "exit" to ensure this : even if use dispatch, the timer also can be
			// canceled success)
			asio::post(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, this_ptr = derive.selfptr(), timer_id = std::forward<TimerId>(timer_id)]
			() mutable
			{
				detail::ignore_unused(this_ptr);

				auto iter = this->user_timers_.find(timer_id);
				if (iter != this->user_timers_.end())
				{
					iter->second->exited = true;

					detail::cancel_timer(iter->second->timer);

					this->user_timers_.erase(iter);
				}
			}));
		}

		/**
		 * @brief stop all timers
		 */
		inline void stop_all_timers()
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// must use post, otherwise when call stop_all_timers immediately after start_timer
			// will cause the stop_all_timers has no effect.
			asio::post(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, this_ptr = derive.selfptr()]() mutable
			{
				// close user custom timers
				for (auto &[id, timer_obj_ptr] : this->user_timers_)
				{
					detail::ignore_unused(this_ptr, id);

					timer_obj_ptr->exited = true;

					detail::cancel_timer(timer_obj_ptr->timer);
				}
				this->user_timers_.clear();
			}));
		}

		/**
		 * @brief Returns true if the specified timer exists
		 */
		template<class TimerId>
		inline bool is_timer_exists(TimerId&& timer_id)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if (derive.io_->running_in_this_thread())
			{
				return (this->user_timers_.find(timer_id) != this->user_timers_.end());
			}

			std::promise<bool> prm;
			std::future<bool> fut = prm.get_future();

			asio::post(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, this_ptr = derive.selfptr(), timer_id = std::forward<TimerId>(timer_id), &prm]
			() mutable
			{
				detail::ignore_unused(this_ptr);

				prm.set_value(this->user_timers_.find(timer_id) != this->user_timers_.end());
			}));

			return fut.get();
		}

		/**
		 * @brief Get the interval for the specified timer.
		 */
		template<class TimerId>
		inline typename asio::steady_timer::duration get_timer_interval(TimerId&& timer_id)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if (derive.io_->running_in_this_thread())
			{
				auto iter = this->user_timers_.find(timer_id);
				if (iter != this->user_timers_.end())
				{
					return iter->second->interval;
				}
				else
				{
					return typename asio::steady_timer::duration{};
				}
			}

			std::promise<typename asio::steady_timer::duration> prm;
			std::future<typename asio::steady_timer::duration> fut = prm.get_future();

			asio::post(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, this_ptr = derive.selfptr(), timer_id = std::forward<TimerId>(timer_id), &prm]
			() mutable
			{
				detail::ignore_unused(this_ptr);

				auto iter = this->user_timers_.find(timer_id);
				if (iter != this->user_timers_.end())
				{
					prm.set_value(iter->second->interval);
				}
				else
				{
					prm.set_value(typename asio::steady_timer::duration{});
				}
			}));

			return fut.get();
		}

		/**
		 * @brief Set the interval for the specified timer.
		 */
		template<class TimerId, class Rep, class Period>
		inline void set_timer_interval(TimerId&& timer_id, std::chrono::duration<Rep, Period> interval)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if (interval > std::chrono::duration_cast<
				std::chrono::duration<Rep, Period>>((asio::steady_timer::duration::max)()))
				interval = std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(
					(asio::steady_timer::duration::max)());

			if (derive.io_->running_in_this_thread())
			{
				auto iter = this->user_timers_.find(timer_id);
				if (iter != this->user_timers_.end())
				{
					iter->second->interval = interval;
				}
				return;
			}

			asio::dispatch(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, this_ptr = derive.selfptr(), timer_id = std::forward<TimerId>(timer_id), interval]
			() mutable
			{
				detail::ignore_unused(this_ptr);

				auto iter = this->user_timers_.find(timer_id);
				if (iter != this->user_timers_.end())
				{
					iter->second->interval = interval;
				}
			}));
		}

		/**
		 * @brief Set the interval for the specified timer.
		 */
		template<class TimerId, class IntegerMilliseconds>
		inline typename std::enable_if_t<std::is_integral_v<detail::remove_cvref_t<IntegerMilliseconds>>, void>
		set_timer_interval(TimerId&& timer_id, IntegerMilliseconds interval)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return derive.set_timer_interval(std::forward<TimerId>(timer_id), std::chrono::milliseconds(interval));
		}

		/**
		 * @brief Reset the interval for the specified timer.
		 */
		template<class TimerId, class Rep, class Period>
		inline void reset_timer_interval(TimerId&& timer_id, std::chrono::duration<Rep, Period> interval)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return derive.set_timer_interval(std::forward<TimerId>(timer_id), interval);
		}

		/**
		 * @brief Reset the interval for the specified timer.
		 */
		template<class TimerId, class IntegerMilliseconds>
		inline typename std::enable_if_t<std::is_integral_v<detail::remove_cvref_t<IntegerMilliseconds>>, void>
		reset_timer_interval(TimerId&& timer_id, IntegerMilliseconds interval)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return derive.set_timer_interval(std::forward<TimerId>(timer_id), interval);
		}

	protected:
		/**
		 * @brief stop all timers
		 * Use dispatch instead of post, this function is used for inner.
		 */
		inline void _dispatch_stop_all_timers()
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// must use post, otherwise when call stop_all_timers immediately after start_timer
			// will cause the stop_all_timers has no effect.
			asio::dispatch(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, this_ptr = derive.selfptr()]() mutable
			{
				// close user custom timers
				for (auto &[id, timer_obj_ptr] : this->user_timers_)
				{
					detail::ignore_unused(this_ptr, id);

					timer_obj_ptr->exited = true;

					detail::cancel_timer(timer_obj_ptr->timer);
				}
				this->user_timers_.clear();
			}));
		}

	protected:
		template<class Rep, class Period>
		inline void _post_user_timers(std::shared_ptr<derived_t> this_ptr,
			std::shared_ptr<user_timer_obj> timer_obj_ptr, std::chrono::duration<Rep, Period> expiry)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(this->user_timers_.find(timer_obj_ptr->id) != this->user_timers_.end());

			asio::steady_timer& timer = timer_obj_ptr->timer;

			timer.expires_after(expiry);
			timer.async_wait(
			[&derive, this_ptr = std::move(this_ptr), timer_ptr = std::move(timer_obj_ptr)]
			(const error_code& ec) mutable
			{
				derive._handle_user_timers(ec, std::move(this_ptr), std::move(timer_ptr));
			});
		}

		inline void _handle_user_timers(error_code ec, std::shared_ptr<derived_t> this_ptr,
			std::shared_ptr<user_timer_obj> timer_obj_ptr)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT((!ec) || ec == asio::error::operation_aborted);

			if (timer_obj_ptr->exited && (!ec))
			{
				ec = asio::error::operation_aborted;
			}

			set_last_error(ec);

			// Should the callback function also be called if there is an error in the timer ?
			// It made me difficult to choose.After many adjustments and the requirements of the
			// actual project, it is more reasonable to still call the callback function when 
			// there are some errors.
			// eg. user call start_timer, then call stop_timer after a later, but the timer's 
			// timeout is not reached, perhaps the user wants the timer callback function also 
			// can be called even if the timer is aborted by stop_timer.

			if (timer_obj_ptr->repeat == static_cast<std::size_t>(-1))
			{
				derive._invoke_user_timer_callback(ec, timer_obj_ptr);
			}
			else
			{
				ASIO2_ASSERT(timer_obj_ptr->repeat > static_cast<std::size_t>(0));

				derive._invoke_user_timer_callback(ec, timer_obj_ptr);

				timer_obj_ptr->repeat--;
			}

			/**
			 * @note If the timer has already expired when cancel() is called, then the
			 * handlers for asynchronous wait operations will:
			 *
			 * @li have already been invoked; or
			 *
			 * @li have been queued for invocation in the near future.
			 *
			 * These handlers can no longer be cancelled, and therefore are passed an
			 * error code that indicates the successful completion of the wait operation.
			 */

			// must detect whether the timer is still exists by "exited", in some cases,
			// after remove and cancel a timer, the ec is 0 (not operation_aborted) and 
			// the steady_timer is still exist

			if (timer_obj_ptr->exited)
			{
				// if exited is true, can't erase the timer object from the "user_timers_",
				// beacuse maybe user start timer multi times with same id.
				derive.io_->timers().erase(std::addressof(timer_obj_ptr->timer));

				return;
			}

			if (ec == asio::error::operation_aborted || timer_obj_ptr->repeat == static_cast<std::size_t>(0))
			{
				derive.io_->timers().erase(std::addressof(timer_obj_ptr->timer));

				auto iter = this->user_timers_.find(timer_obj_ptr->id);
				if (iter != this->user_timers_.end())
				{
					iter->second->exited = true;

					this->user_timers_.erase(iter);
				}

				return;
			}

			typename asio::steady_timer::duration expiry = timer_obj_ptr->interval;

			derive._post_user_timers(std::move(this_ptr), std::move(timer_obj_ptr), expiry);
		}

		inline void _invoke_user_timer_callback(const error_code& ec, std::shared_ptr<user_timer_obj>& timer_obj_ptr)
		{
			detail::ignore_unused(ec);

		#if defined(ASIO2_ENABLE_TIMER_CALLBACK_WHEN_ERROR)
				(timer_obj_ptr->callback)();
		#else
			if (!ec)
			{
				(timer_obj_ptr->callback)();
			}
		#endif
		}

	protected:
		/// user-defined timer
		user_timer_map                                  user_timers_;
	};
}

#endif // !__ASIO2_USER_TIMER_COMPONENT_HPP__
