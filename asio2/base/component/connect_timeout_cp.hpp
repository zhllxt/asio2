/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
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
			, connect_timeout_timer_io_(timer_io)
			, connect_timeout_timer_(timer_io.context())
		{
			this->connect_timer_canceled_.clear();
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
		inline void _post_connect_timeout_timer(std::chrono::duration<Rep, Period> duration,
			std::shared_ptr<derived_t> this_ptr)
		{
			this->connect_error_code_.clear();
			this->connect_timeout_flag_.store(false);

			this->connect_timeout_timer_.expires_after(duration);
			this->connect_timeout_timer_.async_wait(
				asio::bind_executor(this->connect_timeout_timer_io_.strand(),
					[this, self_ptr = std::move(this_ptr)](const error_code& ec) mutable
			{
				// bug fixed : 
				// note : after call derive._handle_connect_timeout_timer(ec, std::move(self_ptr)); 
				// the pointer of "this" can no longer be used immediately, beacuse when 
				// self_ptr's reference counter is 1, after call 
				// derive._handle_connect_timeout_timer(ec, std::move(self_ptr)); the self_ptr's 
				// object will be destroyed, then below code "this->..." will cause crash.
				derive._handle_connect_timeout_timer(ec, std::move(self_ptr));
			}));
		}

		inline void _handle_connect_timeout_timer(const error_code& ec,
			std::shared_ptr<derived_t> this_ptr)
		{
			std::ignore = this_ptr;

			if (!ec)
			{
				this->connect_timeout_flag_.store(true);
			}

			if (ec == asio::error::operation_aborted || this->connect_timer_canceled_.test_and_set())
			{
				this->connect_timer_canceled_.clear();
				return;
			}

			this->connect_timer_canceled_.clear();

			if (!ec)
			{
				derive._do_disconnect(asio::error::timed_out);
			}
			else
			{
				derive._do_disconnect(this->connect_error_code_ ? this->connect_error_code_ : ec);
			}
		}

		template<class Rep, class Period, class Fn>
		inline void _post_connect_timeout_timer(std::chrono::duration<Rep, Period> duration,
			std::shared_ptr<derived_t> this_ptr, Fn&& fn)
		{
			this->connect_error_code_.clear();
			this->connect_timeout_flag_.store(false);

			this->connect_timeout_timer_.expires_after(duration);
			this->connect_timeout_timer_.async_wait(
				asio::bind_executor(this->connect_timeout_timer_io_.strand(),
					[this, self_ptr = std::move(this_ptr), f = std::forward<Fn>(fn)]
			(const error_code& ec) mutable
			{
				if (!ec)
				{
					this->connect_timeout_flag_.store(true);
				}

				this->connect_timer_canceled_.clear();

				f(ec);
			}));
		}

		inline void _stop_connect_timeout_timer(asio::error_code ec)
		{
			try
			{
				this->connect_error_code_ = ec;
				this->connect_timer_canceled_.test_and_set();
				this->connect_timeout_timer_.cancel(ec_ignore);
			}
			catch (system_error&) {}
			catch (std::exception&) {}
		}

		inline bool _is_connect_timeout()
		{
			return this->connect_timeout_flag_.load();
		}

		inline asio::error_code _connect_error_code()
		{
			return this->connect_error_code_;
		}

	protected:
		derived_t                                 & derive;

		io_t                                      & connect_timeout_timer_io_;

		asio::steady_timer                          connect_timeout_timer_;

		std::atomic_flag                            connect_timer_canceled_;

		std::chrono::milliseconds                   connect_timeout_         = std::chrono::seconds(5);

		asio::error_code                            connect_error_code_;

		/// Used to check the error_code is connection timeout or others.
		std::atomic_bool                            connect_timeout_flag_    = false;
	};
}

#endif // !__ASIO2_CONNECT_TIMEOUT_COMPONENT_HPP__
