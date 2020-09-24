/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_SILENCE_TIMER_COMPONENT_HPP__
#define __ASIO2_SILENCE_TIMER_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <chrono>

#include <asio2/base/selector.hpp>
#include <asio2/base/iopool.hpp>
#include <asio2/base/error.hpp>

namespace asio2::detail
{
	template<class derived_t, bool isSession>
	class silence_timer_cp
	{
	public:
		/**
		 * @constructor
		 */
		explicit silence_timer_cp(io_t & timer_io)
			: derive(static_cast<derived_t&>(*this))
			, silence_timer_io_(timer_io)
			, silence_timer_(timer_io.context())
		{
			this->silence_timer_canceled_.clear();
		}

		/**
		 * @destructor
		 */
		~silence_timer_cp() = default;

	public:
		/**
		 * @function : get silence timeout value ,unit : second
		 */
		inline std::chrono::milliseconds silence_timeout() const
		{
			return this->silence_timeout_;
		}

		/**
		 * @function : set silence timeout value
		 */
		template<class Rep, class Period>
		inline derived_t & silence_timeout(std::chrono::duration<Rep, Period> duration)
		{
			this->silence_timeout_ = duration;
			return (derive);
		}

	protected:
		template<class Rep, class Period>
		inline void _post_silence_timer(std::chrono::duration<Rep, Period> duration,
			std::shared_ptr<derived_t> this_ptr)
		{
			// start the timer of check silence timeout
			if (duration > std::chrono::milliseconds(0))
			{
				this->silence_timer_.expires_after(duration);
				this->silence_timer_.async_wait(asio::bind_executor(this->silence_timer_io_.strand(),
					[this, self_ptr = std::move(this_ptr)](const error_code & ec)
				{
					derive._handle_silence_timer(ec, std::move(self_ptr));
				}));
			}
		}

		inline void _handle_silence_timer(const error_code & ec, std::shared_ptr<derived_t> this_ptr)
		{
			if (ec == asio::error::operation_aborted || this->silence_timer_canceled_.test_and_set())
				return;

			this->silence_timer_canceled_.clear();

			// silence duration seconds not exceed the silence timeout,post a timer
			// event agagin to avoid this session shared_ptr object disappear.
			if (derive.silence_duration() < this->silence_timeout_)
			{
				derive._post_silence_timer(this->silence_timeout_ -
					derive.silence_duration(), std::move(this_ptr));
			}
			else
			{
				// silence timeout has elasped,but has't data trans,don't post
				// a timer event again,so this session, shared_ptr will disappear 
				// and the object will be destroyed automatically after this handler returns.
				set_last_error(asio::error::timed_out);
				derive._do_disconnect(asio::error::timed_out);
			}
		}

		inline void _stop_silence_timer()
		{
			this->silence_timer_canceled_.test_and_set();
			this->silence_timer_.cancel(ec_ignore);
		}

	protected:
		derived_t                                 & derive;

		/// The io (include io_context and strand) used to handle the recv/send event.
		io_t                                      & silence_timer_io_;

		/// timer for session silence time out
		asio::steady_timer                          silence_timer_;

		/// 
		std::atomic_flag                            silence_timer_canceled_;

		/// if there has no data transfer for a long time,the session will be disconnect
		std::chrono::milliseconds                   silence_timeout_ = std::chrono::minutes(60);
	};
}

#endif // !__ASIO2_SILENCE_TIMER_COMPONENT_HPP__
