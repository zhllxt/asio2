/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_CONNECT_TIMEOUT_COMPONENT_HPP__
#define __ASIO2_CONNECT_TIMEOUT_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <chrono>

#include <asio2/base/iopool.hpp>
#include <asio2/base/log.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t>
	class connect_timeout_cp
	{
	public:
		/**
		 * @brief constructor
		 */
		connect_timeout_cp() = default;

		/**
		 * @brief destructor
		 */
		~connect_timeout_cp() = default;

		/**
		 * @brief get the connect timeout
		 */
		inline std::chrono::steady_clock::duration get_connect_timeout() const noexcept
		{
			return this->connect_timeout_;
		}

		/**
		 * @brief set the connect timeout
		 */
		template<class Rep, class Period>
		inline derived_t& set_connect_timeout(std::chrono::duration<Rep, Period> timeout) noexcept
		{
			if (timeout > std::chrono::duration_cast<
				std::chrono::duration<Rep, Period>>((std::chrono::steady_clock::duration::max)()))
				this->connect_timeout_ = (std::chrono::steady_clock::duration::max)();
			else
				this->connect_timeout_ = timeout;
			return static_cast<derived_t&>(*this);
		}

	protected:
		template<class Rep, class Period>
		inline void _make_connect_timeout_timer(
			std::shared_ptr<derived_t> this_ptr, std::chrono::duration<Rep, Period> duration)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			asio::dispatch(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, this_ptr = std::move(this_ptr), duration = std::move(duration)]() mutable
			{
				derived_t& derive = static_cast<derived_t&>(*this);

				if (this->connect_timeout_timer_)
				{
					this->connect_timeout_timer_->cancel();
				}

				this->connect_timeout_timer_ = std::make_shared<safe_timer>(derive.io_->context());

				derive._post_connect_timeout_timer(std::move(this_ptr), this->connect_timeout_timer_, duration);
			}));
		}

		template<class Rep, class Period>
		inline void _post_connect_timeout_timer(std::shared_ptr<derived_t> this_ptr,
			std::shared_ptr<safe_timer> timer_ptr, std::chrono::duration<Rep, Period> duration)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(this->is_stop_connect_timeout_timer_called_ == false);
		#endif

			// a new timer is maked, this is the prev timer, so return directly.
			if (timer_ptr.get() != this->connect_timeout_timer_.get())
				return;

			safe_timer* ptimer = timer_ptr.get();

			ptimer->timer.expires_after(duration);
			ptimer->timer.async_wait(
			[&derive, this_ptr = std::move(this_ptr), timer_ptr = std::move(timer_ptr)]
			(const error_code& ec) mutable
			{
				// bug fixed : 
				// note : after call derive._handle_connect_timeout_timer(ec, std::move(this_ptr)); 
				// the pointer of "this" can no longer be used immediately, beacuse when 
				// this_ptr's reference counter is 1, after call 
				// derive._handle_connect_timeout_timer(ec, std::move(this_ptr)); the this_ptr's 
				// object will be destroyed, then below code "this->..." will cause crash.
				derive._handle_connect_timeout_timer(ec, std::move(this_ptr), std::move(timer_ptr));

				// after call derive._handle_connect_timeout_timer(ec, std::move(this_ptr));
				// can't do it like below, beacuse "this" maybe deleted already.
				// this->...
			});
		}

		template<class D = derived_t>
		inline void _handle_connect_timeout_timer(
			const error_code& ec, std::shared_ptr<D> this_ptr, std::shared_ptr<safe_timer> timer_ptr)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT((!ec) || ec == asio::error::operation_aborted);

			// a new timer is maked, this is the prev timer, so return directly.
			if (timer_ptr.get() != this->connect_timeout_timer_.get())
				return;

			// member variable timer should't be empty
			if (!this->connect_timeout_timer_)
			{
				ASIO2_ASSERT(false);
				return;
			}

			this->connect_timeout_timer_.reset();

			// ec maybe zero when timer_canceled_ is true.
			if (ec == asio::error::operation_aborted || timer_ptr->canceled.test_and_set())
				return;

			if constexpr (D::is_session())
			{
				if (!ec)
				{
					derive._do_disconnect(asio::error::timed_out, std::move(this_ptr));
				}
				else
				{
					// should't go to here
					ASIO2_ASSERT(false);
					derive._do_disconnect(ec, std::move(this_ptr));
				}
			}
			else
			{
				// no errors indicating that the client connection timed out
				if (!ec)
				{
					// we close the socket, so the async_connect will returned 
					// with operation_aborted.
					error_code ec_ignore{};

					derive.socket().shutdown(asio::socket_base::shutdown_both, ec_ignore);
					derive.socket().cancel(ec_ignore);
					derive.socket().close(ec_ignore);

					ASIO2_ASSERT(!derive.socket().is_open());
				}
			}
		}

		inline void _stop_connect_timeout_timer()
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

			derive.dispatch([this]() mutable
			{
			#if defined(_DEBUG) || defined(DEBUG)
				this->is_stop_connect_timeout_timer_called_ = true;
			#endif

				if (this->connect_timeout_timer_)
				{
					this->connect_timeout_timer_->cancel();
				}
			});
		}

	protected:
		/// beacuse the connect timeout timer is used only when connect, so we use a pointer
		/// to reduce memory space occupied when running
		std::shared_ptr<safe_timer>                 connect_timeout_timer_;

		std::chrono::steady_clock::duration         connect_timeout_         = std::chrono::seconds(30);

	#if defined(_DEBUG) || defined(DEBUG)
		bool                                        is_stop_connect_timeout_timer_called_ = false;
	#endif
	};
}

#endif // !__ASIO2_CONNECT_TIMEOUT_COMPONENT_HPP__
