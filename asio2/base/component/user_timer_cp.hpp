/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
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

#include <asio2/base/selector.hpp>
#include <asio2/base/iopool.hpp>
#include <asio2/base/error.hpp>

#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/allocator.hpp>

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

		inline bool operator==(const user_timer_handle& r) const
		{
			return (r.handle == this->handle);
		}

		template<typename T>
		inline void bind(T&& id)
		{
			using type = std::remove_cv_t<std::remove_reference_t<T>>;
			using rtype = std::remove_pointer_t<std::remove_all_extents_t<type>>;
			if constexpr /**/ (std::is_same_v<std::string, type>)
			{
				handle = std::forward<T>(id);
			}
			else if constexpr (is_string_view_v<type>)
			{
				handle.resize(sizeof(typename type::value_type) * id.size());
				std::memcpy((void*)handle.data(), (const void*)id.data(),
					sizeof(typename type::value_type) * id.size());
			}
			else if constexpr (is_string_v<type>)
			{
				handle.resize(sizeof(typename type::value_type) * id.size());
				std::memcpy((void*)handle.data(), (const void*)id.data(),
					sizeof(typename type::value_type) * id.size());
			}
			else if constexpr (std::is_integral_v<type>)
			{
				handle.resize(sizeof(type));
				std::memcpy((void*)handle.data(), (const void*)&id, sizeof(type));
			}
			else if constexpr (std::is_floating_point_v<type>)
			{
				handle.resize(sizeof(type));
				std::memcpy((void*)handle.data(), (const void*)&id, sizeof(type));
			}
			else if constexpr (std::is_pointer_v<type>)
			{
				ASIO2_ASSERT(id && (*id));
				if (id)
				{
					std::basic_string_view<rtype> sv{ id };
					handle.resize(sizeof(rtype) * sv.size());
					std::memcpy((void*)handle.data(), (const void*)sv.data(), sizeof(rtype) * sv.size());
				}
			}
			else if constexpr (std::is_array_v<type>)
			{
				std::basic_string_view<rtype> sv{ id };
				handle.resize(sizeof(rtype) * sv.size());
				std::memcpy((void*)handle.data(), (const void*)sv.data(), sizeof(rtype) * sv.size());
			}
			else if constexpr (std::is_same_v<user_timer_handle, type>)
			{
				ASIO2_ASSERT(false);
			}
			else
			{
				ASIO2_ASSERT(false);
			}
		}

		std::string handle;
	};

	struct user_timer_handle_hash
	{
		inline std::size_t operator()(const user_timer_handle& k) const
		{
			return std::hash<std::string>{}(k.handle);
		}
	};

	struct user_timer_handle_equal
	{
		inline bool operator()(const user_timer_handle& lhs, const user_timer_handle& rhs) const
		{
			return lhs.handle == rhs.handle;
		}
	};

	struct user_timer_obj
	{
		user_timer_handle id;
		asio::steady_timer timer;
		std::function<void()> task;
		bool exited = false;

		user_timer_obj(user_timer_handle Id, asio::io_context & context, std::function<void()> t)
			: id(std::move(Id)), timer(context), task(std::move(t)) {}
	};

	template<class derived_t, bool isSession>
	class user_timer_cp
	{
	public:
		using user_timer_map = std::unordered_map<user_timer_handle, std::shared_ptr<user_timer_obj>,
			user_timer_handle_hash, user_timer_handle_equal>;

		/**
		 * @constructor
		 */
		explicit user_timer_cp(io_t & timer_io)
			: derive(static_cast<derived_t&>(*this))
			, user_timer_io_(timer_io)
		{
		}

		/**
		 * @destructor
		 */
		~user_timer_cp()
		{
		}

	public:
		template<class TimerId, class Rep, class Period, class Fun, class... Args>
		inline void start_timer(TimerId&& timer_id, std::chrono::duration<Rep, Period> duration,
			Fun&& fun, Args&&... args)
		{
			std::function<void()> t = std::bind(std::forward<Fun>(fun), std::forward<Args>(args)...);

			auto fn = [this, this_ptr = derive.selfptr(),
				timer_handle = user_timer_handle(std::forward<TimerId>(timer_id)),
				duration, task = std::move(t)]() mutable
			{
				std::shared_ptr<user_timer_obj> timer_obj_ptr;

				auto iter = this->user_timers_.find(timer_handle);
				if (iter != this->user_timers_.end())
				{
					timer_obj_ptr = iter->second;
					timer_obj_ptr->task = std::move(task);
				}
				else
				{
					timer_obj_ptr = std::make_shared<user_timer_obj>(timer_handle,
						this->user_timer_io_.context(), std::move(task));

					this->user_timers_[std::move(timer_handle)] = timer_obj_ptr;
				}

				derive._post_user_timers(std::move(timer_obj_ptr), duration, std::move(this_ptr));
			};

			// Make sure we run on the strand
			if (!this->user_timer_io_.strand().running_in_this_thread())
				asio::post(this->user_timer_io_.strand(),
					make_allocator(derive.wallocator(), std::move(fn)));
			else
				fn();
		}

		template<class TimerId>
		inline void stop_timer(TimerId&& timer_id)
		{
			// Make sure we run on the strand
			if (!this->user_timer_io_.strand().running_in_this_thread())
				return asio::post(this->user_timer_io_.strand(),
					make_allocator(derive.wallocator(),
						[this, this_ptr = derive.selfptr(),
						timer_id = std::forward<TimerId>(timer_id)]() mutable
			{
				this->stop_timer(std::move(timer_id));
			}));

			auto iter = this->user_timers_.find(timer_id);
			if (iter != this->user_timers_.end())
			{
				iter->second->exited = true;
				iter->second->timer.cancel(ec_ignore);
				this->user_timers_.erase(iter);
			}
		}

		/**
		 * @function : stop session
		 * note : this function must be noblocking,if it's blocking,will 
		 *        cause circle lock in session_mgr::stop function
		 */
		inline void stop_all_timers()
		{
			if (!this->user_timer_io_.strand().running_in_this_thread())
				return asio::post(this->user_timer_io_.strand(),
					make_allocator(derive.wallocator(),
						[this, this_ptr = derive.selfptr()]()
			{
				this->stop_all_timers();
			}));

			// close user custom timers
			for (auto &[id, timer_obj_ptr] : this->user_timers_)
			{
				std::ignore = id;
				timer_obj_ptr->exited = true;
				timer_obj_ptr->timer.cancel(ec_ignore);
			}
			this->user_timers_.clear();
		}

	protected:
		template<class Rep, class Period>
		inline void _post_user_timers(std::shared_ptr<user_timer_obj> timer_obj_ptr,
			std::chrono::duration<Rep, Period> duration, std::shared_ptr<derived_t> this_ptr)
		{
			// detect whether the timer is still exists, in some cases, after erase and
			// cancel a timer, the steady_timer is still exist
			if (timer_obj_ptr->exited)
				return;

			asio::steady_timer& timer = timer_obj_ptr->timer;

			timer.expires_after(duration);
			timer.async_wait(asio::bind_executor(this->user_timer_io_.strand(),
				make_allocator(derive.wallocator(), [this, timer_ptr = std::move(timer_obj_ptr), duration,
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

			if (!ec)
				(timer_obj_ptr->task)();

			if (ec == asio::error::operation_aborted)
				return;

			derive._post_user_timers(std::move(timer_obj_ptr), duration, std::move(this_ptr));
		}

	protected:
		derived_t                                     & derive;

		/// The io (include io_context and strand) 
		io_t                                          & user_timer_io_;

		/// user-defined timer
		user_timer_map                                  user_timers_;
	};
}

#endif // !__ASIO2_USER_TIMER_COMPONENT_HPP__
