/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_SERVER_HPP__
#define __ASIO2_SERVER_HPP__

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
#include <asio2/base/impl/user_data_cp.hpp>
#include <asio2/base/impl/user_timer_cp.hpp>
#include <asio2/base/impl/post_cp.hpp>
#include <asio2/base/impl/event_queue_cp.hpp>
#include <asio2/base/impl/condition_event_cp.hpp>

namespace asio2
{
	class server
	{
	public:
		inline constexpr static bool is_session() noexcept { return false; }
		inline constexpr static bool is_client () noexcept { return false; }
		inline constexpr static bool is_server () noexcept { return true ; }
	};
}

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;

	template<class derived_t, class session_t>
	class server_impl_t
		: public asio2::server
		, public object_t          <derived_t>
		, public iopool_cp         <derived_t>
		, public io_context_cp     <derived_t>
		, public thread_id_cp      <derived_t>
		, public event_queue_cp    <derived_t>
		, public user_data_cp      <derived_t>
		, public user_timer_cp     <derived_t>
		, public post_cp           <derived_t>
		, public condition_event_cp<derived_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;

	public:
		using super = object_t     <derived_t>;
		using self  = server_impl_t<derived_t, session_t>;

		using iopoolcp = iopool_cp <derived_t>;

		using args_type = typename session_t::args_type;
		using key_type = std::size_t;

	public:
		/**
		 * @brief constructor
		 */
		template<class ThreadCountOrScheduler>
		explicit server_impl_t(ThreadCountOrScheduler&& tcos)
			: object_t          <derived_t>()
			, iopool_cp         <derived_t>(std::forward<ThreadCountOrScheduler>(tcos))
			, io_context_cp     <derived_t>(iopoolcp::_get_io(0))
			, thread_id_cp      <derived_t>()
			, event_queue_cp    <derived_t>()
			, user_data_cp      <derived_t>()
			, user_timer_cp     <derived_t>()
			, post_cp           <derived_t>()
			, condition_event_cp<derived_t>()
			, rallocator_()
			, wallocator_()
			, listener_  ()
			, sessions_  (this->io_, this->state_)
		{
		}

		/**
		 * @brief destructor
		 */
		~server_impl_t()
		{
		}

		/**
		 * @brief start the server
		 */
		inline bool start() noexcept
		{
			ASIO2_ASSERT(this->io_->running_in_this_thread());

			return true;
		}

		/**
		 * @brief stop the server
		 */
		inline void stop()
		{
			ASIO2_ASSERT(this->io_->running_in_this_thread());

			// can't use post, we need ensure when the derived stop is called, the chain
			// must be executed completed.
			this->derived().dispatch([this]() mutable
			{
				// close user custom timers
				this->_dispatch_stop_all_timers();

				// close all posted timed tasks
				this->_dispatch_stop_all_timed_events();

				// close all async_events
				this->notify_all_condition_events();

				// destroy user data, maybe the user data is self shared_ptr, 
				// if don't destroy it, will cause loop reference.
				// read/write user data in other thread which is not the io_context
				// thread maybe cause crash.
				this->user_data_.reset();

				// destroy the ecs
				this->ecs_.reset();
			});
		}

		/**
		 * @brief destroy the content of all member variables, this is used for solve the memory leaks.
		 * After this function is called, this class object cannot be used again.
		 */
		inline void destroy()
		{
			derived_t& derive = this->derived();

			derive.io_.reset();
			derive.listener_.clear();

			derive.destroy_iopool();
		}

		/**
		 * @brief check whether the server is started 
		 */
		inline bool is_started() const noexcept
		{
			return (this->state_ == state_t::started);
		}

		/**
		 * @brief check whether the server is stopped
		 */
		inline bool is_stopped() const noexcept
		{
			return (this->state_ == state_t::stopped);
		}

		/**
		 * @brief get this object hash key
		 */
		inline key_type hash_key() const noexcept
		{
			return reinterpret_cast<key_type>(this);
		}

		/**
		 * @brief Asynchronous send data for each session
		 * supporting multi data formats,see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * PodType * : async_send("abc");
		 * PodType (&data)[N] : double m[10]; async_send(m);
		 * std::array<PodType, N> : std::array<int,10> m; async_send(m);
		 * std::vector<PodType, Allocator> : std::vector<float> m; async_send(m);
		 * std::basic_string<Elem, Traits, Allocator> : std::string m; async_send(m);
		 */
		template<class T>
		inline derived_t & async_send(const T& data)
		{
			this->sessions_.quick_for_each([&data](std::shared_ptr<session_t>& session_ptr) mutable
			{
				session_ptr->async_send(data);
			});
			return this->derived();
		}

		/**
		 * @brief Asynchronous send data for each session
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * PodType * : async_send("abc");
		 */
		template<class CharT, class Traits = std::char_traits<CharT>>
		inline typename std::enable_if_t<detail::is_char_v<CharT>, derived_t&> async_send(CharT* s)
		{
			return this->async_send(s, s ? Traits::length(s) : 0);
		}

		/**
		 * @brief Asynchronous send data for each session
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * PodType (&data)[N] : double m[10]; async_send(m,5);
		 */
		template<class CharT, class SizeT>
		inline typename std::enable_if_t<std::is_integral_v<detail::remove_cvref_t<SizeT>>, derived_t&>
			async_send(CharT* s, SizeT count)
		{
			if (s)
			{
				this->sessions_.quick_for_each([s, count](std::shared_ptr<session_t>& session_ptr) mutable
				{
					session_ptr->async_send(s, count);
				});
			}
			return this->derived();
		}

	public:
		/**
		 * @brief get the acceptor reference, derived classes must override this function
		 */
		inline auto & acceptor() noexcept { return this->derived().acceptor(); }

		/**
		 * @brief get the acceptor reference, derived classes must override this function
		 */
		inline auto const& acceptor() const noexcept { return this->derived().acceptor(); }

		/**
		 * @brief get the listen address, same as get_listen_address
		 */
		inline std::string listen_address() const noexcept
		{
			return this->get_listen_address();
		}

		/**
		 * @brief get the listen address
		 */
		inline std::string get_listen_address() const noexcept
		{
			try
			{
				return this->acceptor().local_endpoint().address().to_string();
			}
			catch (system_error & e) { set_last_error(e); }
			return std::string();
		}

		/**
		 * @brief get the listen port, same as get_listen_port
		 */
		inline unsigned short listen_port() const noexcept
		{
			return this->get_listen_port();
		}

		/**
		 * @brief get the listen port
		 */
		inline unsigned short get_listen_port() const noexcept
		{
			return this->acceptor().local_endpoint(get_last_error()).port();
		}

		/**
		 * @brief get connected session count, same as get_session_count
		 */
		inline std::size_t session_count() const noexcept { return this->get_session_count(); }

		/**
		 * @brief get connected session count
		 */
		inline std::size_t get_session_count() const noexcept { return this->sessions_.size(); }

		/**
		 * @brief Applies the given function object fn for each session.
		 * @param fn - The handler to be called for each session.
		 * Function signature :
		 * void(std::shared_ptr<asio2::xxx_session>& session_ptr)
		 */
		template<class Fun>
		inline derived_t & foreach_session(Fun&& fn)
		{
			this->sessions_.for_each(std::forward<Fun>(fn));
			return this->derived();
		}

		/**
		 * @brief find the session by session's hash key
		 */
		template<class KeyType>
		inline std::shared_ptr<session_t> find_session(const KeyType& key)
		{
			return this->sessions_.find(key);
		}

		/**
		 * @brief find the session by user custom role
		 * @param fn - The handler to be called when search the session.
		 * Function signature :
		 * bool(std::shared_ptr<asio2::xxx_session>& session_ptr)
		 * @return std::shared_ptr<asio2::xxx_session>
		 */
		template<class Fun>
		inline std::shared_ptr<session_t> find_session_if(Fun&& fn)
		{
			return std::shared_ptr<session_t>(this->sessions_.find_if(std::forward<Fun>(fn)));
		}

	protected:
		/**
		 * @brief get the recv/read allocator object reference
		 */
		inline auto & rallocator() noexcept { return this->rallocator_; }
		/**
		 * @brief get the send/write/post allocator object reference
		 */
		inline auto & wallocator() noexcept { return this->wallocator_; }

		inline session_mgr_t<session_t> & sessions() noexcept { return this->sessions_; }
		inline listener_t               & listener() noexcept { return this->listener_; }
		inline std::atomic<state_t>     & state   () noexcept { return this->state_;    }

	protected:
		// The memory to use for handler-based custom memory allocation. used for acceptor.
		handler_memory<std::true_type , assizer<args_type>>     rallocator_;

		/// The memory to use for handler-based custom memory allocation. used fo send/write/post.
		handler_memory<std::false_type, assizer<args_type>>     wallocator_;

		/// listener
		listener_t                                  listener_;

		/// state
		std::atomic<state_t>                        state_ = state_t::stopped;

		/// session_mgr
		session_mgr_t<session_t>                    sessions_;

		/// use this to ensure that server stop only after all sessions are closed
		std::shared_ptr<void>                       counter_ptr_;

		/// the pointer of ecs_t
		std::shared_ptr<ecs_base>                   ecs_;

	#if defined(_DEBUG) || defined(DEBUG)
		std::atomic<int>                            post_send_counter_ = 0;
		std::atomic<int>                            post_recv_counter_ = 0;
	#endif
	};
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_SERVER_HPP__
