/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_SESSION_HPP__
#define __ASIO2_SESSION_HPP__

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
#include <asio2/base/listener.hpp>
#include <asio2/base/session_mgr.hpp>
#include <asio2/base/define.hpp>

#include <asio2/base/detail/object.hpp>
#include <asio2/base/detail/allocator.hpp>
#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/buffer_wrap.hpp>
#include <asio2/base/detail/ecs.hpp>

#include <asio2/base/impl/io_context_cp.hpp>
#include <asio2/base/impl/thread_id_cp.hpp>
#include <asio2/base/impl/connect_time_cp.hpp>
#include <asio2/base/impl/alive_time_cp.hpp>
#include <asio2/base/impl/user_data_cp.hpp>
#include <asio2/base/impl/socket_cp.hpp>
#include <asio2/base/impl/connect_cp.hpp>
#include <asio2/base/impl/shutdown_cp.hpp>
#include <asio2/base/impl/close_cp.hpp>
#include <asio2/base/impl/disconnect_cp.hpp>
#include <asio2/base/impl/user_timer_cp.hpp>
#include <asio2/base/impl/silence_timer_cp.hpp>
#include <asio2/base/impl/post_cp.hpp>
#include <asio2/base/impl/connect_timeout_cp.hpp>
#include <asio2/base/impl/event_queue_cp.hpp>
#include <asio2/base/impl/condition_event_cp.hpp>
#include <asio2/base/impl/send_cp.hpp>

#include <asio2/component/rdc/rdc_call_cp.hpp>

namespace asio2
{
	class session
	{
	public:
		inline constexpr static bool is_session() noexcept { return true ; }
		inline constexpr static bool is_client () noexcept { return false; }
		inline constexpr static bool is_server () noexcept { return false; }
	};
}

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;

	template<class derived_t, class args_t>
	class session_impl_t
		: public asio2::session
		, public object_t              <derived_t        >
		, public thread_id_cp          <derived_t, args_t>
		, public io_context_cp         <derived_t, args_t>
		, public event_queue_cp        <derived_t, args_t>
		, public user_data_cp          <derived_t, args_t>
		, public connect_time_cp       <derived_t, args_t>
		, public alive_time_cp         <derived_t, args_t>
		, public socket_cp             <derived_t, args_t>
		, public connect_cp            <derived_t, args_t>
		, public shutdown_cp           <derived_t, args_t>
		, public close_cp              <derived_t, args_t>
		, public disconnect_cp         <derived_t, args_t>
		, public user_timer_cp         <derived_t, args_t>
		, public silence_timer_cp      <derived_t, args_t>
		, public connect_timeout_cp    <derived_t, args_t>
		, public send_cp               <derived_t, args_t>
		, public post_cp               <derived_t, args_t>
		, public condition_event_cp    <derived_t, args_t>
		, public rdc_call_cp           <derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;

	public:
		using super       = object_t      <derived_t        >;
		using self        = session_impl_t<derived_t, args_t>;

		using args_type   = args_t;
		using buffer_type = typename args_t::buffer_t;

		using send_cp<derived_t, args_t>::send;
		using send_cp<derived_t, args_t>::async_send;

	public:
		/**
		 * @brief constructor
		 * @throws maybe throw exception "Too many open files" (exception code : 24)
		 * asio::error::no_descriptors - Too many open files
		 */
		template<class ...Args>
		explicit session_impl_t(
			session_mgr_t<derived_t> & sessions,
			listener_t               & listener,
			std::shared_ptr<io_t>      rwio,
			std::size_t                init_buf_size,
			std::size_t                max_buf_size,
			Args&&...                  args
		)
			: super()
			, thread_id_cp        <derived_t, args_t>()
			, io_context_cp       <derived_t, args_t>(std::move(rwio))
			, event_queue_cp      <derived_t, args_t>()
			, user_data_cp        <derived_t, args_t>()
			, connect_time_cp     <derived_t, args_t>()
			, alive_time_cp       <derived_t, args_t>()
			, socket_cp           <derived_t, args_t>(std::forward<Args>(args)...)
			, connect_cp          <derived_t, args_t>()
			, shutdown_cp         <derived_t, args_t>()
			, close_cp            <derived_t, args_t>()
			, disconnect_cp       <derived_t, args_t>()
			, user_timer_cp       <derived_t, args_t>()
			, silence_timer_cp    <derived_t, args_t>()
			, connect_timeout_cp  <derived_t, args_t>()
			, send_cp             <derived_t, args_t>()
			, post_cp             <derived_t, args_t>()
			, condition_event_cp  <derived_t, args_t>()
			, rdc_call_cp         <derived_t, args_t>()
			, sessions_(sessions)
			, listener_(listener)
			, buffer_  (init_buf_size, max_buf_size)
		{
		}

		/**
		 * @brief destructor
		 */
		~session_impl_t()
		{
		}

	protected:
		/**
		 * @brief start session
		 */
		inline void start()
		{
			this->derived().dispatch([this]() mutable
			{
				// init the running thread id 
				this->derived().io_->init_thread_id();

				// start the timer of check connect timeout
				this->derived()._make_connect_timeout_timer(
					this->derived().selfptr(), this->connect_timeout_);
			});
		}

	public:
		/**
		 * @brief stop session
		 * note : this function must be noblocking,if it's blocking,
		 *        will cause circle lock in session_mgr::stop function
		 */
		inline void stop()
		{
			ASIO2_ASSERT(this->io_->running_in_this_thread());

			// can't use post, we need ensure when the derived stop is called, the chain
			// must be executed completed.
			this->derived().dispatch([this]() mutable
			{
				// close silence timer
				this->_stop_silence_timer();

				// close connect timeout timer
				this->_stop_connect_timeout_timer();

				// close user custom timers
				this->_dispatch_stop_all_timers();

				// close all posted timed tasks
				this->_dispatch_stop_all_timed_events();

				// close all async_events
				this->notify_all_condition_events();

				// clear recv buffer
				this->buffer().consume(this->buffer().size());

				// destroy user data, maybe the user data is self shared_ptr, 
				// if don't destroy it, will cause loop reference.
				// read/write user data in other thread which is not the io_context 
				// thread maybe cause crash.
				this->user_data_.reset();

				// destroy the ecs, the user maybe saved the session ptr in the match role init
				// function, so we must destroy ecs, otherwise the server will can't be exited
				// forever.
				this->ecs_.reset();

				// 
				this->reset_life_id();

				// 
				this->counter_ptr_.reset();

				ASIO2_ASSERT(this->sessions_.find(this->derived().hash_key()) == nullptr);
			});
		}

		/**
		 * @brief destroy the content of all member variables, this is used for solve the memory leaks.
		 * After this function is called, this class object cannot be used again.
		 */
		inline void destroy()
		{
			derived_t& derive = this->derived();

			derive.socket_.reset();
			derive.io_.reset();
		}

		/**
		 * @brief check whether the session is started
		 */
		inline bool is_started() const
		{
			return (this->state_ == state_t::started && this->socket().is_open());
		}

		/**
		 * @brief check whether the session is stopped
		 */
		inline bool is_stopped() const
		{
			return (this->state_ == state_t::stopped && !this->socket().is_open());
		}

		/**
		 * @brief get the buffer object reference
		 */
		inline buffer_wrap<buffer_type> & buffer() noexcept
		{
			return this->buffer_;
		}

		/**
		 * @brief set the default remote call timeout for rpc/rdc
		 */
		template<class Rep, class Period>
		inline derived_t & set_default_timeout(std::chrono::duration<Rep, Period> duration) noexcept
		{
			this->rc_timeout_ = duration;
			return (this->derived());
		}

		/**
		 * @brief get the default remote call timeout for rpc/rdc
		 */
		inline std::chrono::steady_clock::duration get_default_timeout() const noexcept
		{
			return this->rc_timeout_;
		}

	protected:
		inline session_mgr_t<derived_t> & sessions() noexcept { return this->sessions_; }
		inline listener_t               & listener() noexcept { return this->listener_; }
		inline std::atomic<state_t>     & state   () noexcept { return this->state_;    }

		inline constexpr bool             life_id () noexcept { return true; }
		inline constexpr void       reset_life_id () noexcept { }

	protected:
		/// asio::strand ,used to ensure socket multi thread safe,we must ensure that only one operator
		/// can operate the same socket at the same time,and strand can enuser that the event will
		/// be processed in the order of post, eg : strand.post(1);strand.post(2); the 2 will processed
		/// certaion after the 1,if 1 is block,the 2 won't be processed,util the 1 is processed completed
		/// more details see : http://bbs.csdn.net/topics/390931471

		/// session_mgr
		session_mgr_t<derived_t>           & sessions_;

		/// listener
		listener_t                         & listener_;

		/// buffer
		buffer_wrap<buffer_type>             buffer_;

		/// use to check whether the user call stop in the listener
		std::atomic<state_t>                 state_        = state_t::stopped;

		/// use this to ensure that server stop only after all sessions are closed
		std::shared_ptr<void>                counter_ptr_;

		/// Remote call (rpc/rdc) response timeout.
		std::chrono::steady_clock::duration  rc_timeout_   = std::chrono::milliseconds(http_execute_timeout);

		/// the pointer of ecs_t
		std::shared_ptr<ecs_base>            ecs_;

		/// Whether the async_read... is called.
		bool                                 reading_ = false;

	#if defined(_DEBUG) || defined(DEBUG)
		std::atomic<int>                     post_send_counter_ = 0;
		std::atomic<int>                     post_recv_counter_ = 0;
	#endif
	};
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_SESSION_HPP__
