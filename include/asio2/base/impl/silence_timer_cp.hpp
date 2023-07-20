/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_SILENCE_TIMER_COMPONENT_HPP__
#define __ASIO2_SILENCE_TIMER_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <chrono>

#include <asio2/base/iopool.hpp>
#include <asio2/base/log.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t>
	class silence_timer_cp
	{
	public:
		/**
		 * @brief constructor
		 */
		silence_timer_cp() = default;

		/**
		 * @brief destructor
		 */
		~silence_timer_cp() = default;

	public:
		/**
		 * @brief get silence timeout value
		 */
		inline std::chrono::steady_clock::duration get_silence_timeout() const noexcept
		{
			return this->silence_timeout_;
		}

		/**
		 * @brief set silence timeout value
		 */
		template<class Rep, class Period>
		inline derived_t & set_silence_timeout(std::chrono::duration<Rep, Period> duration) noexcept
		{
			if (duration > std::chrono::duration_cast<
				std::chrono::duration<Rep, Period>>((std::chrono::steady_clock::duration::max)()))
				this->silence_timeout_ = (std::chrono::steady_clock::duration::max)();
			else
				this->silence_timeout_ = duration;
			return static_cast<derived_t&>(*this);
		}

	protected:
		template<class Rep, class Period>
		inline void _post_silence_timer(
			std::chrono::duration<Rep, Period> duration, std::shared_ptr<derived_t> this_ptr)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			asio::dispatch(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, this_ptr = std::move(this_ptr), duration]() mutable
			{
				derived_t& derive = static_cast<derived_t&>(*this);

			#if defined(_DEBUG) || defined(DEBUG)
				ASIO2_ASSERT(this->is_stop_silence_timer_called_ == false);
			#endif

				// reset the "canceled" flag to false, see reconnect_timer_cp.hpp -> _make_reconnect_timer
				this->silence_timer_canceled_.clear();

				// start the timer of check silence timeout
				if (duration > std::chrono::duration<Rep, Period>::zero())
				{
					if (this->silence_timer_ == nullptr)
					{
						this->silence_timer_ = std::make_unique<asio::steady_timer>(derive.io_->context());
					}

					this->silence_timer_->expires_after(duration);
					this->silence_timer_->async_wait(
					[&derive, this_ptr = std::move(this_ptr)](const error_code & ec) mutable
					{
						derive._handle_silence_timer(ec, std::move(this_ptr));
					});
				}
			}));
		}

		inline void _handle_silence_timer(const error_code & ec, std::shared_ptr<derived_t> this_ptr)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT((!ec) || ec == asio::error::operation_aborted);

			// ec maybe zero when timer_canceled_ is true.
			if (ec == asio::error::operation_aborted || this->silence_timer_canceled_.test_and_set())
			{
				this->silence_timer_.reset();
				return;
			}

			this->silence_timer_canceled_.clear();

			// silence duration seconds not exceed the silence timeout,post a timer
			// event agagin to avoid this session shared_ptr object disappear.

			std::chrono::system_clock::duration silence = derive.get_silence_duration();

			if (silence < this->silence_timeout_)
			{
				derive._post_silence_timer(this->silence_timeout_ - silence, std::move(this_ptr));
			}
			else
			{
				// silence timeout has elasped,but has't data trans,don't post
				// a timer event again,so this session, shared_ptr will disappear 
				// and the object will be destroyed automatically after this handler returns.
				set_last_error(asio::error::timed_out);

				derive._do_disconnect(asio::error::timed_out, std::move(this_ptr));

				this->silence_timer_.reset();
			}
		}

		inline void _stop_silence_timer()
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			derive.dispatch([this]() mutable
			{
			#if defined(_DEBUG) || defined(DEBUG)
				this->is_stop_silence_timer_called_ = true;
			#endif

				this->silence_timer_canceled_.test_and_set();

				if (this->silence_timer_)
				{
					detail::cancel_timer(*(this->silence_timer_));
				}
			});
		}

	protected:
		/// timer for session silence time out
		std::unique_ptr<asio::steady_timer>         silence_timer_;

		/// Why use this flag, beacuase the ec param maybe zero when the timer callback is
		/// called after the timer cancel function has called already.
		std::atomic_flag                            silence_timer_canceled_;

		/// if there has no data transfer for a long time,the session will be disconnect
		std::chrono::steady_clock::duration         silence_timeout_ = std::chrono::milliseconds(tcp_silence_timeout);

	#if defined(_DEBUG) || defined(DEBUG)
		bool                                        is_stop_silence_timer_called_ = false;
	#endif
	};
}

#endif // !__ASIO2_SILENCE_TIMER_COMPONENT_HPP__
