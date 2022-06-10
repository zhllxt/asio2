/*
 * COPYRIGHT (C) 2017-2021, zhllxt
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

#include <asio2/external/asio.hpp>
#include <asio2/base/iopool.hpp>
#include <asio2/base/error.hpp>
#include <asio2/base/log.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t = void>
	class connect_timeout_cp
	{
	public:
		/**
		 * @constructor
		 */
		explicit connect_timeout_cp(io_t & io)
			: connect_timeout_timer_(io.context())
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
		inline std::chrono::steady_clock::duration get_connect_timeout() noexcept
		{
			return this->connect_timeout_;
		}

		/**
		 * @function : set the connect timeout
		 */
		template<class Rep, class Period>
		inline derived_t& set_connect_timeout(std::chrono::duration<Rep, Period> timeout) noexcept
		{
			this->connect_timeout_ = timeout;
			return static_cast<derived_t&>(*this);
		}

	protected:
		template<class Rep, class Period>
		inline void _post_connect_timeout_timer(
			std::chrono::duration<Rep, Period> duration, std::shared_ptr<derived_t> this_ptr)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if (!derive.io().strand().running_in_this_thread())
			{
				asio::post(derive.io().strand(), make_allocator(derive.wallocator(),
				[this, this_ptr = std::move(this_ptr), duration]() mutable
				{
					this->_post_connect_timeout_timer(duration, std::move(this_ptr));
				}));
				return;
			}

		#if defined(ASIO2_ENABLE_LOG)
			ASIO2_ASSERT(this->is_stop_connect_timeout_timer_called_ == false);
		#endif

			this->connect_error_code_.clear();
			this->connect_timeout_flag_.store(false);

			// reset the "canceled" flag to false, see reconnect_timer_cp.hpp -> _make_reconnect_timer
			this->connect_timer_canceled_.clear();

			this->connect_timeout_timer_.expires_after(duration);
			this->connect_timeout_timer_.async_wait(asio::bind_executor(derive.io().strand(),
			[&derive, self_ptr = std::move(this_ptr)](const error_code& ec) mutable
			{
				// bug fixed : 
				// note : after call derive._handle_connect_timeout_timer(ec, std::move(self_ptr)); 
				// the pointer of "this" can no longer be used immediately, beacuse when 
				// self_ptr's reference counter is 1, after call 
				// derive._handle_connect_timeout_timer(ec, std::move(self_ptr)); the self_ptr's 
				// object will be destroyed, then below code "this->..." will cause crash.
				derive._handle_connect_timeout_timer(ec, std::move(self_ptr));

				// can't do it like below, beacuse "this" maybe deleted already.
				// this->...
			}));
		}

		inline void _handle_connect_timeout_timer(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT((!ec) || ec == asio::error::operation_aborted);

		#if defined(ASIO2_ENABLE_LOG)
			if (ec && ec != asio::error::operation_aborted)
			{
				ASIO2_LOG(spdlog::level::info, "connect_timeout_timer error : [{}] {}", ec.value(), ec.message());
			}
		#endif

			if (!ec)
			{
				this->connect_timeout_flag_.store(true);
			}

			if (ec == asio::error::operation_aborted || this->connect_timer_canceled_.test_and_set())
				return;

			this->connect_timer_canceled_.clear();

			if (!ec)
			{
				derive._do_disconnect(asio::error::timed_out, std::move(this_ptr));
			}
			else
			{
				derive._do_disconnect(
					this->connect_error_code_ ? this->connect_error_code_ : ec, std::move(this_ptr));
			}
		}

		template<class Rep, class Period, class Fn>
		inline void _post_connect_timeout_timer(std::chrono::duration<Rep, Period> duration,
			std::shared_ptr<derived_t> this_ptr, Fn&& fn)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if (!derive.io().strand().running_in_this_thread())
			{
				asio::post(derive.io().strand(), make_allocator(derive.wallocator(),
				[this, this_ptr = std::move(this_ptr), duration, fn = std::forward<Fn>(fn)]() mutable
				{
					this->_post_connect_timeout_timer(duration, std::move(this_ptr), std::move(fn));
				}));
				return;
			}

		#if defined(ASIO2_ENABLE_LOG)
			ASIO2_ASSERT(this->is_stop_connect_timeout_timer_called_ == false);
		#endif

			this->connect_error_code_.clear();
			this->connect_timeout_flag_.store(false);

			// reset the "canceled" flag to false, see reconnect_timer_cp.hpp -> _make_reconnect_timer
			this->connect_timer_canceled_.clear();

			this->connect_timeout_timer_.expires_after(duration);
			this->connect_timeout_timer_.async_wait(asio::bind_executor(derive.io().strand(),
			[this, self_ptr = std::move(this_ptr), f = std::forward<Fn>(fn)]
			(const error_code& ec) mutable
			{
				ASIO2_ASSERT((!ec) || ec == asio::error::operation_aborted);

			#if defined(ASIO2_ENABLE_LOG)
				if (ec && ec != asio::error::operation_aborted)
				{
					ASIO2_LOG(spdlog::level::info, "connect_timeout_timer error : [{}] {}", ec.value(), ec.message());
				}
			#endif

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
			// 2021-12-10 bug fix : when a client connected, the tcp_session::start will be 
			// called, and super::start will be called in tcp_session::start, so session::start
			// will be called, and session::start will call _post_connect_timeout_timer, but
			// session::start is not running in the io thread, so it will post a event, then
			// tcp_session::_handle_connect will be called, and tcp_session::_done_connect
			// will be called, and _stop_connect_timeout_timer will be called, but at this 
			// time, the _post_connect_timeout_timer in the session::start has't called yet,
			// this will cause the client to be fore disconnect with timeout error.
			// so we should ensure the _stop_connect_timeout_timer and _post_connect_timeout_timer
			// was called in the io thread.
			derived_t& derive = static_cast<derived_t&>(*this);

			if (!derive.io().strand().running_in_this_thread())
			{
				derive.post([this, ec]() mutable
				{
					this->_stop_connect_timeout_timer(ec);
				});
				return;
			}

		#if defined(ASIO2_ENABLE_LOG)
			this->is_stop_connect_timeout_timer_called_ = true;
		#endif

			error_code ec_ignore{};
			try
			{
				this->connect_error_code_ = ec;
				this->connect_timer_canceled_.test_and_set();
				this->connect_timeout_timer_.cancel(ec_ignore);
			}
			catch (system_error&) {}
			catch (std::exception&) {}
		}

		inline bool _is_connect_timeout() const noexcept
		{
			return this->connect_timeout_flag_.load();
		}

		inline asio::error_code _connect_error_code() noexcept
		{
			return this->connect_error_code_;
		}

	protected:
		asio::steady_timer                          connect_timeout_timer_;

		std::atomic_flag                            connect_timer_canceled_;

		std::chrono::steady_clock::duration         connect_timeout_         = std::chrono::seconds(5);

		asio::error_code                            connect_error_code_;

		/// Used to check the error_code is connection timeout or others.
		std::atomic_bool                            connect_timeout_flag_    = false;

	#if defined(ASIO2_ENABLE_LOG)
		bool                                        is_stop_connect_timeout_timer_called_ = false;
	#endif
	};
}

#endif // !__ASIO2_CONNECT_TIMEOUT_COMPONENT_HPP__
