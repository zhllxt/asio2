/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_TIMER_HPP__
#define __ASIO2_TIMER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <cstdint>
#include <memory>
#include <chrono>
#include <atomic>
#include <string>
#include <string_view>

#include <asio2/base/iopool.hpp>
#include <asio2/base/log.hpp>
#include <asio2/base/define.hpp>

#include <asio2/base/detail/object.hpp>
#include <asio2/base/detail/allocator.hpp>
#include <asio2/base/detail/util.hpp>

#include <asio2/base/impl/thread_id_cp.hpp>
#include <asio2/base/impl/user_timer_cp.hpp>
#include <asio2/base/impl/post_cp.hpp>
#include <asio2/base/impl/condition_event_cp.hpp>

namespace asio2::detail
{
	struct template_args_timer
	{
		static constexpr std::size_t allocator_storage_size = 256;
	};

	ASIO2_CLASS_FORWARD_DECLARE_BASE;

	template<class derived_t, class args_t = template_args_timer>
	class timer_impl_t
		: public object_t          <derived_t        >
		, public iopool_cp         <derived_t, args_t>
		, public thread_id_cp      <derived_t, args_t>
		, public user_timer_cp     <derived_t, args_t>
		, public post_cp           <derived_t, args_t>
		, public condition_event_cp<derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;

	public:
		using super = object_t    <derived_t        >;
		using self  = timer_impl_t<derived_t, args_t>;

		using iopoolcp = iopool_cp<derived_t, args_t>;

		using args_type = args_t;

		/**
		 * @brief constructor
		 */
		explicit timer_impl_t()
			: super()
			, iopool_cp         <derived_t, args_t>(1)
			, user_timer_cp     <derived_t, args_t>()
			, post_cp           <derived_t, args_t>()
			, condition_event_cp<derived_t, args_t>()
			, io_                                  (iopoolcp::_get_io(0))
		{
			this->start();
		}

		template<class Scheduler, std::enable_if_t<!std::is_integral_v<detail::remove_cvref_t<Scheduler>>, int> = 0>
		explicit timer_impl_t(Scheduler&& scheduler)
			: super()
			, iopool_cp         <derived_t, args_t>(std::forward<Scheduler>(scheduler))
			, user_timer_cp     <derived_t, args_t>()
			, post_cp           <derived_t, args_t>()
			, condition_event_cp<derived_t, args_t>()
			, io_                                  (iopoolcp::_get_io(0))
		{
		#if defined(ASIO2_ENABLE_LOG)
		#if defined(ASIO2_ALLOCATOR_STORAGE_SIZE)
			static_assert(decltype(wallocator_)::storage_size == ASIO2_ALLOCATOR_STORAGE_SIZE);
		#else
			static_assert(decltype(wallocator_)::storage_size == args_t::allocator_storage_size);
		#endif
		#endif

			this->start();
		}

		/**
		 * @brief destructor
		 */
		~timer_impl_t()
		{
			this->stop();
		}

		/**
		 * @brief start
		 */
		inline bool start()
		{
			derived_t& derive = this->derived();

			// if log is enabled, init the log first, otherwise when "Too many open files" error occurs,
			// the log file will be created failed too.
		#if defined(ASIO2_ENABLE_LOG)
			asio2::detail::get_logger();
		#endif

			bool ret = this->start_iopool(); // start the io_context pool

			if (ret)
			{
				derive.io_->regobj(&derive);

				derive.dispatch([&derive]() mutable
				{
					// init the running thread id 
					derive.io_->init_thread_id();
				});
			}

			return ret;
		}

		/**
		 * @brief stop
		 */
		inline void stop()
		{
			if (this->is_iopool_stopped())
				return;

			derived_t& derive = this->derived();

			derive.io_->unregobj(&derive);

			// close user custom timers
			this->stop_all_timers();

			// close all posted timed tasks
			this->stop_all_timed_tasks();

			// close all async_events
			this->notify_all_condition_events();

			// stop the io_context pool
			this->stop_iopool();
		}

		/**
		 * @brief destroy the content of all member variables, this is used for solve the memory leaks.
		 * After this function is called, this class object cannot be used again.
		 */
		inline void destroy()
		{
			derived_t& derive = this->derived();

			derive.io_.reset();

			derive.destroy_iopool();
		}

	public:
		/**
		 * @brief get the io object reference
		 */
		inline io_t & io() noexcept { return *(this->io_); }

		/**
		 * @brief get the io object reference
		 */
		inline io_t const& io() const noexcept { return *(this->io_); }

	protected:
		/**
		 * @brief get the recv/read allocator object reference
		 */
		inline auto & rallocator() noexcept { return this->wallocator_; }
		/**
		 * @brief get the send/write allocator object reference
		 */
		inline auto & wallocator() noexcept { return this->wallocator_; }

	protected:
		/// The io_context wrapper used to handle the accept event.
		std::shared_ptr<io_t>                              io_;

		/// The memory to use for handler-based custom memory allocation. used fo send/write.
		handler_memory<std::false_type, assizer<args_t>>   wallocator_;
	};
}

namespace asio2
{
	class timer : public detail::timer_impl_t<timer, detail::template_args_timer>
	{
	public:
		using detail::timer_impl_t<timer, detail::template_args_timer>::timer_impl_t;
	};
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_TIMER_HPP__
