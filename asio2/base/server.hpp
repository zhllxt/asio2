/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_SERVER_HPP__
#define __ASIO2_SERVER_HPP__

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

#include <asio2/base/component/user_data_cp.hpp>
#include <asio2/base/component/user_timer_cp.hpp>
#include <asio2/base/component/post_cp.hpp>

namespace asio2::detail
{
	template<class derived_t, class session_t>
	class server_impl_t
		: public object_t<derived_t>
		, public iopool_cp
		, public user_data_cp<derived_t>
		, public user_timer_cp<derived_t, false>
		, public post_cp<derived_t>
	{
		template <class, bool>  friend class user_timer_cp;
		template <class>        friend class post_cp;

	public:
		using self = server_impl_t<derived_t, session_t>;
		using super = object_t<derived_t>;

		/**
		 * @constructor
		 */
		explicit server_impl_t(std::size_t concurrency = std::thread::hardware_concurrency() * 2)
			: object_t<derived_t>()
			, iopool_cp(concurrency)
			, user_data_cp<derived_t>()
			, user_timer_cp<derived_t, false>(iopool_.get(0))
			, post_cp<derived_t>()
			, rallocator_()
			, wallocator_()
			, listener_()
			, io_(iopool_.get(0))
			, sessions_(io_)
		{
		}

		/**
		 * @destructor
		 */
		~server_impl_t()
		{
		}

		/**
		 * @function : start the server
		 */
		inline bool start()
		{
			return true;
		}

		/**
		 * @function : stop the server
		 */
		inline void stop()
		{
			if (!this->io_.strand().running_in_this_thread())
			{
				this->derived().post([this, this_ptr = this->derived().selfptr()]() mutable
				{
					this->stop();
				});
				return;
			}

			// close user custom timers
			this->stop_all_timers();

			// destroy user data, maybe the user data is self shared_ptr, 
			// if don't destroy it, will cause loop refrence.
			this->user_data_.reset();
		}

		/**
		 * @function : check whether the server is started 
		 */
		inline bool is_started() const
		{
			return (this->state_ == state_t::started);
		}

		/**
		 * @function : check whether the server is stopped
		 */
		inline bool is_stopped() const
		{
			return (this->state_ == state_t::stopped);
		}

		/**
		 * @function : Asynchronous send data for each session
		 * supporting multi data formats,see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * PodType * : send("abc");
		 * PodType (&data)[N] : double m[10]; send(m);
		 * std::array<PodType, N> : std::array<int,10> m; send(m);
		 * std::vector<PodType, Allocator> : std::vector<float> m; send(m);
		 * std::basic_string<Elem, Traits, Allocator> : std::string m; send(m);
		 */
		template<class T>
		inline derived_t & send(const T& data)
		{
			this->sessions_.foreach([&data](std::shared_ptr<session_t>& session_ptr) mutable
			{
				session_ptr->send(data);
			});
			return this->derived();
		}

		/**
		 * @function : Asynchronous send data for each session
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * We do not provide synchronous send function,because the synchronous send code is very simple,
		 * if you want use synchronous send data,you can do it like this (example):
		 * asio::write(session_ptr->socket(), asio::buffer(std::string("abc")));
		 * PodType * : send("abc");
		 */
		template<class CharT, class Traits = std::char_traits<CharT>>
		inline typename std::enable_if_t<
			std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char> ||
			std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, wchar_t> ||
			std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char16_t> ||
			std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char32_t>, derived_t&> send(CharT * s)
		{
			return this->send(s, s ? Traits::length(s) : 0);
		}

		/**
		 * @function : Asynchronous send data for each session
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * We do not provide synchronous send function,because the synchronous send code is very simple,
		 * if you want use synchronous send data,you can do it like this (example):
		 * asio::write(session_ptr->socket(), asio::buffer(std::string("abc")));
		 * PodType (&data)[N] : double m[10]; send(m,5);
		 */
		template<class CharT, class SizeT>
		inline typename std::enable_if_t<std::is_integral_v<std::remove_cv_t<
			std::remove_reference_t<SizeT>>>, derived_t&>
			send(CharT* s, SizeT count)
		{
			if (s)
			{
				this->sessions_.foreach([s, count](std::shared_ptr<session_t>& session_ptr)
				{
					session_ptr->send(s, count);
				});
			}
			return this->derived();
		}

	public:
		/**
		 * @function : get the acceptor refrence,derived classes must override this function
		 */
		inline auto & acceptor() { return this->derived().acceptor(); }

		/**
		 * @function : get the listen address
		 */
		inline std::string listen_address()
		{
			try
			{
				return this->acceptor().local_endpoint().address().to_string();
			}
			catch (system_error & e) { set_last_error(e); }
			return std::string();
		}

		/**
		 * @function : get the listen port
		 */
		inline unsigned short listen_port()
		{
			try
			{
				return this->acceptor().local_endpoint().port();
			}
			catch (system_error & e) { set_last_error(e); }
			return static_cast<unsigned short>(0);
		}


		/**
		 * @function : get connected session count
		 */
		inline std::size_t session_count() { return this->sessions_.size(); }

		/**
		 * @function :
		 * @param    : fn - The handler to be called for each session.
		 * Function signature :
		 * void(std::shared_ptr<asio2::xxx_session>& session_ptr)
		 */
		inline derived_t & foreach_session(const std::function<void(std::shared_ptr<session_t>&)> & fn)
		{
			this->sessions_.foreach(fn);
			return this->derived();
		}

		/**
		 * @function : find the session by user custom role
		 * @param    : fn - The handler to be called when search the session.
		 * Function signature :
		 * bool(std::shared_ptr<asio2::xxx_session>& session_ptr)
		 * @return   : std::shared_ptr<asio2::xxx_session>
		 */
		inline std::shared_ptr<session_t> find_session_if(
			const std::function<bool(std::shared_ptr<session_t>&)> & fn)
		{
			return std::shared_ptr<session_t>(this->sessions_.find_if(fn));
		}

		/**
		 * @function : get the io object refrence
		 */
		inline io_t & io() { return this->io_; }

	protected:
		/**
		 * @function : get the recv/read allocator object refrence
		 */
		inline auto & rallocator() { return this->rallocator_; }
		/**
		 * @function : get the send/write/post allocator object refrence
		 */
		inline auto & wallocator() { return this->wallocator_; }

		inline session_mgr_t<session_t> & sessions() { return this->sessions_; }
		inline listener_t               & listener() { return this->listener_; }
		inline std::atomic<state_t>     & state()    { return this->state_;    }
		inline std::shared_ptr<derived_t> selfptr()  { return std::shared_ptr<derived_t>{}; }

	protected:
		// The memory to use for handler-based custom memory allocation. used for acceptor.
		handler_memory<>                            rallocator_;

		/// The memory to use for handler-based custom memory allocation. used fo send/write/post.
		handler_memory<size_op<>, std::true_type>   wallocator_;

		/// listener
		listener_t                                  listener_;

		/// The io (include io_context and strand) used to handle the accept event.
		io_t                                      & io_;

		/// session_mgr
		session_mgr_t<session_t>                    sessions_;

		/// state
		std::atomic<state_t>                        state_ = state_t::stopped;

		/// use this to ensure that server stop only after all sessions are closed
		std::shared_ptr<void>                       counter_ptr_;
	};
}

#endif // !__ASIO2_SERVER_HPP__
