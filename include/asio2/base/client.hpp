/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_CLIENT_HPP__
#define __ASIO2_CLIENT_HPP__

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
#include <asio2/base/impl/post_cp.hpp>
#include <asio2/base/impl/connect_timeout_cp.hpp>
#include <asio2/base/impl/event_queue_cp.hpp>
#include <asio2/base/impl/condition_event_cp.hpp>
#include <asio2/base/impl/reconnect_timer_cp.hpp>
#include <asio2/base/impl/send_cp.hpp>

#include <asio2/component/rdc/rdc_call_cp.hpp>
#include <asio2/component/socks/socks5_client_cp.hpp>

namespace asio2
{
	class client
	{
	public:
		inline constexpr static bool is_session() noexcept { return false; }
		inline constexpr static bool is_client () noexcept { return true ; }
		inline constexpr static bool is_server () noexcept { return false; }
	};
}

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;

	template<class derived_t, class args_t>
	class client_impl_t
		: public asio2::client
		, public object_t              <derived_t        >
		, public iopool_cp             <derived_t, args_t>
		, public io_context_cp         <derived_t, args_t>
		, public thread_id_cp          <derived_t, args_t>
		, public event_queue_cp        <derived_t, args_t>
		, public user_data_cp          <derived_t, args_t>
		, public connect_time_cp       <derived_t, args_t>
		, public alive_time_cp         <derived_t, args_t>
		, public socket_cp             <derived_t, args_t>
		, public connect_cp            <derived_t, args_t>
		, public shutdown_cp           <derived_t, args_t>
		, public close_cp              <derived_t, args_t>
		, public disconnect_cp         <derived_t, args_t>
		, public reconnect_timer_cp    <derived_t, args_t>
		, public user_timer_cp         <derived_t, args_t>
		, public connect_timeout_cp    <derived_t, args_t>
		, public send_cp               <derived_t, args_t>
		, public post_cp               <derived_t, args_t>
		, public condition_event_cp    <derived_t, args_t>
		, public rdc_call_cp           <derived_t, args_t>
		, public socks5_client_cp      <derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;

	public:
		using super = object_t     <derived_t        >;
		using self  = client_impl_t<derived_t, args_t>;

		using iopoolcp = iopool_cp <derived_t, args_t>;

		using args_type   = args_t;
		using key_type    = std::size_t;
		using buffer_type = typename args_t::buffer_t;

		using send_cp<derived_t, args_t>::send;
		using send_cp<derived_t, args_t>::async_send;

	public:
		/**
		 * @brief constructor
		 * @throws maybe throw exception "Too many open files" (exception code : 24)
		 * asio::error::no_descriptors - Too many open files
		 */
		template<class ThreadCountOrScheduler, class ...Args>
		explicit client_impl_t(
			std::size_t init_buf_size,
			std::size_t  max_buf_size,
			ThreadCountOrScheduler&& tcos
		)
			: super()
			, iopool_cp           <derived_t, args_t>(std::forward<ThreadCountOrScheduler>(tcos))
			, io_context_cp       <derived_t, args_t>(iopoolcp::_get_io(0))
			, event_queue_cp      <derived_t, args_t>()
			, user_data_cp        <derived_t, args_t>()
			, connect_time_cp     <derived_t, args_t>()
			, alive_time_cp       <derived_t, args_t>()
			, socket_cp           <derived_t, args_t>(iopoolcp::_get_io(0)->context())
			, connect_cp          <derived_t, args_t>()
			, shutdown_cp         <derived_t, args_t>()
			, close_cp            <derived_t, args_t>()
			, disconnect_cp       <derived_t, args_t>()
			, reconnect_timer_cp  <derived_t, args_t>()
			, user_timer_cp       <derived_t, args_t>()
			, connect_timeout_cp  <derived_t, args_t>()
			, send_cp             <derived_t, args_t>()
			, post_cp             <derived_t, args_t>()
			, condition_event_cp  <derived_t, args_t>()
			, rdc_call_cp         <derived_t, args_t>()
			, rallocator_()
			, wallocator_()
			, listener_  ()
			, buffer_    (init_buf_size, max_buf_size)
		{
		#if defined(ASIO2_ENABLE_LOG)
		#if defined(ASIO2_ALLOCATOR_STORAGE_SIZE)
			static_assert(decltype(rallocator_)::storage_size == ASIO2_ALLOCATOR_STORAGE_SIZE);
			static_assert(decltype(wallocator_)::storage_size == ASIO2_ALLOCATOR_STORAGE_SIZE);
		#endif
		#endif
		}

		/**
		 * @brief destructor
		 */
		~client_impl_t()
		{
		}

		/**
		 * @brief start the client
		 */
		inline bool start() noexcept
		{
			ASIO2_ASSERT(this->io_->running_in_this_thread());

			this->stopped_ = false;

			return true;
		}

		/**
		 * @brief async start the client
		 */
		inline bool async_start() noexcept
		{
			ASIO2_ASSERT(this->io_->running_in_this_thread());

			this->stopped_ = false;

			return true;
		}

		/**
		 * @brief stop the client
		 */
		inline void stop()
		{
			ASIO2_ASSERT(this->io_->running_in_this_thread());

			// can't use post, we need ensure when the derived stop is called, the chain
			// must be executed completed.
			this->derived().dispatch([this]() mutable
			{
				// close reconnect timer
				this->_stop_reconnect_timer();

				// close connect timeout timer
				this->_stop_connect_timeout_timer();

				// close user custom timers
				this->_dispatch_stop_all_timers();

				// close all posted timed tasks
				this->_dispatch_stop_all_timed_events();

				// close all async_events
				this->notify_all_condition_events();

				// can't use push event to close the socket, beacuse when used with websocket,
				// the websocket's async_close will be called, and the chain will passed into
				// the async_close, but the async_close will cause the chain interrupted, and
				// we don't know when the async_close will be completed, if another push event
				// was called during async_close executing, then here push event will after 
				// the another event in the queue.

				// clear recv buffer
				this->buffer().consume(this->buffer().size());

				// destroy user data, maybe the user data is self shared_ptr, if
				// don't destroy it, will cause loop reference.
				// read/write user data in other thread which is not the io_context
				// thread maybe cause crash.
				this->user_data_.reset();

				// destroy the ecs
				this->ecs_.reset();

				// 
				this->reset_life_id();

				//
				this->stopped_ = true;
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
			derive.listener_.clear();

			derive.destroy_iopool();
		}

		/**
		 * @brief check whether the client is started
		 */
		inline bool is_started() const
		{
			return (this->state_ == state_t::started && this->socket().is_open());
		}

		/**
		 * @brief check whether the client is stopped
		 */
		inline bool is_stopped() const
		{
			return (this->state_ == state_t::stopped && !this->socket().is_open() && this->stopped_);
		}

		/**
		 * @brief get this object hash key
		 */
		inline key_type hash_key() const noexcept
		{
			return reinterpret_cast<key_type>(this);
		}

		/**
		 * @brief get the buffer object reference
		 */
		inline buffer_wrap<buffer_type> & buffer() noexcept { return this->buffer_; }

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
		/**
		 * @brief get the recv/read allocator object reference
		 */
		inline auto & rallocator() noexcept { return this->rallocator_; }
		/**
		 * @brief get the send/write allocator object reference
		 */
		inline auto & wallocator() noexcept { return this->wallocator_; }

		inline listener_t                 & listener() noexcept { return this->listener_; }
		inline std::atomic<state_t>       & state   () noexcept { return this->state_;    }

		inline const char*                  life_id () noexcept { return this->life_id_.get(); }
		inline void                   reset_life_id () noexcept { this->life_id_ = std::make_unique<char>(); }

	protected:
		/// The memory to use for handler-based custom memory allocation. used fo recv/read.
		handler_memory<std::true_type , assizer<args_t>>   rallocator_;

		/// The memory to use for handler-based custom memory allocation. used fo send/write.
		handler_memory<std::false_type, assizer<args_t>>   wallocator_;

		/// listener 
		listener_t                                  listener_;

		/// buffer
		buffer_wrap<buffer_type>                    buffer_;

		/// state
		std::atomic<state_t>                        state_      = state_t::stopped;

		/// Remote call (rpc/rdc) response timeout.
		std::chrono::steady_clock::duration         rc_timeout_ = std::chrono::milliseconds(http_execute_timeout);

		/// the pointer of ecs_t
		std::shared_ptr<ecs_base>                   ecs_;

		/// client has two status:
		/// 1. completely stopped.
		/// 2. disconnected but not stopped, the timer or other event are running.
		bool                                        stopped_ = true;

		/// Whether the async_read... is called.
		bool                                        reading_ = false;

		/// Used to solve this problem:
		/// 
		/// client_ptr->bind_connect([client_ptr]()
		/// {
		///		client_ptr->async_send(...);
		/// });
		/// 
		/// client_ptr->post([client_ptr]()
		/// {
		/// 	client_ptr->stop();
		/// 	client_ptr->start(...);
		/// 
		/// 	client_ptr->stop();
		/// 	client_ptr->start(...);
		/// });
		/// 
		/// We wanted is :
		/// 
		/// stop  event
		/// start event 
		/// async send event
		/// stop  event
		/// start event
		/// async send event
		/// 
		/// but beacuse the async send will push event into the event queue, so the
		/// event queue will be like this:
		/// 
		/// stop  event
		/// start event 
		/// stop  event
		/// start event
		/// async send event
		/// async send event
		///
		/// the async send will be called twice for the last time started client.
		std::unique_ptr<char>                       life_id_ = std::make_unique<char>();

	#if defined(_DEBUG) || defined(DEBUG)
		std::atomic<int>                            post_send_counter_ = 0;
		std::atomic<int>                            post_recv_counter_ = 0;
	#endif
	};
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_CLIENT_HPP__
