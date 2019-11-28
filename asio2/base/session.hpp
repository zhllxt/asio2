/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_SESSION_HPP__
#define __ASIO2_SESSION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

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
#include <unordered_map>
#include <type_traits>

#include <asio2/base/selector.hpp>
#include <asio2/base/iopool.hpp>
#include <asio2/base/error.hpp>
#include <asio2/base/listener.hpp>
#include <asio2/base/session_mgr.hpp>

#include <asio2/base/detail/object.hpp>
#include <asio2/base/detail/allocator.hpp>
#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/buffer_wrap.hpp>

#include <asio2/base/component/connect_time_cp.hpp>
#include <asio2/base/component/active_time_cp.hpp>
#include <asio2/base/component/user_data_cp.hpp>
#include <asio2/base/component/socket_cp.hpp>
#include <asio2/base/component/user_timer_cp.hpp>
#include <asio2/base/component/silence_timer_cp.hpp>
#include <asio2/base/component/post_cp.hpp>
#include <asio2/base/component/send_cp.hpp>
#include <asio2/base/component/connect_timeout_cp.hpp>

namespace asio2::detail
{
	template<class derived_t, class socket_t, class buffer_t>
	class session_impl_t
		: public object_t<derived_t>
		, public user_data_cp<derived_t>
		, public connect_time_cp<derived_t>
		, public active_time_cp<derived_t>
		, public socket_cp<derived_t, socket_t>
		, public user_timer_cp<derived_t, true>
		, public silence_timer_cp<derived_t, true>
		, public connect_timeout_cp<derived_t, true>
		, public send_cp<derived_t, true>
		, public post_cp<derived_t>
	{
		template <class, bool>         friend class user_timer_cp;
		template <class, bool>         friend class silence_timer_cp;
		template <class, bool>         friend class connect_timeout_cp;
		template <class, bool>         friend class send_queue_cp;
		template <class, bool>         friend class send_cp;
		template <class>               friend class post_cp;

	public:
		using self = session_impl_t<derived_t, socket_t, buffer_t>;
		using super = object_t<derived_t>;
		using buffer_type = buffer_t;
		using send_cp<derived_t, true>::send;

		/**
		 * @constructor
		 */
		template<class ...Args>
		explicit session_impl_t(
			session_mgr_t<derived_t> & sessions,
			listener_t & listener,
			io_t & rwio,
			std::size_t init_buffer_size,
			std::size_t max_buffer_size,
			Args&&... args
		)
			: super()
			, user_data_cp<derived_t>()
			, connect_time_cp<derived_t>()
			, active_time_cp<derived_t>()
			, socket_cp<derived_t, socket_t>(std::forward<Args>(args)...)
			, user_timer_cp<derived_t, true>(rwio)
			, silence_timer_cp<derived_t, true>(rwio)
			, connect_timeout_cp<derived_t, true>(rwio)
			, send_cp<derived_t, true>(rwio)
			, post_cp<derived_t>()
			, sessions_(sessions)
			, listener_(listener)
			, io_(rwio)
			, buffer_(init_buffer_size, max_buffer_size)
		{
		}

		/**
		 * @destructor
		 */
		~session_impl_t()
		{
		}

	protected:
		/**
		 * @function : start session
		 */
		inline void start()
		{
			if (!this->io_.strand().running_in_this_thread())
				return asio::post(this->io_.strand(), std::bind(&self::start, this->shared_from_this()));

			// start the timer of check connect timeout
			this->derived()._post_timeout_timer(this->connect_timeout_, this->shared_from_this());
		}

	public:
		/**
		 * @function : stop session
		 * note : this function must be noblocking,if it's blocking,will cause circle lock in session_mgr::stop function
		 */
		inline void stop()
		{
			if (!this->io_.strand().running_in_this_thread())
				return asio::post(this->io_.strand(),
					std::bind(&self::stop, this->shared_from_this()));

			// close silence timer
			this->_stop_silence_timer();

			// close timeout timer
			this->_stop_timeout_timer();

			// close user custom timers
			this->stop_all_timers();

			// destroy user data, maybe the user data is self shared_ptr, if don't destroy it, will cause loop refrence.
			this->user_data_.reset();

			// destroy the counter
			this->counter_ptr_.reset();
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
		inline buffer_wrap<buffer_t> & buffer()
		{
			return this->buffer_;
		}

		/**
		 * @function : get the io object refrence
		 */
		inline io_t & io()
		{
			return this->io_;
		}

	protected:
		inline session_mgr_t<derived_t> & sessions() { return this->sessions_; }
		inline listener_t               & listener() { return this->listener_; }
		inline std::atomic<state_t>     & state()    { return this->state_;    }
		inline std::shared_ptr<derived_t> selfptr()  { return this->derived().shared_from_this(); }

	protected:
		/// asio::strand ,used to ensure socket multi thread safe,we must ensure that only one operator
		/// can operate the same socket at the same time,and strand can enuser that the event will
		/// be processed in the order of post, eg : strand.post(1);strand.post(2); the 2 will processed
		/// certaion after the 1,if 1 is block,the 2 won't be processed,util the 1 is processed completed
		/// more details see : http://bbs.csdn.net/topics/390931471

		/// session_mgr
		session_mgr_t<derived_t>  & sessions_;

		/// listener
		listener_t                & listener_;

		/// The io (include io_context and strand) used to handle the recv/send event.
		io_t                      & io_;

		/// buffer
		buffer_wrap<buffer_t>       buffer_;

		/// use to check whether the user call stop in the listener
		std::atomic<state_t>        state_ = state_t::stopped;

		/// Is it successful to insert the current session to the session map
		bool						in_sessions = false;

		/// use this to ensure that server stop only after all sessions are closed
		std::shared_ptr<void>       counter_ptr_;
	};
}

#endif // !__ASIO2_SESSION_HPP__
