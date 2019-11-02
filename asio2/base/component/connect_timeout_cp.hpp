/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_CONNECT_TIMEOUT_COMPONENT_HPP__
#define __ASIO2_CONNECT_TIMEOUT_COMPONENT_HPP__

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
	class connect_timeout_cp
	{
	public:
		/**
		 * @constructor
		 */
		explicit connect_timeout_cp(io_t & timer_io)
			: derive(static_cast<derived_t&>(*this))
			, timeout_timer_io_(timer_io)
			, timeout_timer_(timer_io.context())
		{
		}

		/**
		 * @destructor
		 */
		~connect_timeout_cp() = default;

		/**
		 * @function : get the connect timeout
		 */
		inline std::chrono::milliseconds connect_timeout() { return this->connect_timeout_; }

		/**
		 * @function : set the connect timeout
		 */
		template<class Rep, class Period>
		inline derived_t& connect_timeout(std::chrono::duration<Rep, Period> timeout)
		{
			this->connect_timeout_ = timeout;
			return (this->derive);
		}

	protected:
		template<class Rep, class Period>
		inline void _post_timeout_timer(std::chrono::duration<Rep, Period> duration, std::shared_ptr<derived_t> this_ptr)
		{
			this->timeout_timer_.expires_after(duration);
			this->timeout_timer_.async_wait(asio::bind_executor(this->timeout_timer_io_.strand(),
				[this, self_ptr = std::move(this_ptr)](const error_code & ec) mutable
			{
				derive._handle_timeout_timer(ec, std::move(self_ptr));
				this->timer_canceled_.clear();
			}));
		}

		inline void _handle_timeout_timer(const error_code & ec, std::shared_ptr<derived_t> this_ptr)
		{
			std::ignore = this_ptr;

			if (ec == asio::error::operation_aborted || this->timer_canceled_.test_and_set()) return;

			derive._do_stop(asio::error::timed_out);
		}

		template<class Rep, class Period, class Fn>
		inline std::future<error_code> _post_timeout_timer(std::chrono::duration<Rep, Period> duration,
			std::shared_ptr<derived_t> this_ptr, Fn&& fn)
		{
			std::promise<error_code> promise;
			std::future<error_code> future = promise.get_future();

			this->timeout_timer_.expires_after(duration);
			this->timeout_timer_.async_wait(asio::bind_executor(this->timeout_timer_io_.strand(),
				[this, self_ptr = std::move(this_ptr), p = std::move(promise), f = std::forward<Fn>(fn)]
			(const error_code & ec) mutable
			{
				f(ec);
				p.set_value(ec);
				this->timer_canceled_.clear();
			}));

			return future;
		}

		template<class Future>
		inline void _wait_timeout_timer(Future&& future)
		{
			if (!this->timeout_timer_io_.strand().running_in_this_thread())
				future.wait();
		}

		inline void _stop_timeout_timer()
		{
			this->timer_canceled_.test_and_set();
			try
			{
				this->timeout_timer_.cancel();
			}
			catch (system_error &) {}
			catch (std::exception &) {}
		}

	protected:
		derived_t                                 & derive;

		io_t                                      & timeout_timer_io_;

		asio::steady_timer                          timeout_timer_;

		std::atomic_flag                            timer_canceled_ = ATOMIC_FLAG_INIT;

		std::chrono::milliseconds                   connect_timeout_ = std::chrono::seconds(5);
	};
}

#endif // !__ASIO2_CONNECT_TIMEOUT_COMPONENT_HPP__
