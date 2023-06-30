/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_DISCONNECT_COMPONENT_HPP__
#define __ASIO2_DISCONNECT_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <future>
#include <utility>
#include <string_view>

#include <asio2/base/iopool.hpp>
#include <asio2/base/listener.hpp>

#include <asio2/base/impl/event_queue_cp.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t>
	class disconnect_cp
	{
	public:
		using self = disconnect_cp<derived_t, args_t>;

	public:
		/**
		 * @brief constructor
		 */
		disconnect_cp() noexcept {}

		/**
		 * @brief destructor
		 */
		~disconnect_cp() = default;

	protected:
		template<typename E = defer_event<void, derived_t>>
		inline void _do_disconnect(
			const error_code& ec, std::shared_ptr<derived_t> this_ptr, E chain = defer_event<void, derived_t>{})
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			derive.dispatch([&derive, ec, this_ptr = std::move(this_ptr), chain = std::move(chain)]() mutable
			{
				ASIO2_LOG_DEBUG("disconnect_cp::_do_disconnect: {} {}", ec.value(), ec.message());

				derive._do_shutdown(ec, std::move(this_ptr), std::move(chain));
			});
		}

		template<typename DeferEvent>
		inline void _post_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io_->running_in_this_thread());

			ASIO2_LOG_DEBUG("disconnect_cp::_post_disconnect: {} {}", ec.value(), ec.message());

			derive._handle_disconnect(ec, std::move(this_ptr), std::move(chain));
		}

		template<typename DeferEvent>
		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io_->running_in_this_thread());

			// we should wait for the async read functions returned.
			// the reading flag will be always false of udp session.
			if (derive.reading_)
			{
				derive._make_readend_timer(ec, std::move(this_ptr), std::move(chain));
			}
			else
			{
				derive._handle_readend(ec, std::move(this_ptr), std::move(chain));
			}
		}

		template<typename DeferEvent>
		inline void _handle_readend(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io_->running_in_this_thread());

			// at here the disconnect is completely finished.
			derive.disconnecting_ = false;

			if constexpr (args_t::is_session)
			{
				derive._do_stop(ec, std::move(this_ptr), std::move(chain));
			}
			else
			{
				detail::ignore_unused(ec, this_ptr, chain);
			}
		}

	protected:
		template<typename DeferEvent>
		inline void _make_readend_timer(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io_->running_in_this_thread());

			asio::dispatch(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, ec, this_ptr = std::move(this_ptr), chain = std::move(chain)]() mutable
			{
				derived_t& derive = static_cast<derived_t&>(*this);

				ASIO2_ASSERT(this->readend_timer_ == nullptr);

				if (this->readend_timer_)
				{
					this->readend_timer_->cancel();
				}

				this->readend_timer_ = std::make_shared<safe_timer>(derive.io_->context());

				derive._post_readend_timer(ec, std::move(this_ptr), std::move(chain), this->readend_timer_);
			}));
		}

		template<typename DeferEvent>
		inline void _post_readend_timer(
			const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain,
			std::shared_ptr<safe_timer> timer_ptr)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// a new timer is maked, this is the prev timer, so return directly.
			if (timer_ptr.get() != this->readend_timer_.get())
				return;

			safe_timer* ptimer = timer_ptr.get();

			ptimer->timer.expires_after(derive.get_disconnect_timeout());
			ptimer->timer.async_wait(
			[&derive, ec, this_ptr = std::move(this_ptr), chain = std::move(chain), timer_ptr = std::move(timer_ptr)]
			(const error_code& timer_ec) mutable
			{
				derive._handle_readend_timer(
					timer_ec, ec, std::move(this_ptr), std::move(chain), std::move(timer_ptr));
			});
		}

		template<typename DeferEvent>
		inline void _handle_readend_timer(
			const error_code& timer_ec,
			const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain,
			std::shared_ptr<safe_timer> timer_ptr)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT((!timer_ec) || timer_ec == asio::error::operation_aborted);

			// a new timer is maked, this is the prev timer, so return directly.
			if (timer_ptr.get() != this->readend_timer_.get())
				return;

			// member variable timer should't be empty
			if (!this->readend_timer_)
			{
				ASIO2_ASSERT(false);
				return;
			}

			// current timer is canceled by manual
			if (timer_ec == asio::error::operation_aborted || timer_ptr->canceled.test_and_set())
			{
				ASIO2_LOG_DEBUG("disconnect_cp::_handle_readend_timer: canceled");
			}
			// timeout
			else
			{
				ASIO2_LOG_DEBUG("disconnect_cp::_handle_readend_timer: timeout");
			}

			timer_ptr->canceled.clear();

			this->readend_timer_.reset();

			derive._handle_readend(ec, std::move(this_ptr), std::move(chain));
		}

		inline void _stop_readend_timer(std::shared_ptr<derived_t> this_ptr)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			asio::dispatch(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, this_ptr = std::move(this_ptr)]() mutable
			{
				if (this->readend_timer_)
				{
					this->readend_timer_->cancel();
				}
			}));
		}

	public:
		/**
		 * @brief get the disconnect timeout
		 */
		inline std::chrono::steady_clock::duration get_disconnect_timeout() noexcept
		{
			return this->disconnect_timeout_;
		}

		/**
		 * @brief set the disconnect timeout
		 */
		template<class Rep, class Period>
		inline derived_t& set_disconnect_timeout(std::chrono::duration<Rep, Period> timeout) noexcept
		{
			if (timeout > std::chrono::duration_cast<
				std::chrono::duration<Rep, Period>>((std::chrono::steady_clock::duration::max)()))
				this->disconnect_timeout_ = (std::chrono::steady_clock::duration::max)();
			else
				this->disconnect_timeout_ = timeout;
			return static_cast<derived_t&>(*this);
		}

	protected:
		std::chrono::steady_clock::duration         disconnect_timeout_ = std::chrono::seconds(30);

		/// 
		bool                                        disconnecting_ = false;

		std::shared_ptr<safe_timer>                 readend_timer_;
	};
}

#endif // !__ASIO2_DISCONNECT_COMPONENT_HPP__
