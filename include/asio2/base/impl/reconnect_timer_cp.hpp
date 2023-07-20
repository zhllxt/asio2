/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_RECONNECT_TIMER_COMPONENT_HPP__
#define __ASIO2_RECONNECT_TIMER_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <chrono>

#include <asio2/base/iopool.hpp>
#include <asio2/base/log.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t>
	class reconnect_timer_cp
	{
	public:
		/**
		 * @brief constructor
		 */
		 reconnect_timer_cp() = default;

		/**
		 * @brief destructor
		 */
		~reconnect_timer_cp() = default;

	public:
		/**
		 * @brief set the option of whether auto reconnect when disconnected, same as set_auto_reconnect
		 * @param enable - whether reconnect or not
		 */
		template<typename = void>
		inline derived_t& auto_reconnect(bool enable) noexcept
		{
			return this->set_auto_reconnect(enable);
		}

		/**
		 * @brief set the option of whether auto reconnect when disconnected, same as set_auto_reconnect
		 * @param enable - whether reconnect or not
		 * @param delay - how long is the delay before reconnecting, when enalbe is
		 * false, the delay param is ignored
		 */
		template<class Rep, class Period>
		inline derived_t& auto_reconnect(bool enable, std::chrono::duration<Rep, Period> delay) noexcept
		{
			return this->set_auto_reconnect(enable, std::move(delay));
		}

		/**
		 * @brief set the option of whether auto reconnect when disconnected
		 * @param enable - whether reconnect or not
		 */
		template<typename = void>
		inline derived_t& set_auto_reconnect(bool enable) noexcept
		{
			this->reconnect_enable_ = enable;
			return static_cast<derived_t&>(*this);
		}

		/**
		 * @brief set the option of whether auto reconnect when disconnected
		 * @param enable - whether reconnect or not
		 * @param delay - how long is the delay before reconnecting, when enalbe is
		 * false, the delay param is ignored
		 */
		template<class Rep, class Period>
		inline derived_t& set_auto_reconnect(bool enable, std::chrono::duration<Rep, Period> delay) noexcept
		{
			this->reconnect_enable_ = enable;

			if (delay > std::chrono::duration_cast<
				std::chrono::duration<Rep, Period>>((std::chrono::steady_clock::duration::max)()))
				this->reconnect_delay_ = (std::chrono::steady_clock::duration::max)();
			else
				this->reconnect_delay_ = delay;

			return static_cast<derived_t&>(*this);
		}

		/**
		 * @brief get whether auto reconnect is enabled or not
		 */
		template<typename = void>
		inline bool is_auto_reconnect() const noexcept
		{
			return this->reconnect_enable_;
		}

		/**
		 * @brief get the delay before reconnecting, when enalbe is
		 */
		template<typename = void>
		inline std::chrono::steady_clock::duration get_auto_reconnect_delay() const noexcept
		{
			return this->reconnect_delay_;
		}

	protected:
		template<class C>
		inline void _make_reconnect_timer(std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			asio::dispatch(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, this_ptr = std::move(this_ptr), ecs = std::move(ecs)]() mutable
			{
				derived_t& derive = static_cast<derived_t&>(*this);

				if (this->reconnect_timer_)
				{
					this->reconnect_timer_->cancel();
				}

				this->reconnect_timer_ = std::make_shared<safe_timer>(derive.io_->context());

				derive._post_reconnect_timer(std::move(this_ptr), std::move(ecs),
					this->reconnect_timer_, (std::chrono::nanoseconds::max)()); // 292 yeas
			}));
		}

		template<class Rep, class Period, class C>
		inline void _post_reconnect_timer(
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs,
			std::shared_ptr<safe_timer> timer_ptr, std::chrono::duration<Rep, Period> delay)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

		#if defined(_DEBUG) || defined(DEBUG)
			this->is_post_reconnect_timer_called_ = true;
			ASIO2_ASSERT(this->is_stop_reconnect_timer_called_ == false);
		#endif

			// When goto timer callback, and execute the reconnect operation : 
			// call _start_connect -> _make_reconnect_timer -> a new timer is maked, and 
			// the prev timer will be canceled, then call _post_reconnect_timer, the prev
			// timer will be enqueue to, this will cause the prev timer never be exit.
			// so we check if timer_ptr is not equal the member variable reconnect_timer_,
			// don't call async_wait again.
			if (timer_ptr.get() != this->reconnect_timer_.get())
				return;

			safe_timer* ptimer = timer_ptr.get();

			ptimer->timer.expires_after(delay);
			ptimer->timer.async_wait(
			[&derive, this_ptr = std::move(this_ptr), ecs = std::move(ecs), timer_ptr = std::move(timer_ptr)]
			(const error_code & ec) mutable
			{
				derive._handle_reconnect_timer(ec, std::move(this_ptr), std::move(ecs), std::move(timer_ptr));
			});
		}

		template<class C>
		inline void _handle_reconnect_timer(const error_code& ec,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, std::shared_ptr<safe_timer> timer_ptr)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT((!ec) || ec == asio::error::operation_aborted);

			// a new timer is maked, this is the prev timer, so return directly.
			if (timer_ptr.get() != this->reconnect_timer_.get())
				return;

			// member variable timer should't be empty
			if (!this->reconnect_timer_)
			{
				ASIO2_ASSERT(false);
				return;
			}

			// current reconnect timer is canceled, so return directly
			if (timer_ptr->canceled.test_and_set())
			{
				this->reconnect_timer_.reset();
				return;
			}

			timer_ptr->canceled.clear();

			if (ec == asio::error::operation_aborted)
			{
				derive._post_reconnect_timer(
					std::move(this_ptr), std::move(ecs), std::move(timer_ptr), this->reconnect_delay_);
			}
			else
			{
				if (this->reconnect_enable_)
				{
					derive.push_event(
					[&derive, this_ptr, ecs, timer_ptr](event_queue_guard<derived_t> g) mutable
					{
						if (timer_ptr->canceled.test_and_set())
							return;

						timer_ptr->canceled.clear();

						state_t expected = state_t::stopped;
						if (derive.state_.compare_exchange_strong(expected, state_t::starting))
						{
							derive.template _start_connect<true>(
								std::move(this_ptr), std::move(ecs), defer_event(std::move(g)));
						}
					});
				}

				derive._post_reconnect_timer(std::move(this_ptr), std::move(ecs),
					std::move(timer_ptr), (std::chrono::nanoseconds::max)()); // 292 yeas
			}
		}

		inline void _stop_reconnect_timer()
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			derive.dispatch([this]() mutable
			{
			#if defined(_DEBUG) || defined(DEBUG)
				this->is_stop_reconnect_timer_called_ = true;
			#endif

				if (this->reconnect_timer_)
				{
					this->reconnect_timer_->cancel();
				}
			});
		}

		inline void _wake_reconnect_timer()
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			derive.dispatch([this]() mutable
			{
			#if defined(_DEBUG) || defined(DEBUG)
				ASIO2_ASSERT(this->is_post_reconnect_timer_called_ == true);
			#endif

				if (this->reconnect_enable_)
				{
					ASIO2_ASSERT(this->reconnect_timer_);

					if (this->reconnect_timer_)
					{
						detail::cancel_timer(this->reconnect_timer_->timer);
					}
				}
			});
		}

	protected:
		/// timer for client reconnect
		std::shared_ptr<safe_timer>                 reconnect_timer_;

		/// if there has no data transfer for a long time,the session will be disconnect
		std::chrono::steady_clock::duration         reconnect_delay_           = std::chrono::seconds(1);

		/// flag of whether reconnect when disconnect
		bool                                        reconnect_enable_          = true;

	#if defined(_DEBUG) || defined(DEBUG)
		bool                                        is_stop_reconnect_timer_called_ = false;
		bool                                        is_post_reconnect_timer_called_ = false;
	#endif
	};
}

#endif // !__ASIO2_RECONNECT_TIMER_COMPONENT_HPP__
