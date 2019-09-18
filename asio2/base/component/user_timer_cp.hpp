/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
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

#include <asio2/base/selector.hpp>
#include <asio2/base/iopool.hpp>
#include <asio2/base/error.hpp>

#include <asio2/base/detail/allocator.hpp>

namespace asio2::detail
{
#ifndef ASIO2_USER_TIMER_ID_TYPE
#define ASIO2_USER_TIMER_ID_TYPE std::size_t
#endif

	struct user_timer_obj
	{
		ASIO2_USER_TIMER_ID_TYPE id;
		asio::steady_timer timer;
		std::function<void()> task;
		handler_memory<> allocator;

		user_timer_obj(ASIO2_USER_TIMER_ID_TYPE Id, asio::io_context & context, std::function<void()> t)
			: id(Id), timer(context), task(std::move(t)) {}
	};

	template<typename T>
	struct has_mfn_is_started
	{
	private:
		template<typename U>
		static auto check(bool) -> decltype(std::declval<U>().is_started(), std::true_type());
		template<typename U>
		static std::false_type check(...);
	public:
		static constexpr bool value = std::is_same_v<decltype(check<T>(true)), std::true_type>;
	};

	template<class derived_t, bool isSession>
	class user_timer_cp
	{
	public:
		/**
		 * @constructor
		 */
		explicit user_timer_cp(io_t & timer_io) : derive(static_cast<derived_t&>(*this)), user_timer_io_(timer_io)
		{
		}

		/**
		 * @destructor
		 */
		~user_timer_cp()
		{
		}

	public:
		template<class Rep, class Period, class Fun, class... Args>
		inline void start_timer(ASIO2_USER_TIMER_ID_TYPE timer_id, std::chrono::duration<Rep, Period> duration, Fun&& fun, Args&&... args)
		{
			std::function<void()> t = std::bind(std::forward<Fun>(fun), std::forward<Args>(args)...);

			auto fn = [this, this_ptr = this->_mkptr(), timer_id, duration, task = std::move(t)]()
			{
				if constexpr (has_mfn_is_started<derived_t>::value)
				{
					if (!derive.is_started())
						return;
				}

				std::shared_ptr<user_timer_obj> timer_obj_ptr = std::make_shared<user_timer_obj>(
					timer_id, this->user_timer_io_.context(), std::move(task));

				auto pair = this->user_timers_.try_emplace(timer_id, std::move(timer_obj_ptr));
				if (pair.second == false)
					pair.first->second->task = std::move(timer_obj_ptr->task);

				derive._post_user_timers(pair.first->second, duration, std::move(this_ptr));
			};

			// Make sure we run on the strand
			if (!this->user_timer_io_.strand().running_in_this_thread())
				asio::post(this->user_timer_io_.strand(), std::move(fn));
			else
				fn();
		}

		inline void stop_timer(ASIO2_USER_TIMER_ID_TYPE timer_id)
		{
			// Make sure we run on the strand
			if (!this->user_timer_io_.strand().running_in_this_thread())
				return asio::post(this->user_timer_io_.strand(),
					[this, this_ptr = this->_mkptr(), timer_id]()
			{
				this->stop_timer(timer_id);
			});

			auto iter = this->user_timers_.find(timer_id);
			if (iter != this->user_timers_.end())
			{
				iter->second->timer.cancel();
				this->user_timers_.erase(iter);
			}
		}

		/**
		 * @function : stop session
		 * note : this function must be noblocking,if it's blocking,will cause circle lock in session_mgr::stop function
		 */
		inline void stop_all_timers()
		{
			if (!this->user_timer_io_.strand().running_in_this_thread())
				return asio::post(this->user_timer_io_.strand(),
					[this, this_ptr = this->_mkptr()]()
			{
				this->stop_all_timers();
			});

			// close user custom timers
			for (auto &[id, timer_obj_ptr] : this->user_timers_)
			{
				timer_obj_ptr->timer.cancel();
			}
			this->user_timers_.clear();
		}

	protected:
		template<class Rep, class Period>
		inline void _post_user_timers(std::shared_ptr<user_timer_obj> timer_obj_ptr,
			std::chrono::duration<Rep, Period> duration, std::shared_ptr<derived_t> this_ptr)
		{
			auto iter = this->user_timers_.find(timer_obj_ptr->id);
			if (iter == this->user_timers_.end())
				return;

			asio::steady_timer& timer = timer_obj_ptr->timer;
			handler_memory<>& allocator = timer_obj_ptr->allocator;

			timer.expires_after(duration);
			timer.async_wait(asio::bind_executor(this->user_timer_io_.strand(),
				make_allocator(allocator, [this, timer_ptr = std::move(timer_obj_ptr), duration,
					self_ptr = std::move(this_ptr)](const error_code & ec)
			{
				derive._handle_user_timers(ec, std::move(timer_ptr), duration, std::move(self_ptr));
			})));
		}

		template<class Rep, class Period>
		inline void _handle_user_timers(const error_code & ec, std::shared_ptr<user_timer_obj> timer_obj_ptr,
			std::chrono::duration<Rep, Period> duration, std::shared_ptr<derived_t> this_ptr)
		{
			set_last_error(ec);

			(timer_obj_ptr->task)();

			if (ec == asio::error::operation_aborted)
				return;

			derive._post_user_timers(std::move(timer_obj_ptr), duration, std::move(this_ptr));
		}

	protected:
		inline std::shared_ptr<derived_t> _mkptr()
		{
			if constexpr (isSession)
				return derive.shared_from_this();
			else
				return std::shared_ptr<derived_t>{};
		}

	protected:
		derived_t                                     & derive;

		/// The io (include io_context and strand) 
		io_t                                          & user_timer_io_;

		/// user-defined timer
		std::unordered_map<ASIO2_USER_TIMER_ID_TYPE, std::shared_ptr<user_timer_obj>> user_timers_;
	};
}

#endif // !__ASIO2_USER_TIMER_COMPONENT_HPP__
