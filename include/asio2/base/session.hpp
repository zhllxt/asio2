/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
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
#include <functional>
#include <atomic>
#include <string>
#include <string_view>
#include <queue>
#include <any>
#include <future>
#include <tuple>
#include <map>
#include <unordered_map>
#include <type_traits>

#include <asio2/external/asio.hpp>
#include <asio2/external/magic_enum.hpp>

#include <asio2/base/iopool.hpp>
#include <asio2/base/error.hpp>
#include <asio2/base/log.hpp>
#include <asio2/base/listener.hpp>
#include <asio2/base/session_mgr.hpp>
#include <asio2/base/define.hpp>

#include <asio2/base/detail/object.hpp>
#include <asio2/base/detail/allocator.hpp>
#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/buffer_wrap.hpp>
#include <asio2/base/detail/condition_wrap.hpp>

#include <asio2/base/component/thread_id_cp.hpp>
#include <asio2/base/component/connect_time_cp.hpp>
#include <asio2/base/component/alive_time_cp.hpp>
#include <asio2/base/component/user_data_cp.hpp>
#include <asio2/base/component/socket_cp.hpp>
#include <asio2/base/component/connect_cp.hpp>
#include <asio2/base/component/disconnect_cp.hpp>
#include <asio2/base/component/user_timer_cp.hpp>
#include <asio2/base/component/silence_timer_cp.hpp>
#include <asio2/base/component/post_cp.hpp>
#include <asio2/base/component/connect_timeout_cp.hpp>
#include <asio2/base/component/event_queue_cp.hpp>
#include <asio2/base/component/async_event_cp.hpp>
#include <asio2/base/component/send_cp.hpp>

#include <asio2/ecs/rdc/rdc_call_cp.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;

	template<class derived_t, class args_t>
	class session_impl_t
		: public object_t              <derived_t        >
		, public thread_id_cp          <derived_t, args_t>
		, public event_queue_cp        <derived_t, args_t>
		, public user_data_cp          <derived_t, args_t>
		, public connect_time_cp       <derived_t, args_t>
		, public alive_time_cp         <derived_t, args_t>
		, public socket_cp             <derived_t, args_t>
		, public connect_cp            <derived_t, args_t>
		, public disconnect_cp         <derived_t, args_t>
		, public user_timer_cp         <derived_t, args_t>
		, public silence_timer_cp      <derived_t, args_t>
		, public connect_timeout_cp    <derived_t, args_t>
		, public send_cp               <derived_t, args_t>
		, public post_cp               <derived_t, args_t>
		, public async_event_cp        <derived_t, args_t>
		, public rdc_call_cp           <derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;

	public:
		using super       = object_t      <derived_t        >;
		using self        = session_impl_t<derived_t, args_t>;

		using buffer_type = typename args_t::buffer_t;

		using send_cp<derived_t, args_t>::send;
		using send_cp<derived_t, args_t>::async_send;

	public:
		/**
		 * @constructor
		 * maybe throw exception "Too many open files" (exception code : 24)
		 * asio::error::no_descriptors - Too many open files
		 */
		template<class ...Args>
		explicit session_impl_t(
			session_mgr_t<derived_t> & sessions,
			listener_t               & listener,
			io_t                     & rwio,
			std::size_t                init_buf_size,
			std::size_t                max_buf_size,
			Args&&...                  args
		)
			: super()
			, event_queue_cp      <derived_t, args_t>()
			, user_data_cp        <derived_t, args_t>()
			, connect_time_cp     <derived_t, args_t>()
			, alive_time_cp       <derived_t, args_t>()
			, socket_cp           <derived_t, args_t>(std::forward<Args>(args)...)
			, connect_cp          <derived_t, args_t>()
			, disconnect_cp       <derived_t, args_t>()
			, user_timer_cp       <derived_t, args_t>()
			, silence_timer_cp    <derived_t, args_t>(rwio)
			, connect_timeout_cp  <derived_t, args_t>(rwio)
			, send_cp             <derived_t, args_t>()
			, post_cp             <derived_t, args_t>()
			, async_event_cp      <derived_t, args_t>()
			, rdc_call_cp         <derived_t, args_t>()
			, sessions_(sessions)
			, listener_(listener)
			, io_      (rwio)
			, buffer_  (init_buf_size, max_buf_size)
		{
		}

		/**
		 * @destructor
		 */
		~session_impl_t()
		{
			// Previously "counter_ptr_.reset()" was in the "stop" function, I don't remember
			// why I put "counter_ptr_.reset()" in "stop" function before, but now, i find 
			// some problem if put "counter_ptr_.reset()" in "stop" function, It looks like 
			// this:
			// when use async rpc user function, when code run to 
			// "asio::dispatch(caller->io().strand(), make_allocator(caller->wallocator(),"
			// ( the code is in the rpc_invoker.hpp, line 405 ), the server maybe stopped 
			// already ( beacuse server.stop will call all session's stop, and session.stop
			// will call "counter_ptr_.reset()", so the server's counter_ptr_'s destructor
			// can be called, so the iopool.stop can be returned in the server's stop function)
			// after the iopool is stopped, the all io_context will be invalid, so when the 
			// rpc_invoker call "caller->io().strand()", it will be crashed.
			// destroy the counter
			this->counter_ptr_.reset();
		}

	protected:
		/**
		 * @function : start session
		 */
		inline void start()
		{
			if (!this->io_.strand().running_in_this_thread())
			{
				this->derived().post([this]() mutable
				{
					this->start();
				});
				return;
			}

			// init the running thread id 
			if (this->derived().io().get_thread_id() == std::thread::id{})
				this->derived().io().init_thread_id();

			// start the timer of check connect timeout
			this->derived()._post_connect_timeout_timer(
				this->connect_timeout_, this->derived().selfptr());
		}

	public:
		/**
		 * @function : stop session
		 * note : this function must be noblocking,if it's blocking,
		 *        will cause circle lock in session_mgr::stop function
		 */
		inline void stop()
		{
			ASIO2_ASSERT(this->io_.strand().running_in_this_thread());

			if (!this->io_.strand().running_in_this_thread())
			{
				this->derived().post([this]() mutable
				{
					this->stop();
				});
				return;
			}

			// close silence timer
			this->_stop_silence_timer();

			// close connect timeout timer
			this->_stop_connect_timeout_timer(asio::error::operation_aborted);

			// close user custom timers
			this->stop_all_timers();

			// close all posted timed tasks
			this->stop_all_timed_tasks();

			// close all async_events
			this->notify_all_events();

			// destroy user data, maybe the user data is self shared_ptr, 
			// if don't destroy it, will cause loop refrence.
			this->user_data_.reset();
		}

		/**
		 * @function : check whether the session is started
		 */
		inline bool is_started() const
		{
			return (this->state_ == state_t::started && this->socket_.lowest_layer().is_open());
		}

		/**
		 * @function : check whether the session is stopped
		 */
		inline bool is_stopped() const
		{
			return (this->state_ == state_t::stopped && !this->socket_.lowest_layer().is_open());
		}

		/**
		 * @function : get the buffer object refrence
		 */
		inline buffer_wrap<buffer_type> & buffer() noexcept
		{
			return this->buffer_;
		}

		/**
		 * @function : get the io object refrence
		 */
		inline io_t & io() noexcept
		{
			return this->io_;
		}

		/**
		 * @function : set the default remote call timeout for rpc/rdc
		 */
		template<class Rep, class Period>
		inline derived_t & set_default_timeout(std::chrono::duration<Rep, Period> duration) noexcept
		{
			this->rc_timeout_ = duration;
			return (this->derived());
		}

		/**
		 * @function : get the default remote call timeout for rpc/rdc
		 */
		inline std::chrono::steady_clock::duration get_default_timeout() noexcept
		{
			return this->rc_timeout_;
		}

	protected:
		inline session_mgr_t<derived_t> & sessions() noexcept { return this->sessions_; }
		inline listener_t               & listener() noexcept { return this->listener_; }
		inline std::atomic<state_t>     & state   () noexcept { return this->state_;    }

		inline constexpr static bool is_session() noexcept { return true ; }
		inline constexpr static bool is_client () noexcept { return false; }
		inline constexpr static bool is_server () noexcept { return false; }

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

		/// The io (include io_context and strand) used to handle the recv/send event.
		io_t                               & io_;

		/// buffer
		buffer_wrap<buffer_type>             buffer_;

		/// use to check whether the user call stop in the listener
		std::atomic<state_t>                 state_        = state_t::stopped;

		/// Is it successful to insert the current session to the session map
		bool						         in_sessions_  = false;

		/// use this to ensure that server stop only after all sessions are closed
		std::shared_ptr<void>                counter_ptr_;

		/// Remote call (rpc/rdc) response timeout.
		std::chrono::steady_clock::duration  rc_timeout_   = std::chrono::milliseconds(http_execute_timeout);
	};
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_SESSION_HPP__
