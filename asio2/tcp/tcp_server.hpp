/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_TCP_SERVER_HPP__
#define __ASIO2_TCP_SERVER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/server.hpp>
#include <asio2/tcp/tcp_session.hpp>

namespace asio2::detail
{
	template<class derived_t, class session_t>
	class tcp_server_impl_t : public server_impl_t<derived_t, session_t>
	{
		template <class, bool>  friend class user_timer_cp;
		template <class>        friend class post_cp;
		template <class, class> friend class server_impl_t;

	public:
		using self = tcp_server_impl_t<derived_t, session_t>;
		using super = server_impl_t<derived_t, session_t>;
		using session_type = session_t;

		/**
		 * @constructor
		 */
		explicit tcp_server_impl_t(
			std::size_t init_buffer_size = tcp_frame_size,
			std::size_t max_buffer_size = (std::numeric_limits<std::size_t>::max)(),
			std::size_t concurrency = std::thread::hardware_concurrency() * 2
		)
			: super(concurrency)
			, acceptor_(this->io_.context())
			, acceptor_timer_(this->io_.context())
			, counter_timer_(this->io_.context())
			, init_buffer_size_(init_buffer_size)
			, max_buffer_size_(max_buffer_size)
		{
		}

		/**
		 * @destructor
		 */
		~tcp_server_impl_t()
		{
			this->stop();
		}

		/**
		 * @function : start the server
		 * @param service A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename StrOrInt>
		inline bool start(StrOrInt&& service)
		{
			return this->start(std::string_view{}, std::forward<StrOrInt>(service));
		}

		/**
		 * @function : start the server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param service A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt>
		inline bool start(String&& host, StrOrInt&& service)
		{
			return this->derived()._do_start(
				std::forward<String>(host), std::forward<StrOrInt>(service),
				condition_wrap<asio::detail::transfer_at_least_t>{asio::transfer_at_least(1)});
		}

		/**
		 * @function : start the server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param service A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 * @param condition The delimiter condition.Valid value types include the following:
		 * char,std::string,std::string_view,
		 * function:std::pair<iterator, bool> match_condition(iterator begin, iterator end),
		 * asio::transfer_at_least,asio::transfer_exactly
		 * more details see asio::read_until
		 */
		template<typename String, typename StrOrInt, typename MatchCondition>
		inline bool start(String&& host, StrOrInt&& service, MatchCondition condition)
		{
			return this->derived()._do_start(
				std::forward<String>(host), std::forward<StrOrInt>(service),
				condition_wrap<MatchCondition>(condition));
		}

		/**
		 * @function : stop the server
		 * You can call this function on the communication thread and anywhere to stop the server.
		 */
		inline void stop()
		{
			this->derived()._do_stop(asio::error::operation_aborted);

			this->iopool_.stop();

			// asio bug , see : https://www.boost.org/users/history/version_1_72_0.html
			// Fixed a lost "outstanding work count" that can occur when an asynchronous 
			// accept operation is automatically restarted.
			// Using boost 1.72.0 or above version can avoid this problem (asio 1.16.0)
			ASIO2_ASSERT(this->state_ == state_t::stopped);
		}

		/**
		 * @function : check whether the server is started 
		 */
		inline bool is_started() { return (super::is_started() && this->acceptor_.is_open()); }

		/**
		 * @function : check whether the server is stopped
		 */
		inline bool is_stopped() { return (super::is_stopped() && !this->acceptor_.is_open()); }

	public:
		/**
		 * @function : bind recv listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * Function signature : void(std::shared_ptr<asio2::tcp_session>& session_ptr, std::string_view s)
		 */
		template<class F, class ...C>
		inline derived_t & bind_recv(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::recv,
				observer_t<std::shared_ptr<session_t>&, std::string_view>(
					std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @function : bind accept listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * This notification is invoked immediately when the server accept a new connection
		 * Function signature : void(std::shared_ptr<asio2::tcp_session>& session_ptr)
		 */
		template<class F, class ...C>
		inline derived_t & bind_accept(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::accept,
				observer_t<std::shared_ptr<session_t>&>(
					std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @function : bind connect listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * This notification is invoked after the connection is fully established,
		 * and only after the connect/handshake/upgrade are completed.
		 * Function signature : void(std::shared_ptr<asio2::tcp_session>& session_ptr)
		 */
		template<class F, class ...C>
		inline derived_t & bind_connect(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::connect,
				observer_t<std::shared_ptr<session_t>&>(
					std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @function : bind disconnect listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * This notification is invoked before the connection is disconnected, you can call
		 * get_last_error/last_error_msg, etc, to get the disconnected error information
		 * Function signature : void(std::shared_ptr<asio2::tcp_session>& session_ptr)
		 */
		template<class F, class ...C>
		inline derived_t & bind_disconnect(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::disconnect,
				observer_t<std::shared_ptr<session_t>&>(
					std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @function : bind init listener,we should set socket options at here
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * This notification is called after the socket is opened.
		 * You can set the socket option in this notification.
		 * Function signature : void()
		 */
		template<class F, class ...C>
		inline derived_t & bind_init(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::init, observer_t<>(
				std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @function : bind start listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * This notification is called after the server starts up, whether successful or unsuccessful
		 * Function signature : void(asio2::error_code ec)
		 */
		template<class F, class ...C>
		inline derived_t & bind_start(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::start, observer_t<error_code>(
				std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @function : bind stop listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * This notification is called before the server is ready to stop
		 * Function signature : void(asio2::error_code ec)
		 */
		template<class F, class ...C>
		inline derived_t & bind_stop(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::stop, observer_t<error_code>(
				std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	public:
		/**
		 * @function : get the acceptor refrence
		 */
		inline asio::ip::tcp::acceptor & acceptor() { return this->acceptor_; }

	protected:
		template<typename String, typename StrOrInt, typename MatchCondition>
		bool _do_start(String&& host, StrOrInt&& service, condition_wrap<MatchCondition> condition)
		{
			state_t expected = state_t::stopped;
			if (!this->state_.compare_exchange_strong(expected, state_t::starting))
			{
				set_last_error(asio::error::already_started);
				return false;
			}

			try
			{
				clear_last_error();

				this->iopool_.start();

				if (this->iopool_.is_stopped())
				{
					set_last_error(asio::error::shut_down);
					return false;
				}

				this->counter_ptr_ = std::shared_ptr<void>((void*)1, [this](void*)
				{
					this->derived().post([this]() mutable
					{
						state_t expected = state_t::stopping;
						if (this->state_.compare_exchange_strong(expected, state_t::stopped))
							this->derived()._handle_stop(
								asio::error::operation_aborted, this->derived().selfptr());
						else
							ASIO2_ASSERT(false);
					});
				});

				this->acceptor_.close(ec_ignore);

				std::string h = to_string(std::forward<String>(host));
				std::string p = to_string(std::forward<StrOrInt>(service));

				// parse address and port
				asio::ip::tcp::resolver resolver(this->io_.context());
				asio::ip::tcp::endpoint endpoint = *resolver.resolve(h, p,
					asio::ip::resolver_base::flags::passive |
					asio::ip::resolver_base::flags::address_configured).begin();

				this->acceptor_.open(endpoint.protocol());

				// when you close socket in linux system,and start socket
				// immediate,you will get like this "the address is in use",
				// and bind is failed,but i'm suer i close the socket correct
				// already before,why does this happen? the reasion is the 
				// socket option "TIME_WAIT",although you close the socket,
				// but the system not release the socket,util 2~4 seconds 
				// later,so we can use the SO_REUSEADDR option to avoid this
				// problem,like below
				
				// set port reuse
				this->acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));

				this->derived()._fire_init();

				this->acceptor_.bind(endpoint);
				this->acceptor_.listen();

				this->derived()._handle_start(error_code{}, std::move(condition));

				return (this->is_started());
			}
			catch (system_error & e)
			{
				this->derived()._handle_start(e.code(), std::move(condition));
			}
			return false;
		}

		template<typename MatchCondition>
		void _handle_start(error_code ec, condition_wrap<MatchCondition> condition)
		{
			try
			{
				// Whether the startup succeeds or fails, always call fire_start notification
				state_t expected = state_t::starting;
				if (!ec)
					if (!this->state_.compare_exchange_strong(expected, state_t::started))
						ec = asio::error::operation_aborted;

				set_last_error(ec);

				this->derived()._fire_start(ec);

				expected = state_t::started;
				if (!ec)
					if (!this->state_.compare_exchange_strong(expected, state_t::started))
						asio::detail::throw_error(asio::error::operation_aborted);

				asio::detail::throw_error(ec);

				this->derived().post([this, condition]() mutable
				{
					this->derived()._post_accept(std::move(condition));
				});
			}
			catch (system_error & e)
			{
				set_last_error(e);
				this->derived()._do_stop(e.code());
			}
		}

		inline void _do_stop(const error_code& ec)
		{
			state_t expected = state_t::starting;
			if (this->state_.compare_exchange_strong(expected, state_t::stopping))
				return this->derived()._post_stop(ec, this->derived().selfptr(), expected);

			expected = state_t::started;
			if (this->state_.compare_exchange_strong(expected, state_t::stopping))
				return this->derived()._post_stop(ec, this->derived().selfptr(), expected);
		}

		inline void _post_stop(const error_code& ec, std::shared_ptr<derived_t> self_ptr, state_t old_state)
		{
			// asio don't allow operate the same socket in multi thread,
			// if you close socket in one thread and another thread is
			// calling socket's async_... function,it will crash.so we
			// must care for operate the socket. when need close the 
			// socket ,we use the strand to post a event,make sure the
			// socket's close operation is in the same thread.
			this->derived().post([this, ec, this_ptr = std::move(self_ptr), old_state]() mutable
			{
				detail::ignore::unused(this, old_state);

				set_last_error(ec);

				// start timer to hold the acceptor io_context
				this->counter_timer_.expires_after((std::chrono::nanoseconds::max)());
				this->counter_timer_.async_wait(asio::bind_executor(
					this->io_.strand(), [](const error_code&) {}));

				// stop all the sessions, the session::stop must be no blocking,
				// otherwise it may be cause loop lock.
				this->sessions_.foreach([](std::shared_ptr<session_t> & session_ptr) mutable
				{
					session_ptr->stop();
				});

				this->counter_ptr_.reset();
			});
		}

		inline void _handle_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			detail::ignore::unused(ec, this_ptr);

			this->derived()._fire_stop(ec);

			this->acceptor_timer_.cancel(ec_ignore);
			this->counter_timer_.cancel(ec_ignore);

			// call the base class stop function
			super::stop();

			// call acceptor's close function to notify the _handle_accept
			// function response with error > 0 , then the listen socket
			// can get notify to exit must ensure the close function has 
			// been called,otherwise the _handle_accept will never return
			this->acceptor_.close(ec_ignore);
		}

		template<typename... Args>
		inline std::shared_ptr<session_t> _make_session(Args&&... args)
		{
			return std::make_shared<session_t>(std::forward<Args>(args)...,
				this->sessions_, this->listener_, this->iopool_.get(),
				this->init_buffer_size_, this->max_buffer_size_);
		}

		template<typename MatchCondition>
		inline void _post_accept(condition_wrap<MatchCondition> condition)
		{
			if (!this->is_started())
				return;

			try
			{
				std::shared_ptr<session_t> session_ptr = this->derived()._make_session();

				auto& socket = session_ptr->socket().lowest_layer();
				this->acceptor_.async_accept(socket, asio::bind_executor(this->io_.strand(),
					make_allocator(this->rallocator_,
						[this, sptr = std::move(session_ptr), condition]
				(const error_code& ec) mutable
				{
					this->derived()._handle_accept(ec, std::move(sptr), std::move(condition));
				})));
			}
			// handle exception,may be is the exception "Too many open files" (exception code : 24)
			catch (system_error & e)
			{
				set_last_error(e);

				this->acceptor_timer_.expires_after(std::chrono::seconds(1));
				this->acceptor_timer_.async_wait(asio::bind_executor(this->io_.strand(),
					make_allocator(this->rallocator_, [this, condition]
					(const error_code& ec) mutable
				{
					set_last_error(ec);
					if (ec) return;
					asio::post(this->io_.strand(), make_allocator(this->rallocator_,
						[this, condition]() mutable
					{
						this->derived()._post_accept(std::move(condition));
					}));
				})));
			}
		}

		template<typename MatchCondition>
		inline void _handle_accept(const error_code & ec, std::shared_ptr<session_t> session_ptr,
			condition_wrap<MatchCondition> condition)
		{
			set_last_error(ec);

			// if the acceptor status is closed,don't call _post_accept again.
			if (ec == asio::error::operation_aborted)
			{
				this->derived()._do_stop(ec);
				return;
			}

			if (!ec)
			{
				if (this->is_started())
				{
					session_ptr->counter_ptr_ = this->counter_ptr_;
					session_ptr->start(condition);
				}
			}

			this->derived()._post_accept(std::move(condition));
		}

		inline void _fire_init()
		{
			this->listener_.notify(event::init);
		}

		inline void _fire_start(error_code ec)
		{
			this->listener_.notify(event::start, ec);
		}

		inline void _fire_stop(error_code ec)
		{
			this->listener_.notify(event::stop, ec);
		}

	protected:
		/// acceptor to accept client connection
		asio::ip::tcp::acceptor acceptor_;

		/// timer for acceptor exception, like the exception "Too many open files" (exception code : 24)
		asio::steady_timer      acceptor_timer_;

		/// used to hold the acceptor io_context util all sessions are closed already.
		asio::steady_timer      counter_timer_;

		std::size_t             init_buffer_size_ = tcp_frame_size;

		std::size_t             max_buffer_size_ = (std::numeric_limits<std::size_t>::max)();
	};
}

namespace asio2
{
	template<class session_t>
	class tcp_server_t : public detail::tcp_server_impl_t<tcp_server_t<session_t>, session_t>
	{
	public:
		using detail::tcp_server_impl_t<tcp_server_t<session_t>, session_t>::tcp_server_impl_t;
	};

	using tcp_server = tcp_server_t<tcp_session>;
}

#endif // !__ASIO2_TCP_SERVER_HPP__
