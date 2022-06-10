/*
 * COPYRIGHT (C) 2017-2021, zhllxt
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

#include <asio2/external/asio.hpp>
#include <asio2/base/iopool.hpp>
#include <asio2/base/error.hpp>
#include <asio2/base/log.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t = void>
	class silence_timer_cp
	{
	public:
		/**
		 * @constructor
		 */
		explicit silence_timer_cp(io_t & io)
			: silence_timer_(io.context())
		{
			this->silence_timer_canceled_.clear();
		}

		/**
		 * @destructor
		 */
		~silence_timer_cp() = default;

	public:
		/**
		 * @function : get silence timeout value
		 */
		inline std::chrono::steady_clock::duration get_silence_timeout() const noexcept
		{
			return this->silence_timeout_;
		}

		/**
		 * @function : set silence timeout value
		 */
		template<class Rep, class Period>
		inline derived_t & set_silence_timeout(std::chrono::duration<Rep, Period> duration) noexcept
		{
			this->silence_timeout_ = duration;
			return static_cast<derived_t&>(*this);
		}

	protected:
		template<class Rep, class Period>
		inline void _post_silence_timer(std::chrono::duration<Rep, Period> duration,
			std::shared_ptr<derived_t> this_ptr)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if (!derive.io().strand().running_in_this_thread())
			{
				asio::post(derive.io().strand(), make_allocator(derive.wallocator(),
				[this, this_ptr = std::move(this_ptr), duration]() mutable
				{
					this->_post_silence_timer(duration, std::move(this_ptr));
				}));
				return;
			}

		#if defined(ASIO2_ENABLE_LOG)
			ASIO2_ASSERT(this->is_stop_silence_timer_called_ == false);
		#endif

			// reset the "canceled" flag to false, see reconnect_timer_cp.hpp -> _make_reconnect_timer
			this->silence_timer_canceled_.clear();

			// start the timer of check silence timeout
			if (duration > std::chrono::duration<Rep, Period>::zero())
			{
				this->silence_timer_.expires_after(duration);
				this->silence_timer_.async_wait(asio::bind_executor(derive.io().strand(),
				[&derive, self_ptr = std::move(this_ptr)](const error_code & ec) mutable
				{
					derive._handle_silence_timer(ec, std::move(self_ptr));
				}));
			}
		}

		inline void _handle_silence_timer(const error_code & ec, std::shared_ptr<derived_t> this_ptr)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT((!ec) || ec == asio::error::operation_aborted);

		#if defined(ASIO2_ENABLE_LOG)
			if (ec && ec != asio::error::operation_aborted)
			{
				ASIO2_LOG(spdlog::level::info, "silence_timer error : [{}] {}", ec.value(), ec.message());
			}
		#endif

			if (ec == asio::error::operation_aborted || this->silence_timer_canceled_.test_and_set())
				return;

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
			}
		}

		inline void _stop_silence_timer()
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if (!derive.io().strand().running_in_this_thread())
			{
				derive.post([this]() mutable
				{
					this->_stop_silence_timer();
				});
				return;
			}

		#if defined(ASIO2_ENABLE_LOG)
			this->is_stop_silence_timer_called_ = true;
		#endif

			error_code ec_ignore{};

			this->silence_timer_canceled_.test_and_set();
			this->silence_timer_.cancel(ec_ignore);
		}

	protected:
		/// timer for session silence time out
		asio::steady_timer                          silence_timer_;

		/// 
		std::atomic_flag                            silence_timer_canceled_;

		/// if there has no data transfer for a long time,the session will be disconnect
		std::chrono::steady_clock::duration         silence_timeout_ = std::chrono::minutes(60);

	#if defined(ASIO2_ENABLE_LOG)
		bool                                        is_stop_silence_timer_called_ = false;
	#endif
	};
}

#endif // !__ASIO2_SILENCE_TIMER_COMPONENT_HPP__
