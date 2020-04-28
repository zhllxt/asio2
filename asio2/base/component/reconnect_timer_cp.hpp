/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_RECONNECT_TIMER_COMPONENT_HPP__
#define __ASIO2_RECONNECT_TIMER_COMPONENT_HPP__

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
	class reconnect_timer_cp
	{
	public:
		/**
		 * @constructor
		 */
		explicit reconnect_timer_cp(io_t & timer_io)
			: derive(static_cast<derived_t&>(*this))
			, reconnect_timer_io_(timer_io)
			, reconnect_timer_(timer_io.context())
		{
		}

		/**
		 * @destructor
		 */
		~reconnect_timer_cp() = default;

	public:
		/**
		 * @function : set the option of whether reconnect when disconnected
		 * @param : enable - whether reconnect or not
		 * @param : delay - how long is the delay before reconnecting, when enalbe is
		 * false, the delay param is ignored
		 */
		template<class Rep, class Period>
		[[deprecated("Replace reconnect with auto_reconnect")]]
		inline derived_t& reconnect(bool enable, std::chrono::duration<Rep, Period> delay)
		{
			this->reconnect_enable_ = enable;
			if (this->reconnect_enable_)
				this->reconnect_delay_ = delay;
			return (derive);
		}

		/**
		 * @function : set the option of whether reconnect when disconnected
		 * @param : enable - whether reconnect or not
		 */
		template<typename = void>
		[[deprecated("Replace reconnect with auto_reconnect")]]
		inline derived_t& reconnect(bool enable)
		{
			this->reconnect_enable_ = enable;
			return (derive);
		}

		/**
		 * @function : set the option of whether auto reconnect when disconnected
		 * @param : enable - whether reconnect or not
		 * @param : delay - how long is the delay before reconnecting, when enalbe is
		 * false, the delay param is ignored
		 */
		template<class Rep, class Period>
		inline derived_t& auto_reconnect(bool enable, std::chrono::duration<Rep, Period> delay)
		{
			this->reconnect_enable_ = enable;
			if (this->reconnect_enable_)
				this->reconnect_delay_ = delay;
			return (derive);
		}

		/**
		 * @function : set the option of whether auto reconnect when disconnected
		 * @param : enable - whether reconnect or not
		 */
		template<typename = void>
		inline derived_t& auto_reconnect(bool enable)
		{
			this->reconnect_enable_ = enable;
			return (derive);
		}

	private:
		template<class Rep, class Period, class Callback>
		inline void _post_reconnect_timer(std::shared_ptr<derived_t> this_ptr, Callback&& f,
			std::chrono::duration<Rep, Period> delay)
		{
			this->reconnect_timer_.expires_after(delay);
			this->reconnect_timer_.async_wait(asio::bind_executor(this->reconnect_timer_io_.strand(),
				[this, self_ptr = std::move(this_ptr), f = std::forward<Callback>(f)]
			(const error_code & ec) mutable
			{
				derive._handle_reconnect_timer(ec, std::move(self_ptr), std::move(f));
			}));
		}

	protected:
		template<class Callback>
		inline void _make_reconnect_timer(std::shared_ptr<derived_t> this_ptr, Callback&& f)
		{
			if (this->reconnect_is_running_.test_and_set())
				return;

			derive._post_reconnect_timer(std::move(this_ptr), std::forward<Callback>(f),
				(std::chrono::nanoseconds::max)()); // 292 yeas
		}

		template<class Callback>
		inline void _handle_reconnect_timer(const error_code & ec, std::shared_ptr<derived_t> this_ptr, Callback&& f)
		{
			if (this->reconnect_timer_canceled_.test_and_set())
			{
				this->reconnect_is_running_.clear();
				return;
			}

			this->reconnect_timer_canceled_.clear();

			if (ec == asio::error::operation_aborted)
			{
				derive._post_reconnect_timer(std::move(this_ptr), std::forward<Callback>(f), this->reconnect_delay_);
			}
			else
			{
				if (this->reconnect_enable_)
				{
					f();
				}

				derive._post_reconnect_timer(std::move(this_ptr), std::forward<Callback>(f),
					(std::chrono::nanoseconds::max)()); // 292 yeas
			}
		}

		inline void _stop_reconnect_timer()
		{
			try
			{
				this->reconnect_timer_canceled_.test_and_set();
				this->reconnect_timer_.cancel();
			}
			catch (system_error &) {}
			catch (std::exception &) {}
		}

		inline void _wake_reconnect_timer()
		{
			try
			{
				if (this->reconnect_enable_)
					this->reconnect_timer_.cancel();
			}
			catch (system_error &) {}
			catch (std::exception &) {}
		}

	protected:
		derived_t                                 & derive;

		/// The io (include io_context and strand) used to handle the recv/send event.
		io_t                                      & reconnect_timer_io_;

		/// timer for client reconnect
		asio::steady_timer                          reconnect_timer_;

		/// 
		std::atomic_flag                            reconnect_timer_canceled_  = ATOMIC_FLAG_INIT;

		/// if there has no data transfer for a long time,the session will be disconnect
		std::chrono::milliseconds                   reconnect_delay_           = std::chrono::seconds(1);

		/// flag of whether reconnect when disconnect
		bool                                        reconnect_enable_          = true;

		/// Used to chech whether the reconnect timer has started already
		std::atomic_flag                            reconnect_is_running_      = ATOMIC_FLAG_INIT;
	};
}

#endif // !__ASIO2_RECONNECT_TIMER_COMPONENT_HPP__
