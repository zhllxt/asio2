/*
 * COPYRIGHT (C) 2017-2021, zhllxt
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

#include <asio2/external/asio.hpp>
#include <asio2/base/iopool.hpp>
#include <asio2/base/error.hpp>
#include <asio2/base/log.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t = void>
	class reconnect_timer_cp
	{
	public:
		/**
		 * @constructor
		 */
		explicit reconnect_timer_cp(io_t & io)
			: reconnect_timer_(io.context())
		{
			this->reconnect_timer_canceled_.clear();
			this->reconnect_is_running_.clear();
		}

		/**
		 * @destructor
		 */
		~reconnect_timer_cp() = default;

	public:
		/**
		 * @function : set the option of whether auto reconnect when disconnected, same as set_auto_reconnect
		 * @param : enable - whether reconnect or not
		 */
		template<typename = void>
		inline derived_t& auto_reconnect(bool enable) noexcept
		{
			return this->set_auto_reconnect(enable);
		}

		/**
		 * @function : set the option of whether auto reconnect when disconnected, same as set_auto_reconnect
		 * @param : enable - whether reconnect or not
		 * @param : delay - how long is the delay before reconnecting, when enalbe is
		 * false, the delay param is ignored
		 */
		template<class Rep, class Period>
		inline derived_t& auto_reconnect(bool enable, std::chrono::duration<Rep, Period> delay) noexcept
		{
			return this->set_auto_reconnect(enable, std::move(delay));
		}

		/**
		 * @function : set the option of whether auto reconnect when disconnected
		 * @param : enable - whether reconnect or not
		 */
		template<typename = void>
		inline derived_t& set_auto_reconnect(bool enable) noexcept
		{
			this->reconnect_enable_ = enable;
			return static_cast<derived_t&>(*this);
		}

		/**
		 * @function : set the option of whether auto reconnect when disconnected
		 * @param : enable - whether reconnect or not
		 * @param : delay - how long is the delay before reconnecting, when enalbe is
		 * false, the delay param is ignored
		 */
		template<class Rep, class Period>
		inline derived_t& set_auto_reconnect(bool enable, std::chrono::duration<Rep, Period> delay) noexcept
		{
			this->reconnect_enable_ = enable;
			this->reconnect_delay_  = delay;
			return static_cast<derived_t&>(*this);
		}

		/**
		 * @function : get whether auto reconnect is enabled or not
		 */
		template<typename = void>
		inline bool is_auto_reconnect() noexcept
		{
			return this->reconnect_enable_;
		}

		/**
		 * @function : get the delay before reconnecting, when enalbe is
		 */
		template<typename = void>
		inline std::chrono::steady_clock::duration get_auto_reconnect_delay() noexcept
		{
			return this->reconnect_delay_;
		}

	private:
		template<class Rep, class Period, class Callback>
		inline void _post_reconnect_timer(std::shared_ptr<derived_t> this_ptr, Callback&& f,
			std::chrono::duration<Rep, Period> delay)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if (!derive.io().strand().running_in_this_thread())
			{
				asio::post(derive.io().strand(), make_allocator(derive.wallocator(),
				[this, this_ptr = std::move(this_ptr), f = std::forward<Callback>(f), delay]() mutable
				{
					this->_post_reconnect_timer(std::move(this_ptr), std::move(f), delay);
				}));
				return;
			}

		#if defined(ASIO2_ENABLE_LOG)
			this->is_post_reconnect_timer_called_ = true;
			ASIO2_ASSERT(this->is_stop_reconnect_timer_called_ == false);
		#endif

			this->reconnect_timer_.expires_after(delay);
			this->reconnect_timer_.async_wait(asio::bind_executor(derive.io().strand(),
			[&derive, self_ptr = std::move(this_ptr), f = std::forward<Callback>(f)]
			(const error_code & ec) mutable
			{
				derive._handle_reconnect_timer(ec, std::move(self_ptr), std::move(f));
			}));
		}

	protected:
		template<class Callback>
		inline void _make_reconnect_timer(std::shared_ptr<derived_t> this_ptr, Callback&& f)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if (!derive.io().strand().running_in_this_thread())
			{
				asio::post(derive.io().strand(), make_allocator(derive.wallocator(),
				[this, this_ptr = std::move(this_ptr), f = std::forward<Callback>(f)]() mutable
				{
					this->_make_reconnect_timer(std::move(this_ptr), std::move(f));
				}));
				return;
			}

			if (this->reconnect_is_running_.test_and_set())
				return;

			// reset the "canceled" flag to false, otherwise after "client.stop();" then call client.start(...)
			// again, this reconnect timer will doesn't work .
			// can't put this "clear" code into the _handle_reconnect_timer, beacuse the _stop_reconnect_timer
			// maybe called many times. if do so, when the "canceled" flag is set false in the _handle_reconnect_timer
			// and the _stop_reconnect_timer is called later, then the "canceled" flag will be set true again .
			this->reconnect_timer_canceled_.clear();

			derive._post_reconnect_timer(std::move(this_ptr), std::forward<Callback>(f),
				(std::chrono::nanoseconds::max)()); // 292 yeas
		}

		template<class Callback>
		inline void _handle_reconnect_timer(const error_code& ec,
			std::shared_ptr<derived_t> this_ptr, Callback&& f)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT((!ec) || ec == asio::error::operation_aborted);

		#if defined(ASIO2_ENABLE_LOG)
			if (ec && ec != asio::error::operation_aborted)
			{
				ASIO2_LOG(spdlog::level::info, "reconnect_timer error : [{}] {}", ec.value(), ec.message());
			}
		#endif

			if (this->reconnect_timer_canceled_.test_and_set())
			{
				this->reconnect_is_running_.clear();
				return;
			}

			this->reconnect_timer_canceled_.clear();

			if (ec == asio::error::operation_aborted)
			{
				derive._post_reconnect_timer(std::move(this_ptr), std::forward<Callback>(f),
					this->reconnect_delay_);
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
			derived_t& derive = static_cast<derived_t&>(*this);

			if (!derive.io().strand().running_in_this_thread())
			{
				derive.post([this]() mutable
				{
					this->_stop_reconnect_timer();
				});
				return;
			}

		#if defined(ASIO2_ENABLE_LOG)
			this->is_stop_reconnect_timer_called_ = true;
		#endif

			error_code ec_ignore{};

			this->reconnect_timer_canceled_.test_and_set();
			this->reconnect_timer_.cancel(ec_ignore);
		}

		inline void _wake_reconnect_timer()
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if (!derive.io().strand().running_in_this_thread())
			{
				derive.post([this]() mutable
				{
					this->_wake_reconnect_timer();
				});
				return;
			}

		#if defined(ASIO2_ENABLE_LOG)
			ASIO2_ASSERT(this->is_post_reconnect_timer_called_ == true);
		#endif

			if (this->reconnect_enable_)
			{
				error_code ec_ignore{};
				this->reconnect_timer_.cancel(ec_ignore);
			}
		}

	protected:
		/// timer for client reconnect
		asio::steady_timer                          reconnect_timer_;

		/// 
		std::atomic_flag                            reconnect_timer_canceled_;

		/// if there has no data transfer for a long time,the session will be disconnect
		std::chrono::steady_clock::duration         reconnect_delay_           = std::chrono::seconds(1);

		/// flag of whether reconnect when disconnect
		bool                                        reconnect_enable_          = true;

		/// Used to chech whether the reconnect timer has started already
		std::atomic_flag                            reconnect_is_running_;

	#if defined(ASIO2_ENABLE_LOG)
		bool                                        is_stop_reconnect_timer_called_ = false;
		bool                                        is_post_reconnect_timer_called_ = false;
	#endif
	};
}

#endif // !__ASIO2_RECONNECT_TIMER_COMPONENT_HPP__
